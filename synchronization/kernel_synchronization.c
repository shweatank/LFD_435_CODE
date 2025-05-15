// Kernel Module: kthread_mutex_semaphore_spinlock.c

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/device.h>

#define DEVICE_NAME "sync_driver"
#define CLASS_NAME "sync"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("Linux Device Driver with kernel thread, mutex, semaphore, and spinlock");

static int major;
static struct class *sync_class = NULL;
static struct device *sync_device = NULL;

// Shared resource and synchronization primitives
static int shared_resource = 0;
static struct mutex resource_mutex;
static struct semaphore resource_semaphore;
static spinlock_t resource_spinlock;

// Kernel thread
static struct task_struct *kthread;
static int thread_running = 1;

// Thread function
static int thread_function(void *data) {
    while (!kthread_should_stop()) {
        mutex_lock(&resource_mutex);
        pr_info("[Thread-Mutex] Incrementing shared resource: %d\n", ++shared_resource);
        mutex_unlock(&resource_mutex);
        
        msleep(1000);
        
        if (down_interruptible(&resource_semaphore) == 0) {
            pr_info("[Thread-Semaphore] Decrementing shared resource: %d\n", --shared_resource);
            up(&resource_semaphore);
        }

        spin_lock(&resource_spinlock);
        shared_resource += 2;
        pr_info("[Thread-Spinlock] Adding 2 to shared resource: %d\n", shared_resource);
        spin_unlock(&resource_spinlock);

        msleep(1000);
    }
    return 0;
}

// Device file operations
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    char buffer[64];
    int count;

    mutex_lock(&resource_mutex);
    count = snprintf(buffer, sizeof(buffer), "Shared Resource: %d\n", shared_resource);
    mutex_unlock(&resource_mutex);

    if (*offset >= count) {
        return 0;
    }

    if (copy_to_user(buf, buffer, count)) {
        return -EFAULT;
    }

    *offset += count;
    return count;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
};

static int __init sync_driver_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register a major number\n");
        return major;
    }

    sync_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(sync_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        pr_err("Failed to register device class\n");
        return PTR_ERR(sync_class);
    }

    sync_device = device_create(sync_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(sync_device)) {
        class_destroy(sync_class);
        unregister_chrdev(major, DEVICE_NAME);
        pr_err("Failed to create the device\n");
        return PTR_ERR(sync_device);
    }

    mutex_init(&resource_mutex);
    sema_init(&resource_semaphore, 1);
    spin_lock_init(&resource_spinlock);

    kthread = kthread_run(thread_function, NULL, "sync_thread");
    if (IS_ERR(kthread)) {
        device_destroy(sync_class, MKDEV(major, 0));
        class_destroy(sync_class);
        unregister_chrdev(major, DEVICE_NAME);
        pr_err("Failed to create kernel thread\n");
        return PTR_ERR(kthread);
    }

    pr_info("Sync driver loaded successfully\n");
    return 0;
}

static void __exit sync_driver_exit(void) {
    thread_running = 0;
    kthread_stop(kthread);
    device_destroy(sync_class, MKDEV(major, 0));
    class_destroy(sync_class);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Sync driver unloaded\n");
}

module_init(sync_driver_init);
module_exit(sync_driver_exit);