#include <linux/init.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("debugfs demo");

#define DEBUG_FILE "debug_demo"
static char buffer[128] = "Hello from debugfs\n";
static struct dentry *dentry;

static ssize_t dbg_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    return simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
}

static ssize_t dbg_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    if (len >= sizeof(buffer)) return -EINVAL;
    if (copy_from_user(buffer, buf, len)) return -EFAULT;
    buffer[len] = '\0';
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dbg_read,
    .write = dbg_write,
};

static int __init debug_init(void) {
    dentry = debugfs_create_file(DEBUG_FILE, 0666, NULL, NULL, &fops);
    return 0;
}

static void __exit debug_exit(void) {
    debugfs_remove(dentry);
}

module_init(debug_init);
module_exit(debug_exit);