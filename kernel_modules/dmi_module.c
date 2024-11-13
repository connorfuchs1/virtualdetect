#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/string.h>
#include <linux/ptrace.h>

static struct kretprobe kp;

// Static strings to replace virtualization identifiers
static char vendor_override[] = "Dell Inc.";
static char product_override[] = "Latitude E7470";

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

static int __init mymodule_init(void)
{
    int ret;

    kp.handler = ret_handler;
    kp.kp.symbol_name = "dmi_get_system_info";

    ret = register_kretprobe(&kp);
    if (ret < 0) {
        pr_err("register_kretprobe failed, returned %d\n", ret);
        return ret;
    }

    pr_info("Kretprobe registered for dmi_get_system_info\n");
    return 0;
}

static void __exit mymodule_exit(void)
{
    unregister_kretprobe(&kp);
    pr_info("Kretprobe unregistered\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Kretprobe module to override DMI data");
