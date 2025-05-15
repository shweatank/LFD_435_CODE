#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Example Linux Device Driver Using Workqueue");

/* Define a custom structure for passing data to the workqueue */
struct work_data {
    struct work_struct work; /* Work structure */
    int value;               /* Example data to process */
};

/* Declare a custom workqueue */
static struct workqueue_struct *my_workqueue;

/* Workqueue handler function */
static void work_handler(struct work_struct *work)
{
    struct work_data *data = container_of(work, struct work_data, work);

    /* Process the work */
    pr_info("Workqueue: Processing value = %d\n", data->value);

    /* Simulate a time-consuming operation */
    msleep(2000);

    pr_info("Workqueue: Completed processing value = %d\n", data->value);

    /* Free the work_data memory */
    kfree(data);
}

/* Module initialization function */
static int __init workqueue_example_init(void)
{
    struct work_data *data;

    pr_info("Workqueue Example: Module loaded\n");

    /* Create a custom workqueue */
    my_workqueue = create_workqueue("my_workqueue");
    if (!my_workqueue) {
        pr_err("Workqueue Example: Failed to create workqueue\n");
        return -ENOMEM;
    }

    /* Allocate memory for work_data */
    data = kmalloc(sizeof(struct work_data), GFP_KERNEL);
    if (!data) {
        destroy_workqueue(my_workqueue);
        pr_err("Workqueue Example: Failed to allocate memory\n");
        return -ENOMEM;
    }

    /* Initialize the work structure and set value */
    INIT_WORK(&data->work, work_handler);
    data->value = 42; /* Example value */

    /* Queue the work */
    queue_work(my_workqueue, &data->work);

    pr_info("Workqueue Example: Work queued\n");

    return 0;
}

/* Module exit function */
static void __exit workqueue_example_exit(void)
{
    /* Destroy the workqueue */
    if (my_workqueue) {
        flush_workqueue(my_workqueue);
        destroy_workqueue(my_workqueue);
    }

    pr_info("Workqueue Example: Module unloaded\n");
}

module_init(workqueue_example_init);
module_exit(workqueue_example_exit);