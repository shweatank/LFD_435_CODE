#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("KObject Sysfs Demo");

static struct kobject *kobj;
static char buffer[128] = "Hello from kobj\n";

static ssize_t show_fn(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%s", buffer);
}

static ssize_t store_fn(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    snprintf(buffer, sizeof(buffer), "%.*s", (int)count, buf);
    return count;
}

static struct kobj_attribute attr = __ATTR(kmsg, 0660, show_fn, store_fn);

static int __init kobj_init(void) {
    kobj = kobject_create_and_add("kobj_demo", kernel_kobj);
    if (!kobj) return -ENOMEM;
    return sysfs_create_file(kobj, &attr.attr);
}

static void __exit kobj_exit(void) {
    sysfs_remove_file(kobj, &attr.attr);
    kobject_put(kobj);
}

module_init(kobj_init);
module_exit(kobj_exit);