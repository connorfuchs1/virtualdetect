#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>

#define DMI_PHYSICAL_ADDRESS 0x000E0010  // Replace with actual address from dmidecode
#define DMI_TABLE_SIZE 28548             // Replace with actual size from dmidecode

static void __iomem *dmi_table_virt;

// Kretprobe structure for intercepting dmi_get_system_info
static struct kretprobe kp;

// Static strings to replace virtualization identifiers
static char vendor_override[] = "Dell Inc.";
static char product_override[] = "Latitude E7470";

// Function to change the page permissions of the specified virtual address
static void set_page_rw(void *address) {
    unsigned long addr = (unsigned long)address;
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;

    // Use current->mm to access the current process's memory map
    pgd = pgd_offset(current->mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {
        pr_err("Invalid PGD\n");
        return;
    }

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {
        pr_err("Invalid P4D\n");
        return;
    }

    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud)) {
        pr_err("Invalid PUD\n");
        return;
    }

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {
        pr_err("Invalid PMD\n");
        return;
    }

    pte = pte_offset_kernel(pmd, addr);
    if (!pte) {
        pr_err("Invalid PTE\n");
        return;
    }

    // Set the page to writable
    set_pte_atomic(pte, pte_mkwrite(*pte));
    pr_info("Page permissions changed to R/W\n");
}

// Function to replace specific strings in the DMI table
static void replace_dmi_string(void *table, size_t size) {
    char *data = (char *)table;
    const char *target = "VMware";
    const char *replacement = "Dell  "; // Make sure the replacement is the same length

    size_t target_len = strlen(target);
    size_t replacement_len = strlen(replacement);

    for (size_t i = 0; i <= size - target_len; i++) {
        // Check if the target string is found
        if (memcmp(&data[i], target, target_len) == 0) {
            // Replace with the new string
            memcpy(&data[i], replacement, target_len);

            pr_info("Replaced '%s' with '%s' at offset %zu\n", target, replacement, i);
        }
    }
}

// Kretprobe handler function to intercept dmi_get_system_info
static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    const char *retval = (const char *)regs_return_value(regs);

    if (retval) {
        // Compare the returned string and replace if necessary
        if (strcmp(retval, "VMware, Inc.") == 0) {
            regs_set_return_value(regs, (unsigned long)vendor_override);
            pr_info("Overridden sys_vendor\n");
        } else if (strcmp(retval, "VMware Virtual Platform") == 0) {
            regs_set_return_value(regs, (unsigned long)product_override);
            pr_info("Overridden product_name\n");
        }
        // Add more conditions as needed
    }

    return 0;
}

static int __init modify_dmi_init(void) {
    int ret;

    // Map the DMI table
    dmi_table_virt = ioremap(DMI_PHYSICAL_ADDRESS, DMI_TABLE_SIZE);
    if (!dmi_table_virt) {
        pr_err("Failed to map DMI table\n");
        return -ENOMEM;
    }

    // Change permissions to make the table writable
    set_page_rw(dmi_table_virt);

    // Now modify DMI data as needed
    replace_dmi_string(dmi_table_virt, DMI_TABLE_SIZE);

    // Register the kretprobe
    kp.handler = ret_handler;
    kp.kp.symbol_name = "dmi_get_system_info";

    ret = register_kretprobe(&kp);
    if (ret < 0) {
        pr_err("register_kretprobe failed, returned %d\n", ret);
        iounmap(dmi_table_virt);
        return ret;
    }

    pr_info("Module loaded: DMI table modified and kretprobe registered\n");
    return 0;
}

static void __exit modify_dmi_exit(void) {
    unregister_kretprobe(&kp);
    iounmap(dmi_table_virt);
    pr_info("Module unloaded: kretprobe unregistered and DMI table unmapped\n");
}

module_init(modify_dmi_init);
module_exit(modify_dmi_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kernel module to modify DMI table and intercept dmi_get_system_info");
