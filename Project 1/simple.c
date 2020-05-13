#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hash.h>
#include <linux/gcd.h>
#include <asm/param.h>
#include <linux/jiffies.h>

static int startTime, endTime;

/* This function is called when the module is loaded. */
int simple_init(void)
{
    printk(KERN_INFO "Loading Kernel Module\n");
    // Print values of GOLDEN_RATIO_PRIME
    printk(KERN_INFO "The value GOLDEN_RATIO_PRIME  is: %lu\n", GOLDEN_RATIO_PRIME);
    // Record the start time
    startTime = jiffies;
    // Print values of jiffies and HZ
    printk(KERN_INFO "The value of jiffies when init is: %lu\n", jiffies);
    printk(KERN_INFO "The value of HZ is: %lu\n", HZ);
    return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void)
{
    printk(KERN_INFO "Removing Kernel Module\n");
    // Print the result of gcd
    printk("The result of gcd is: %lu\n", gcd(3300, 24));
    // Print values of jiffies when exit
    printk("The value of jiffies when exit is: %lu\n", jiffies);
    // Record the end time
    endTime = jiffies;
    // Print the total number of seconds that have elapsed
    printk("The total elapsing time is: %lu\n", (endTime - startTime) / HZ);
}

/* Macros for registering module entry and exit points. */
module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("Chunyu Xue");
