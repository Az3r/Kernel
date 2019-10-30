#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "random_generator"
#define CLASS_NAME "rg"

static struct class *cl;          // Global variable for the device class
static struct device *dev = NULL; ///< The device-driver device struct pointer

static int major;
static int nDeviceOpen = 0;
static int nGeneratedNumber = 0;
static char message[512] = {0};
static int size_of_message = 0;

static int generate(void);
static int my_open(struct inode *i, struct file *f);
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *offset);

static struct file_operations pugs_fops =
    {
        .owner = THIS_MODULE,
        .open = my_open,
        .read = my_read};

static int __init my_init(void)
{
    printk(KERN_INFO "RandomGenerator: Initializing the LKM\n");

    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    major = register_chrdev(0, DEVICE_NAME, &pugs_fops);
    if (major < 0)
    {
        printk(KERN_ALERT "RandomGenerator failed to register a major number\n");
        return major;
    }
    printk(KERN_INFO "RandomGenerator: registered correctly with major number %d\n", major);

    // Register the device class
    cl = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(cl))
    { // Check for error and clean up if there is
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "RandomGenerator: Failed to register device class\n");
        return PTR_ERR(cl); // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "RandomGenerator: device class registered correctly\n");

    // Register the device driver
    dev = device_create(cl, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(dev))
    {
        // Clean up if there is an error
        class_destroy(cl); // Repeated code but the alternative is goto statements
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "RandomGenerator: Failed to create the device\n");
        return PTR_ERR(dev);
    }
    printk(KERN_INFO "RandomGenerator: device class created correctly\n"); // Made it! device was initialized

    // initialize generator seed
    nGeneratedNumber = generate();
    return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit my_exit(void)
{
    device_destroy(cl, MKDEV(major, 0));   // remove the device
    class_unregister(cl);                  // unregister the device class
    class_destroy(cl);                     // remove the device class
    unregister_chrdev(major, DEVICE_NAME); // unregister the major number
    printk(KERN_INFO "RandomGenerator: Goodbye from the LKM!\n");
}

static int my_open(struct inode *i, struct file *f)
{
    nDeviceOpen++;
    printk(KERN_INFO "Device has been opened for %i times\n", nDeviceOpen);
    printk(KERN_INFO "Generated number: %i\n", generate());
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *offset)
{
    int error_count;

    // copy generated number to message buffer
    size_of_message = sprintf(message, "%i", nGeneratedNumber);

    // copy to user buffer
    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    error_count = copy_to_user(buf, message, size_of_message);

    if (error_count == 0)
    {
        // if true then have success
        printk(KERN_INFO "RandomGenerator: Sent %d characters to the user\n", size_of_message);
        *offset += size_of_message;
        return (size_of_message = 0); // clear the position to the start and return 0
    }
    else
    {
        printk(KERN_INFO "RandomGenerator: Failed to send %d characters to the user\n", error_count);
        return -EFAULT; // Failed -- return a bad address message (i.e. -14)
    }
}
static int generate(void)
{
    get_random_bytes(&nGeneratedNumber, sizeof(nGeneratedNumber));
    return nGeneratedNumber;
}

MODULE_LICENSE("GPL");                                                                                                  ///< The license type -- this affects available functionality
MODULE_AUTHOR("Az3r");                                                                                                  ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux character device driver that generates random number for read and open operations"); ///< The description -- see modinfo
MODULE_VERSION("0.1");
module_init(my_init);
module_exit(my_exit);
