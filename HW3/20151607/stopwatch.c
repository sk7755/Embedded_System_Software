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

//FND device Address
static unsigned char *fpga_fnd_addr;

static dev_t stopwatch_dev;
static struct cdev stopwatch_cdev;


static int stopwatch_cont; 			//continue increasing or not
static unsigned int stopwatch_time;			//time information msec
static wait_queue_head_t my_queue;	//Wait queue

static int push_flag;				//1 : pushed volume down key 0 : release volume down key
static unsigned int pushed_time; 

struct timer_element{
	struct timer_list timer;
}timer_element;

struct timer_element elt;			//stopwatch process timer
struct timer_element exit_elt;		//stopwatch exit condition timer

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

//KEY_HOME start stopwatch
//it is called by kernel if you press the HOME key
//this interrupt handler add timer to timer list which is called every 0.1 second
//return IRQ_HANDLED
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

//KEY_BACK pause stopwatch
//it is called by kernel if you press the BACK key
//this interrupt handler reverse stopwatch_cont variable
//if stopwatch_cont = 1, then add timer to timer list like KEY_HOME interrupt handler
//if stopwatch_cont = 0, then existing timer which is added by start interrupt is deleted
//return IRQ_HANDLED
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

//KEY_VOLUMEUP reset stopwatch
//it is called by kernel if you press the VOLUME_UP key
//this interrupt handler reset stopwatch_time to 0 and print FND
//if there is existing timer, it deal with modified stopwatch time
//return IRQ_HANDLED
irqreturn_t inter_handler3(int irq, void *dev_id, struct pt_regs *reg)
{
	stopwatch_time = 0;
	fpga_fnd_print(stopwatch_time / SEC_INTERVAL);
	return IRQ_HANDLED;
}

//KEY_VOLUMEDOWN exit stopwatch
//it is called by kernel if you press VOLUME_DOWN key
//this interrupt handler reverse push_flag, so push_flag represents push(1) or release(0)
//if pushed_flag = 1, then add exit_timer to timer list, so measures pushed time.
//return IRQ_HANDLED
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

//if application calls the open function, this function is called
//it initialize global variables and set port_usage
//and register irq interrupt hanler to kernel
//finally, initialize wait queue
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


//if application calls the close function, this function is called
//this set port_usage 0 and free registered IRQ
int stopwatch_release(struct inode *minode, struct file *mfile)
{
	dev_driver_port_usage = 0;
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
	free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);
	return 0;
}

//if application calls the write function, this function is called
//this immediately block the process
static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	interruptible_sleep_on(&my_queue);
	return 0;
}

//parameter : sec ( 0 ~ 3600)
//this function convert parameter to min:sec value and print it to fnd device 
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

//this function is called by timer interrupt
//it update stopwatch time
//if 1 second passed, call fpga_fnd_print to print new stopwatch time
//and add new timer to repeat this procedure every 0.1 second
static void stopwatch_timer_function(unsigned long timeout)
{
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

//this function is called by timer interrupt
//it update pushed time
//if pushed flag = 1 and pushed time is greater than 3 minutes, wake bloked process and initialize fnd
//if not, add new timer to repeat this procedure every 0.1 second
static void exit_timer_function(unsigned long timeout)
{
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

//this function register stopwatch device to kernel
//based on device major number and fops register device
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

//this function register stopwatch device
//map FND physical address to kernel virtual address space
//,and initialize two timer which is used by stopwatch
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

//this function unregister stopwatch device
//free FND virtual address soace
//,and delete two timer which is used
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
