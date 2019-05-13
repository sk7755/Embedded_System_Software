/* Device Driver Control
FILE : dev_driver.c
AUTH : sk7755@naver.com */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

#include "dev_driver.h"

//Global variable
static int dev_driver_port_usage = 0;

static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_text_lcd_addr;
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_dot_addr;

static unsigned char text_lcd_value[33] = "20151607        CHUNG JAE HOON  ";
static struct timer_element{
	struct timer_list timer;
	int count;
}timer_element;

struct timer_element elt;
struct current_number;
struct interval;
struct count;

//define functions

//define file_operations structure
struct file_operations iom_dev_driver_fops =
{
	owner:	THIS_MODULE,
	open:	iom_dev_driver_open,
	
	release:	iom_dev_driver_release,
};

int iom_dev_driver_open(struct inode *minode, struct file *mfile)
{
	if(dev_driver_port_usage != 0)
		return -EBUSY;

	dev_driver_port_usage = 1;
	return 0;
}

int iom_dev_driver_release(struct inode *minode, struct file *mfile)
{
	dev_driver_prot_usage = 0;
	return 0;
}

int iom_dev_driver_ioctl(struct inode *inode, struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	unsigned int data;
	switch(ioctl_num){
		case IOCTL_SET_TIMER:
			data = (unsigned int)ioctl_param;
			interval = IOCTL_GET_INTERVAL(data);
			int count = IOCTL_GET_COUNT(data);
			start_number = IOCTL_GET_START(data);
			
			elt.count = 0;
			
			del_timer_sync(&elt.timer);
			
			elt.timer.expires = get_jiffies_64() + (interval * HZ / 10);
			elt.timer.data = (unsigned long)&elt;
			elt.timer.function = iom_fpga_blink;

			add_timer(&elt.timer);
			break;
	}
}

static void iom_fpga_blink(unsigned long timeout)
{
	struct timer_element *p_data = (struct timer_element *)timeout;

	p_data->count++;
	if(p_data->count > count)
		return;

	int i;
	for(i = 1000; i>0;i/=10)
	{
		if(current_number/i){
			current_number += i ;
			break;
		}
	}
	current_number = (current_number - i) % (i * 8) + i;
	if(p_data->count % 8 == 0){
		if(current_number < 10)
			current_number *= 1000;
		else
			current_number /= 10;
	}

	//FND write
	unsigned short int fnd_value = 0;
	int shift_count = 12;
	for(i = 1000;i >0; i/=10, shift_count -= 4)
	{
		if(current_number / i){
			fnd_value = (current_number / i) << shift_count;
			break;
		}
	}
	outw(fnd_value, (unsigned int)iom_fpga_fnd_addr);

	//LED write
	unsigned short led_value;
	led_value = 0x01 << (current_number / i);	//MODIFICATION !!!!!
	outw(led_value, (unsigned int)iom_fpga_led_addr);

	//TEXT LCD write


	//DOT write
	unsigned short int dot_value;


	mydata.timer.expires = get_jiffies_64() + (interval * HZ / 10);
	mydata.timer.data = (unsigned long)&elt;
	mydata.timer.function = iom_fpga_blink;

	add_timer(&elt.timer);
}


int __init iom_dev_driver_init(void)
{
	int result;
	result = register_chrdev(IOM_DEV_DRIVER_MAJOR, IOM_DEV_DRIVER_NAME, &iom_dev_driver_fops);
	if(result < 0) {
		printk(KERN_WARNING"Can't get any major\n");
		return result;
	}
	iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);
	iom_fpga_led_addr = ioremap(IOM_FPGA_LED_ADDRESS, 0x01)
	iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
	iom_fpga_fnd_addr = ioremap(IOM_FPGA_FND_ADDRESS, 0x04);

	printk("init module, %s major number : %d\n", IOM_DEV_DRIVER_NAME, IOM_DEV_DRIVER_MAJOR);

	init_timer(&(elt.timer));
	return 0;
}

void __exit iom_dev_driver_exit(void)
{
	iounmap(iom_fpga_text_lcd_addr);
	iounmap(iom_fpga_led_addr);
	iounmap(iom_fpga_dot_addr);
	iounmap(iom_fpga_fnd_addr);

	del_timer_sync(&elt.timer);
	unregister_chrdev(IOM_DEV_DRIVER_MAJOR, IO_DEV_DRIVER_NAME);
}

module_init(iom_dev_driver_init);
module_exit(iom_dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huins");
