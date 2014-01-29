#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/sched.h> 

MODULE_LICENSE("GPL");  

#define MAJOR_NUM 232

typedef struct {                                                        
	int slot;
	int off;
	void *data;                      
	int size;       
} param_t ;

void *base[2];

static long hello_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	param_t p;
	long r;

	switch (cmd) {
	case 0: // read
		r = copy_from_user(&p, (void __user *)arg, sizeof(p));
		r = copy_to_user(p.data, base[p.slot]+p.off, p.size);
		break;
	case 1: // write
		r = copy_from_user(&p, (void __user *)arg, sizeof(p));
		r = copy_from_user(base[p.slot]+p.off, p.data, p.size);
		break;
	default:
		r = 0;
	}

	return r;
}

static int hello_open(struct inode *inode, struct file *filp) {
	printk(KERN_NOTICE "Hello device open!\n");
	return 0;
}

static DECLARE_WAIT_QUEUE_HEAD(irq_queue); 
static __u32 irqstat;

static int hello_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	int r;
	interruptible_sleep_on(&irq_queue);
	if (count >= 4)
		r = copy_to_user(buf, &irqstat, 4);
	return count;
}

static int hello_release(struct inode *inode, struct file *filp) {
	printk(KERN_NOTICE "Hello device close!\n");
	return 0;
}

static struct file_operations hello_fops = {
	.open     = hello_open,
	.read     = hello_read,
	.unlocked_ioctl    = hello_ioctl,
	.release  = hello_release,
};

static irqreturn_t irq_handler(int irq, void *devid) {
	__u32 status = 0;
	void *reg = base[0] + 0x214;
	int i;

	status = *(__u32 *)(reg);
	for (i = 0; i < 32; i++) {
		if (status & (1<<i)) {
			*(__u32 *)(reg) = 1<<i;
		}
	}
	irqstat = status;
	wake_up_interruptible(&irq_queue);

	return IRQ_HANDLED;
}

int init_module(void) {  
	int err;

	if (register_chrdev(MAJOR_NUM, "hello", &hello_fops)) {
		printk(KERN_ALERT "hello init: failed register %d, %d\n", MAJOR_NUM, 0);
		return -EINVAL;
	}
	base[0] = ioremap(0x01c20800, 0x400);
	base[1] = ioremap(0x01c20e00, 0xc);
	printk(KERN_ALERT "hello init: %d, %d, gpio base %p, pwm base %p\n",
			MAJOR_NUM, 0, base[0], base[1]);

	// SW_INT_IRQNO_PIO = 28
	err = request_irq(28, irq_handler,
			IRQF_SHARED, "sunxi-gpio", (void *)0x1653);
	if (err != 0) {
		printk(KERN_ALERT "hello init: request irq failed: %d\n", err);
		return -EINVAL;
	}

	return 0;  
}  

void cleanup_module(void) {  
	unregister_chrdev(MAJOR_NUM, "hello");
	printk(KERN_ERR "Leave hello module!\n");  
}  

MODULE_AUTHOR("Mike Feng");  
MODULE_DESCRIPTION("This is hello module");  
MODULE_ALIAS("A simple example");  

