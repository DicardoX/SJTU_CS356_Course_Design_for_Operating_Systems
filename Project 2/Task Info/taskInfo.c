/** 
 *  This file is created by Chunyu Xue.
 *  Time: 2020/5/26
 */
 
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>

#define BUFFER_SIZE 128
#define PROC_NAME "pid"

static long pid;

static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos);

static struct file_operations proc_ops = {
		.owner = THIS_MODULE,
		.read = proc_read,
		.write = proc_write,
};

int proc_init(void)
{	
	printk(KERN_INFO, "/proc/%s is being created...\n", PROC_NAME);

	/**	Create an entry in the /proc file system */
	proc_create(PROC_NAME, 0666, NULL, &proc_ops);
	
	printk(KERN_INFO, "/proc/%s has been created...\n", PROC_NAME);
	
	return 0;
}

void proc_exit(void)
{
	/**	Remove the entry */ 
	remove_proc_entry(PROC_NAME, NULL);
	
	printk(KERN_INFO, "/proc/%s has been removed...\n", PROC_NAME);
}

static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
	int rv = 0;
	char buffer[BUFFER_SIZE];
	static int completed = 0;
	struct task_struct *task = NULL;
	
	if(completed){
		completed = 0;
		return 0;
	}
	
	completed = 1;
	
	if((task = pid_task(find_vpid(pid), PIDTYPE_PID)) == NULL){
		printk(KERN_INFO, "Error: pid %ld doesn't exist...\n", pid);
		return 0;
	}
	
	rv = sprintf(buffer, "command = %s pid = %d state = %ld\n", task->comm, task->pid, task->state);
	
	if(copy_to_user(usr_buf, buffer, rv) != 0){
		printk(KERN_INFO, "Error: Failed to copy info to user mode...\n");
		rv = -1;
	}
	
	return rv;
}

static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos)
{
	char *k_mem;
	
	k_mem = kmalloc(count, GFP_KERNEL);
	
	if(copy_from_user(k_mem, usr_buf, count) != 0)
	{
		printk(KERN_INFO, "Error: Failed to copy info from user mode...\n");
		return -1;
	}
	
	sscanf(k_mem, "%ld", &pid);	// The string may not be null-terminated, so use sscanf() instead of kstrol()
	
	kfree(k_mem);
	
	return count;
}

module_init( proc_init );
module_exit( proc_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module created by Chunyu Xue.");
MODULE_AUTHOR("Chunyu Xue");
