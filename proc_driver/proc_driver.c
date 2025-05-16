#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("procfs demo");

#define PROC_NAME "proc_demo"
static char buffer[128] = "Hello from procfs\n";

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

static int __init proc_init(void) {
    proc_create(PROC_NAME, 0666, NULL, &proc_fops);
    return 0;
}

static void __exit proc_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
}

module_init(proc_init);
module_exit(proc_exit);