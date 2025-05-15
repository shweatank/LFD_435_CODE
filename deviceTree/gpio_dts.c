//gpio_led.c                                                                                                                                                       #include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/of.h>   // For Device Tree functions
#include <linux/of_gpio.h>  // For GPIO handling in Device Tree
#include<linux/delay.h>
static int led_gpio = -1;  // GPIO pin for LED (default -1)

static int __init gpio_led_init(void)
{
    struct device_node *node;
    int ret;

    // Get the device tree node for the GPIO
    node = of_find_node_by_name(NULL, "gpio_led");
    if (!node) {
        pr_err("GPIO LED node not found in Device Tree\n");
        return -ENODEV;
    }

    // Get the GPIO from the device tree
    led_gpio = of_get_named_gpio(node, "gpios", 0);
    if (led_gpio < 0) {
        pr_err("Failed to get GPIO from Device Tree\n");
        return led_gpio;
    }

    pr_info("LED GPIO found: %d\n", led_gpio);

    // Request the GPIO and set it as output
    ret = gpio_request(led_gpio, "LED GPIO");
    if (ret) {
        pr_err("Failed to request GPIO %d\n", led_gpio);
        return ret;
    }

    ret = gpio_direction_output(led_gpio, 0);  // Start with LED OFF
    if (ret) {
        pr_err("Failed to set GPIO direction\n");
        gpio_free(led_gpio);
        return ret;
    }

    // Toggle the LED (turn ON and then OFF after a delay)
    gpio_set_value(led_gpio, 1);  // Turn LED ON
    msleep(1000);  // Wait for 1 second
    gpio_set_value(led_gpio, 0);  // Turn LED OFF

    return 0;
}

static void __exit gpio_led_exit(void)
{
    if (led_gpio >= 0) {
        gpio_set_value(led_gpio, 0);  // Ensure the LED is OFF
        gpio_free(led_gpio);  // Free the GPIO
    }

    pr_info("GPIO LED driver removed\n");
}

module_init(gpio_led_init);
module_exit(gpio_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple driver for controlling GPIO LED using Device Tree");