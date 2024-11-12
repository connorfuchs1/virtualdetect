#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/acpi.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/printk.h>
#include <linux/ctype.h>

static struct kprobe kp;

// Pointer to the original `acpi_get_table` function
static acpi_status (*orig_acpi_get_table)(acpi_string signature, u32 instance, struct acpi_table_header **out_table) = acpi_get_table;

// Function to update the checksum for an ACPI table
static void update_acpi_table_checksum(struct acpi_table_header *table, size_t length) {
    unsigned char *bytes = (unsigned char *)table;
    unsigned char sum = 0;
    size_t i;

    for (i = 0; i < length; i++) {
        if (i != offsetof(struct acpi_table_header, checksum))
            sum += bytes[i];
    }
    table->checksum = (unsigned char)(0 - sum);
}

// Function to print a portion of the ACPI table in hex format
static void print_acpi_table_hex(struct acpi_table_header *table, size_t length) {
    size_t i;
    pr_info("ACPI Table Dump (first %zu bytes):\n", length);
    for (i = 0; i < length; i++) {
        if (i % 16 == 0)
            pr_info("\n%04zx: ", i);
        pr_cont("%02x ", ((unsigned char *)table)[i]);
    }
    pr_info("\n");
}

// Function to perform case-insensitive replacement in the table
static void replace_string_in_table(struct acpi_table_header *table, size_t length, const char *target, const char *replacement) {
    unsigned char *data = (unsigned char *)table;
    size_t target_len = strlen(target);
    size_t replacement_len = strlen(replacement);

    for (size_t i = 0; i <= length - target_len; i++) {
        bool match = true;
        
        // Perform case-insensitive comparison
        for (size_t j = 0; j < target_len; j++) {
            if (tolower(data[i + j]) != tolower(target[j])) {
                match = false;
                break;
            }
        }

        // If target is found, replace it
        if (match) {
            // Replace target with replacement, pad with spaces if replacement is shorter
            memset(&data[i], ' ', target_len);
            memcpy(&data[i], replacement, min(target_len, replacement_len));
            i += target_len - 1;
        }
    }
}

// Hook function for acpi_get_table
static int hook_acpi_get_table(struct kprobe *p, struct pt_regs *regs) {
    acpi_status status;
    struct acpi_table_header *table;
    acpi_string signature;
    u32 instance;

#ifdef __x86_64__
    signature = (acpi_string)regs->di;
    instance = (u32)regs->si;
#elif defined(__aarch64__)
    signature = (acpi_string)regs->regs[0];
    instance = (u32)regs->regs[1];
#else
#error "Unsupported architecture"
#endif

    status = orig_acpi_get_table(signature, instance, &table);

    if (ACPI_SUCCESS(status)) {
        pr_info("ACPI table successfully retrieved. Signature: %.4s\n", table->signature);

        // Replace known virtualization artifacts with generic values
        replace_string_in_table(table, table->length, "QEMU", "GENUINE");
        replace_string_in_table(table, table->length, "VMWARE", "MICROSFT");
        replace_string_in_table(table, table->length, "EDK2", "BIOSV");
        replace_string_in_table(table, table->length, "BXPC", "SYSC ");
        replace_string_in_table(table, table->length, "VIRTUAL", "REAL  "); // Added "VIRTUAL"

        // Update the checksum after modification
        update_acpi_table_checksum(table, table->length);

        // Print the modified table for verification
        print_acpi_table_hex(table, 64);
    } else {
        pr_err("Failed to retrieve ACPI table: %d\n", status);
    }

    return 0;
}

// File read function using kernel_read
static ssize_t read_acpi_table(const char *path, char *buffer, size_t len) {
    struct file *file;
    ssize_t ret;

    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        pr_err("Could not open file: %s\n", path);
        return PTR_ERR(file);
    }

    ret = kernel_read(file, buffer, len, &file->f_pos);

    filp_close(file, NULL);
    return ret;
}

static int __init acpi_mask_init(void) {
    int ret;

    kp.pre_handler = hook_acpi_get_table;
    kp.addr = (kprobe_opcode_t *)orig_acpi_get_table;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("register_kprobe failed, returned %d\n", ret);
        return ret;
    }

    pr_info("acpi_mask_module loaded with kernel_read monitoring.\n");
    return 0;
}

static void __exit acpi_mask_exit(void) {
    unregister_kprobe(&kp);
    pr_info("acpi_mask_module unloaded.\n");
}

module_init(acpi_mask_init);
module_exit(acpi_mask_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("ACPI Table Masking Kernel Module with kernel_read");
