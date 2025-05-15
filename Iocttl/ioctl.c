#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "ioctl_device"
#define CLASS_NAME "ioctl_class"

// IOCTL Commands
#define IOCTL_GET_DATA _IOR('i', 1, int)
#define IOCTL_SET_DATA _IOW('i', 2, int)

static int device_open = 0;
static int ioctl_data = 0;

static int dev_open(struct inode *inodep, struct file *filep)
{
    if (device_open)
        return -EBUSY;

    device_open++;
    pr_info("Device opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
    device_open--;
    pr_info("Device closed\n");
    return 0;
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int temp;
    switch (cmd) {
    case IOCTL_GET_DATA:
        if (copy_to_user((int __user *)arg, &ioctl_data, sizeof(ioctl_data))) {
            return -EFAULT;
        }
        pr_info("IOCTL_GET_DATA: Data sent to user: %d\n", ioctl_data);
        break;
    case IOCTL_SET_DATA:
        if (copy_from_user(&temp, (int __user *)arg, sizeof(temp))) {
            return -EFAULT;
        }
        ioctl_data = temp;
        pr_info("IOCTL_SET_DATA: Data received from user: %d\n", ioctl_data);
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
};

static int major_number;
static struct class *ioctl_class;
static struct device *ioctl_device;

static int __init ioctl_driver_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("Failed to register a major number\n");
        return major_number;
    }
    pr_info("Registered with major number %d\n", major_number);

    ioctl_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ioctl_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to register device class\n");
        return PTR_ERR(ioctl_class);
    }
    pr_info("Device class registered\n");

    ioctl_device = device_create(ioctl_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ioctl_device)) {
        class_destroy(ioctl_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        pr_err("Failed to create the device\n");
        return PTR_ERR(ioctl_device);
    }
    pr_info("Device created successfully\n");
    return 0;
}

static void __exit ioctl_driver_exit(void)
{
    device_destroy(ioctl_class, MKDEV(major_number, 0));
    class_unregister(ioctl_class);
    class_destroy(ioctl_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("IOCTL driver unloaded\n");
}

module_init(ioctl_driver_init);
module_exit(ioctl_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("A simple IOCTL Linux driver");
MODULE_VERSION("1.0");