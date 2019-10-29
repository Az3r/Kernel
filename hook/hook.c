#include <linux/kernel.h>      /* We're doing kernel work */
#include <linux/module.h>      /* Specifically, a module, */
#include <linux/moduleparam.h> /* which will have params */
#include <linux/unistd.h>      /* The list of system calls */
#include <linux/sched.h>       /*contains current - pointer to the current process*/
#include <asm/uaccess.h>       /*read, write permission*/

#define SYS_CALL_TABLE_ADDRESS 0xffffffff82000280

void **system_call_table_addr;

static int nOpenCount = 0;
static int nWriteCount = 0;
static char filename[256] = {0};

asmlinkage int custom_open(const char *file, int flags, int mode);
asmlinkage ssize_t custom_write(int fd, const void *buf, size_t nbytes);

// store origin system call functions
asmlinkage int (*origin_open)(const char *filename, int flags, int mode);
asmlinkage int (*origin_write)(int fd, const void *buf, size_t nbytes);

int make_rw(unsigned long address); //Make page writeable
int make_ro(unsigned long address); //Make the page write protected

static int __init entry_point(void)
{
    printk(KERN_INFO "Az3rHook: hook initializing...\n");

    /*MY sys_call_table address*/
    system_call_table_addr = (void *)SYS_CALL_TABLE_ADDRESS;

    // save original syscall
    origin_open = system_call_table_addr[__NR_open];
    origin_write = system_call_table_addr[__NR_write];

    /*Disable page protection*/
    make_rw((unsigned long)system_call_table_addr);

    /*Change syscall to our syscall function*/
    system_call_table_addr[__NR_open] = custom_open;
    system_call_table_addr[__NR_write] = custom_write;

    printk(KERN_INFO "Az3rHook: hook loaded successfully\n");
    return 0;
}
static void __exit exit_point(void)
{
    printk(KERN_INFO "Az3rHook: hook unloading...\n\n");

    /*Restore original system call */
    system_call_table_addr[__NR_open] = origin_open;
    system_call_table_addr[__NR_write] = origin_write;

    /*Renable page protection*/
    make_ro((unsigned long)system_call_table_addr);

    printk(KERN_INFO "Az3rHook: hook unloaded successfully...\n\n");
}

int make_rw(unsigned long address)
{
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    if (pte->pte & ~_PAGE_RW)
    {
        pte->pte |= _PAGE_RW;
    }
    return 0;
}
int make_ro(unsigned long address)
{
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte & ~_PAGE_RW;
    return 0;
}
asmlinkage int custom_open(const char *file, int flags, int mode)
{
    ++nOpenCount;
    printk(KERN_INFO "Az3rHook: Device has been opened for %i times\n", nOpenCount);

    // store opened file
    sprintf(filename, "%s", file);

    // process name: current->comm
    // process id: (int)task_pid_nr(current)
    printk(KERN_INFO "Az3rHook: syscall = open, file = %s, process's id = %i, process's name = %s\n",
           filename, (int)task_pid_nr(current), current->comm);

    return origin_open(filename, flags, mode);
}
asmlinkage ssize_t custom_write(int fd, const void *buf, size_t nbytes)
{
    ++nWriteCount;
    printk(KERN_INFO "Az3rHook: Device has been written for %i times\n", nWriteCount);

    // process name: current->comm
    // process id: (int)task_pid_nr(current)
    printk(KERN_INFO "Az3rHook: syscall = write, file = %s, bytes written = %lu, process's id = %i, process's name = %s\n",
           filename, nbytes, (int)task_pid_nr(current), current->comm);

    return origin_write(fd, buf, nbytes);
}
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Az3r");
module_init(entry_point);
module_exit(exit_point);