#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Tasklet Example Driver");

#define DEVICE_NAME "tasklet_dev"
#define IOCTL_TRIGGER_TASKLET _IO('T', 1)

static struct tasklet_struct my_tasklet;

// Tasklet function
static void tasklet_function(unsigned long data)
{
    printk(KERN_INFO "Tasklet executed! Data: %ld\n", data);
}

// File operations
static int dev_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    if (cmd == IOCTL_TRIGGER_TASKLET) {
        printk(KERN_INFO "Tasklet triggered from user-space\n");
        tasklet_schedule(&my_tasklet);
    }
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

static int major;

static int __init tasklet_example_init(void)
{
    printk(KERN_INFO "Tasklet example module loaded\n");
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Failed to register char device\n");
        return major;
    }
    printk(KERN_INFO "Registered device with major number %d\n", major);

    tasklet_init(&my_tasklet, tasklet_function, 42); // Data = 42
    return 0;
}

static void __exit tasklet_example_exit(void)
{
    printk(KERN_INFO "Tasklet example module unloaded\n");
    tasklet_kill(&my_tasklet);
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(tasklet_example_init);
module_exit(tasklet_example_exit);