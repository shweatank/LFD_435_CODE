#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Combined FS driver: kobject + procfs + debugfs");

static char buffer[128] = "Initial message\n";

/* KObject */
static struct kobject *kobj;
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%s", buffer);
}
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    snprintf(buffer, sizeof(buffer), "%.*s", (int)count, buf);
    return count;
}
static struct kobj_attribute sysfs_attr = __ATTR(kmessage, 0660, sysfs_show, sysfs_store);

/* procfs */
#define PROC_NAME "proc_kmsg"
static ssize_t proc_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    return simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
}
static ssize_t proc_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    if (len >= sizeof(buffer)) return -EINVAL;
    if (copy_from_user(buffer, buf, len)) return -EFAULT;
    buffer[len] = '\0';
    return len;
}
static struct proc_ops proc_fops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

/* debugfs */
#define DEBUG_FILE "debug_kmsg"
static struct dentry *debug_file;
static ssize_t dbg_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    return simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
}
static ssize_t dbg_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    if (len >= sizeof(buffer)) return -EINVAL;
    if (copy_from_user(buffer, buf, len)) return -EFAULT;
    buffer[len] = '\0';
    return len;
}
static struct file_operations dbg_fops = {
    .owner = THIS_MODULE,
    .read = dbg_read,
    .write = dbg_write,
};

static int __init fsdriver_init(void) {
    kobj = kobject_create_and_add("kobj_demo", kernel_kobj);
    if (!kobj) return -ENOMEM;
    if (sysfs_create_file(kobj, &sysfs_attr.attr)) return -EINVAL;

    proc_create(PROC_NAME, 0666, NULL, &proc_fops);

    debug_file = debugfs_create_file(DEBUG_FILE, 0666, NULL, NULL, &dbg_fops);

    printk(KERN_INFO "All FS driver loaded\n");
    return 0;
}

static void __exit fsdriver_exit(void) {
    sysfs_remove_file(kobj, &sysfs_attr.attr);
    kobject_put(kobj);
    remove_proc_entry(PROC_NAME, NULL);
    debugfs_remove(debug_file);
    printk(KERN_INFO "All FS driver unloaded\n");
}

module_init(fsdriver_init);
module_exit(fsdriver_exit);