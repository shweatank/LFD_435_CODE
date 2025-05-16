
/*
====================================================
Lab: Automatic Module Loading + Module Usage Count
Author: TechDhaba
Description: Combined kernel module examples for basic to intermediate level.
====================================================
*/

// -------------------------
// 1. Autoloadable Module
// -------------------------
/*
Steps:
1. Save this file and extract autoloader section into its own .c file if needed.
2. Compile with a Makefile (provided below).
3. Copy to /lib/modules/$(uname -r)/extra/
4. Run `sudo depmod -a`
5. Run `sudo modprobe autoloader`
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init autoloader_init(void)
{
    pr_info("Autoloadable module inserted\n");
    return 0;
}

static void __exit autoloader_exit(void)
{
    pr_info("Autoloadable module removed\n");
}

module_init(autoloader_init);
module_exit(autoloader_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Simple autoloadable module");

// -------------------------
// 2. Usage Count Module
// -------------------------
/*
Steps:
1. This module demonstrates use of try_module_get and module_put.
2. Compile using Makefile.
3. Insert with insmod and observe usage count via /proc/modules.
4. Remove with rmmod (will succeed if count is released).
*/

static int __init usage_count_init(void)
{
    pr_info("Module with usage count loaded\n");

    if (!try_module_get(THIS_MODULE)) {
        pr_err("Failed to increment usage count\n");
        return -EBUSY;
    }

    pr_info("Module usage count incremented\n");
    return 0;
}

static void __exit usage_count_exit(void)
{
    module_put(THIS_MODULE);
    pr_info("Module usage count decremented\n");
    pr_info("Module with usage count unloaded\n");
}

module_init(usage_count_init);
module_exit(usage_count_exit);

// -------------------------
// 3. Exporting and Resolving Symbols
// -------------------------
// Part A: Provider module
int exported_add(int a, int b)
{
    return a + b;
}
EXPORT_SYMBOL(exported_add);

// Part B: Consumer module
extern int exported_add(int, int);

static int __init consumer_init(void)
{
    int result = exported_add(10, 15);
    pr_info("Consumer: Result from exported_add = %d\n", result);
    return 0;
}

static void __exit consumer_exit(void)
{
    pr_info("Consumer module unloaded\n");
}

module_init(consumer_init);
module_exit(consumer_exit);
