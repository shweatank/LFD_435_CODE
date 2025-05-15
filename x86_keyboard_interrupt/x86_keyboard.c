#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>

// IRQ number for the keyboard
#define KEYBOARD_IRQ 1

// Interrupt handler function
static irqreturn_t techdhaba_keyboard_irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "TechDhaba: Keyboard interrupt received!\n");
    return IRQ_HANDLED; // Successfully handled the interrupt
}

// Module initialization
static int __init techdhaba_keyboard_interrupt_init(void)
{
    int result;
    printk(KERN_INFO "TechDhaba: Initializing the Keyboard Interrupt Driver\n");

    // Request the IRQ line for keyboard interrupt
    result = request_irq(KEYBOARD_IRQ, techdhaba_keyboard_irq_handler, IRQF_SHARED,
                         "techdhaba_keyboard_interrupt_driver", (void *)techdhaba_keyboard_irq_handler);
    if (result) {
        printk(KERN_ERR "TechDhaba: Failed to request IRQ %d, error: %d\n", KEYBOARD_IRQ, result);
        return result;
    }

    printk(KERN_INFO "TechDhaba: Keyboard Interrupt Driver successfully loaded\n");
    return 0;
}

// Module cleanup function
static void __exit techdhaba_keyboard_interrupt_exit(void)
{
    free_irq(KEYBOARD_IRQ, (void *)techdhaba_keyboard_irq_handler); // Release the IRQ
    printk(KERN_INFO "TechDhaba: Keyboard Interrupt Driver unloaded\n");
}

// Register module initialization and exit functions
module_init(techdhaba_keyboard_interrupt_init);
module_exit(techdhaba_keyboard_interrupt_exit);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("TechDhaba Keyboard Interrupt Driver");