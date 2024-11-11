#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/acpi.h>

static struct kprobe kp;

// Pointer to the original `acpi_get_table` function
static acpi_status (*orig_acpi_get_table)(acpi_string signature, u32 instance, struct acpi_table_header **out_table) = acpi_get_table;

// Hook function for acpi_get_table
static int hook_acpi_get_table(struct kprobe *p, struct pt_regs *regs) {
    pr_info("ACPI table hook triggered.\n");
    return 0;
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
MODULE_DESCRIPTION("ACPI Masking Module");
