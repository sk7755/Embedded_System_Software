#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <asm/gpio.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/version.h>

#include "stopwatch.h"

//Global variable
static int dev_driver_port_usage = 0;

static unsigned char *fpga_fnd_addr;
static dev_t stopwatch_dev;
static struct cdev stopwatch_cdev;
static int stopwatch_cont; 			//continue increasing or not
static unsigned int stopwatch_time;			//time information msec
static wait_queue_head_t my_queue;

static int push_flag;
static unsigned int pushed_time; 

struct timer_element{
	struct timer_list timer;
}timer_element;

struct timer_element elt;
struct timer_element exit_elt;

static void stopwatch_timer_function(unsigned long timeout);
static int stopwatch_register_cdev(void);
int stopwatch_open(struct inode *, struct file *);
int stopwatch_release(struct inode *, struct file *);
static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);
static void fpga_fnd_print(unsigned int sec);
static void exit_timer_function(unsigned long timeout);
irqreturn_t inter_handler1(int irq, void *dev_id, struct pt_regs *reg);
irqreturn_t inter_handler2(int irq, void *dev_id, struct pt_regs *reg);
irqreturn_t inter_handler3(int irq, void *dev_id, struct pt_regs *reg);
irqreturn_t inter_handler4(int irq, void *dev_id, struct pt_regs *reg);

struct file_operations stopwatch_fops =
{
	.owner = 		THIS_MODULE,
	.open =			stopwatch_open,
	.write = 		stopwatch_write,
	.release = 		stopwatch_release,

};

//KEY_HOME start
irqreturn_t inter_handler1(int irq, void *dev_id, struct pt_regs *reg)
{
	stopwatch_cont = 1;
	del_timer_sync(&elt.timer);
	elt.timer.expires = get_jiffies_64() + HZ / SEC_INTERVAL;
	elt.timer.data = (unsigned long)&elt;
	elt.timer.function = stopwatch_timer_function;
	add_timer(&elt.timer);

	return IRQ_HANDLED;
}

//KEY_BACK pause
irqreturn_t inter_handler2(int irq, void *dev_id, struct pt_regs *reg)
{
	stopwatch_cont = 1 - stopwatch_cont;
	if(stopwatch_cont == 1){
		del_timer_sync(&elt.timer);
		elt.timer.expires = get_jiffies_64() + HZ / SEC_INTERVAL;
		elt.timer.data = (unsigned long)&elt;
		elt.timer.function = stopwatch_timer_function;
		add_timer(&elt.timer);
	}

	return IRQ_HANDLED;
}

//KEY_VOLUMEUP reset
irqreturn_t inter_handler3(int irq, void *dev_id, struct pt_regs *reg)
{
	stopwatch_time = 0;
	fpga_fnd_print(stopwatch_time / SEC_INTERVAL);
	return IRQ_HANDLED;
}

//KEY_VOLUMEDOWN exit
irqreturn_t inter_handler4(int irq, void *dev_id, struct pt_regs *reg)
{
	push_flag = 1 - push_flag;
	if(push_flag == 1){
		del_timer_sync(&exit_elt.timer);
		exit_elt.timer.expires = get_jiffies_64() + HZ / SEC_INTERVAL;
		exit_elt.timer.data = (unsigned long)&elt;
		exit_elt.timer.function = exit_timer_function;
		pushed_time = 0;
		add_timer(&exit_elt.timer);
	}

	return IRQ_HANDLED;
}

int stopwatch_open(struct inode *minode, struct file *mfile)
{
	int ret;
	int irq;
	
	if(dev_driver_port_usage != 0)
		return -EBUSY;
	dev_driver_port_usage = 1;

	stopwatch_cont = 0;
	stopwatch_time = 0;
	push_flag = 0;
	fpga_fnd_print(stopwatch_time / SEC_INTERVAL);
	// int1
	gpio_direction_input(IMX_GPIO_NR(1,11));
	irq = gpio_to_irq(IMX_GPIO_NR(1,11));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler1,IRQF_TRIGGER_FALLING,"HOME",NULL);

	// int2
	gpio_direction_input(IMX_GPIO_NR(1,12));
	irq = gpio_to_irq(IMX_GPIO_NR(1,12));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler2,IRQF_TRIGGER_FALLING,"BACK",NULL);

	// int3
	gpio_direction_input(IMX_GPIO_NR(2,15));
	irq = gpio_to_irq(IMX_GPIO_NR(2,15));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler3,IRQF_TRIGGER_FALLING,"VOL_UP",NULL);

	// int4
	gpio_direction_input(IMX_GPIO_NR(5,14));
	irq = gpio_to_irq(IMX_GPIO_NR(5,14));
	printk(KERN_ALERT "IRQ Number : %d\n",irq);
	ret=request_irq(irq,inter_handler4,IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,"VOL_DOWN_PUSH",NULL);

	init_waitqueue_head(&my_queue);

	return 0;
}


int stopwatch_release(struct inode *minode, struct file *mfile)
{
	dev_driver_port_usage = 0;
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	return 0;
}

static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	interruptible_sleep_on(&my_queue);
	return 0;
}

static void fpga_fnd_print(unsigned int sec)
{
	int i;
	int min = sec / 60;
	unsigned short int fnd_value = 0;
	int number =0;
	sec %= 60;
	 
	number = min * 100 + sec;
	for(i = 1000; i> 0; i/=10){
		fnd_value = (fnd_value << 4) + (number / i);
		number %= i;
	}
	outw(fnd_value,(unsigned int)fpga_fnd_addr);
}

static void stopwatch_timer_function(unsigned long timeout)
{
//	struct timer_element *p_data = (struct timer_element *)timeout;

	if(stopwatch_cont == 0){	//exit condition
		return;
	}
	//update time and print fnd

	stopwatch_time = (stopwatch_time + 1) % STOPWATCH_BOUND;

	if(stopwatch_time % SEC_INTERVAL == 0)
		fpga_fnd_print(stopwatch_time / SEC_INTERVAL);

	elt.timer.expires = get_jiffies_64() + HZ/SEC_INTERVAL;
	elt.timer.data = (unsigned long)&elt;
	elt.timer.function = stopwatch_timer_function;

	add_timer(&elt.timer);
}

static void exit_timer_function(unsigned long timeout)
{
//	struct timer_element *p_data = (struct timer_element *)timeout;

	if(push_flag == 1){
		if(pushed_time >= 3 * SEC_INTERVAL){
			__wake_up(&my_queue,1,1,NULL);	
			del_timer_sync(&elt.timer);
			fpga_fnd_print(0);
			return;
		}
	}
	else
		return;

	pushed_time = (pushed_time + 1 ) % STOPWATCH_BOUND;
	
	exit_elt.timer.expires = get_jiffies_64() + HZ/SEC_INTERVAL;
	exit_elt.timer.data = (unsigned long)&exit_elt;
	exit_elt.timer.function = exit_timer_function;

	add_timer(&exit_elt.timer);
}

static int stopwatch_register_cdev(void)
{
	int error;
	stopwatch_dev = MKDEV(STOPWATCH_MAJOR, 0);
	error = register_chrdev_region(STOPWATCH_MAJOR, 1,STOPWATCH_NAME);
	if(error < 0){
		printk(KERN_WARNING"Can't get any major\n");
		return error;
	}

	cdev_init(&stopwatch_cdev, &stopwatch_fops);
	stopwatch_cdev.owner = THIS_MODULE;
	stopwatch_cdev.ops = &stopwatch_fops;
	error = cdev_add(&stopwatch_cdev, stopwatch_dev,1);
	if(error){
		printk(KERN_NOTICE "Stopwatch Register Error %d\n",error);
		return error;
	}
	return 0;
}

int __init stopwatch_init(void)
{
	int result;
	if((result = stopwatch_register_cdev()) < 0)
		return result;

	fpga_fnd_addr = ioremap(FPGA_FND_ADDRESS,0x04);

	printk("init module, %s major number : %d\n", STOPWATCH_NAME, STOPWATCH_MAJOR);

	init_timer(&(elt.timer));
	init_timer(&(exit_elt.timer));

	return 0;
}

void __exit stopwatch_exit(void)
{
	cdev_del(&stopwatch_cdev);
	iounmap(fpga_fnd_addr);
	del_timer_sync(&elt.timer);
	del_timer_sync(&exit_elt.timer);
	unregister_chrdev(STOPWATCH_MAJOR, STOPWATCH_NAME);
}

module_init(stopwatch_init);
module_exit(stopwatch_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
