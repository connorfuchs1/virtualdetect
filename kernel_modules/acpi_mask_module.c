#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/acpi.h>
#include <linux/printk.h>

static struct kprobe kp;

// Pointer to the original `acpi_get_table` function
static acpi_status (*orig_acpi_get_table)(acpi_string signature, u32 instance, struct acpi_table_header **out_table) = acpi_get_table;

// Function to update the checksum for an ACPI table
static void update_acpi_table_checksum(struct acpi_table_header *table, size_t length) {
    unsigned char *bytes = (unsigned char *)table;
    unsigned char sum = 0;
    size_t i;

    // Calculate the sum of all bytes except the checksum byte
    for (i = 0; i < length; i++) {
        if (i != offsetof(struct acpi_table_header, checksum))
            sum += bytes[i];
    }

    // Set the checksum byte so the total sum is 0
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

// Hook function for acpi_get_table
static int hook_acpi_get_table(struct kprobe *p, struct pt_regs *regs) {
    acpi_status status;
    struct acpi_table_header *table;

    // Call the original function to get the table
    status = orig_acpi_get_table((acpi_string)regs->regs[0], (u32)regs->regs[1], &table);
    
    if (ACPI_SUCCESS(status)) {
        pr_info("ACPI table successfully retrieved. Signature: %.4s\n", table->signature);
        
        // Modify the table (example: modify the OEM ID)
        strncpy(table->oem_id, "MODIF", sizeof(table->oem_id));
        
        // Update the checksum after modification
        update_acpi_table_checksum(table, table->length);
        
        // Print the modified table for verification
        print_acpi_table_hex(table, 64);  // Print first 64 bytes for inspection
    } else {
        pr_err("Failed to retrieve ACPI table: %d\n", status);
    }

    return 0;  // Continue execution
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

    pr_info("acpi_mask_module loaded.\n");
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
MODULE_DESCRIPTION("ACPI Table Masking Kernel Module");
