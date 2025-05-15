#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/gpio/driver.h>

#define GPIO_BASE_OFFSET  0x00200000  // GPIO base relative to peripheral base
#define PERIPHERAL_BASE   0xFE000000  // Peripheral base for Raspberry Pi 4
#define BLOCK_SIZE        (4 * 1024)  // Memory block size for GPIO

// GPIO Registers offsets
#define GPFSEL_OFFSET  (0x00 / 4)  // GPIO Function Select
#define GPSET_OFFSET   (0x1C / 4)  // GPIO Pin Output Set
#define GPCLR_OFFSET   (0x28 / 4)  // GPIO Pin Output Clear
#define GPLEV_OFFSET   (0x34 / 4)  // GPIO Pin Level

static volatile uint32_t *gpio_map;
static struct class *gpio_class;
static struct device *gpio_device;

// Function to initialize memory-mapped GPIO
static int init_gpio_memory_map(void) {
    void __iomem *gpio_base;

    gpio_base = ioremap(PERIPHERAL_BASE + GPIO_BASE_OFFSET, BLOCK_SIZE);
    if (!gpio_base) {
        pr_err("Failed to ioremap GPIO memory\n");
        return -ENOMEM;
    }

    gpio_map = (volatile uint32_t *)gpio_base;
    return 0;
}

// Function to configure a GPIO pin as output
static void set_gpio_output(int pin) {
    int reg_index = pin / 10; // Each GPFSEL register configures 10 pins
    int bit_offset = (pin % 10) * 3; // Each pin takes 3 bits

    gpio_map[GPFSEL_OFFSET + reg_index] &= ~(0b111 << bit_offset); // Clear bits
    gpio_map[GPFSEL_OFFSET + reg_index] |= (0b001 << bit_offset); // Set to output
}

// Function to set a GPIO pin high
static void set_gpio_high(int pin) {
    gpio_map[GPSET_OFFSET] = (1 << pin);
}

// Function to set a GPIO pin low
static void set_gpio_low(int pin) {
    gpio_map[GPCLR_OFFSET] = (1 << pin);
}

// Function to read the state of a GPIO pin
static int read_gpio(int pin) {
    return (gpio_map[GPLEV_OFFSET] & (1 << pin)) != 0;
}

// File operations for user-space interaction
static ssize_t gpio_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char command[16];
    int pin, value;

    if (count > sizeof(command) - 1)
        return -EINVAL;

    if (copy_from_user(command, buf, count))
        return -EFAULT;

    command[count] = '\0';

    if (sscanf(command, "set %d %d", &pin, &value) == 2) {
        if (value)
            set_gpio_high(pin);
        else
            set_gpio_low(pin);
        return count;
    }

    return -EINVAL;
}

static const struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .write = gpio_write,
};

static int __init gpio_driver_init(void) {
    int ret;

    pr_info("Initializing GPIO kernel driver\n");

    // Initialize memory-mapped GPIO
    ret = init_gpio_memory_map();
    if (ret < 0)
        return ret;

    // Create a device in /dev for user interaction
    gpio_class = class_create(THIS_MODULE, "gpio_class");
    if (IS_ERR(gpio_class)) {
        pr_err("Failed to create class\n");
        iounmap((void __iomem *)gpio_map);
        return PTR_ERR(gpio_class);
    }

    gpio_device = device_create(gpio_class, NULL, MKDEV(0, 0), NULL, "gpio_driver");
    if (IS_ERR(gpio_device)) {
        pr_err("Failed to create device\n");
        class_destroy(gpio_class);
        iounmap((void __iomem *)gpio_map);
        return PTR_ERR(gpio_device);
    }

    ret = register_chrdev(0, "gpio_driver", &gpio_fops);
    if (ret < 0) {
        pr_err("Failed to register char device\n");
        device_destroy(gpio_class, MKDEV(0, 0));
        class_destroy(gpio_class);
        iounmap((void __iomem *)gpio_map);
        return ret;
    }

    pr_info("GPIO driver initialized successfully\n");
    return 0;
}

static void __exit gpio_driver_exit(void) {
    unregister_chrdev(0, "gpio_driver");
    device_destroy(gpio_class, MKDEV(0, 0));
    class_destroy(gpio_class);
    iounmap((void __iomem *)gpio_map);
    pr_info("GPIO driver exited\n");
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple GPIO kernel driver");