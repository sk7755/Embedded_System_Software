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

static unsigned char text_lcd_buffer[33] = "20151607        CHUNG JAE HOON  ";
static int first_dir = 1;
static int second_dir = 1;


igned char fpga_number[10][10] = {
	{0x3e,0x7f,0x63,0x73,0x73,0x6f,0x67,0x63,0x7f,0x3e}, // 0
	{0x0c,0x1c,0x1c,0x0c,0x0c,0x0c,0x0c,0x0c,0x0c,0x1e}, // 1
	{0x7e,0x7f,0x03,0x03,0x3f,0x7e,0x60,0x60,0x7f,0x7f}, // 2
 	{0xfe,0x7f,0x03,0x03,0x7f,0x7f,0x03,0x03,0x7f,0x7e}, // 3
	{0x66,0x66,0x66,0x66,0x66,0x66,0x7f,0x7f,0x06,0x06}, // 4
	{0x7f,0x7f,0x60,0x60,0x7e,0x7f,0x03,0x03,0x7f,0x7e}, // 5
	{0x60,0x60,0x60,0x60,0x7e,0x7f,0x63,0x63,0x7f,0x3e}, // 6
	{0x7f,0x7f,0x63,0x63,0x03,0x03,0x03,0x03,0x03,0x03}, // 7
	{0x3e,0x7f,0x63,0x63,0x7f,0x7f,0x63,0x63,0x7f,0x3e}, // 8
	{0x3e,0x7f,0x63,0x63,0x7f,0x3f,0x03,0x03,0x03,0x03} // 9
};


static struct timer_element{
	struct timer_list timer;
	int count;
}timer_element;

struct timer_element elt;
struct current_number;
struct interval;
struct count;

//define functions
int iom_dev_driver_open(struct inode *, struct file *);
int iom_dev_driver_release(struct inode *, struct file *);
long iom_dev_driver_ioctl(struct file *, unsigned int, unsigned long);
static void iom_fpga_blink(unsigned long);

//define file_operations structure
struct file_operations iom_dev_driver_fops =
{
	.owner =			THIS_MODULE,
	.open =				iom_dev_driver_open,
	.unlocked_ioctl = 	iom_dev_driver_ioctl,
	.release =			iom_dev_driver_release,
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

long iom_dev_driver_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
	unsigned int data;
	switch(ioctl_num){
		case IOCTL_SET_TIMER:
			data = (unsigned int)ioctl_param;
			interval = DECODE_INTERVAL(data);
			int count = DECODE_COUNT(data);
			start_number = DECODE_START(data);
			
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

	int i,j ;
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
	if(text_lcd_buffer[DIR_TO_INDEX(first_dir)] != ' ')
		first_dir = -first_dir;
	if(text_lcd_buffer[DIR_TO_INDEX(second_dir) + 16] != ' ')
		second_dir = -second_dir;

	SHIFT_TEXT_LCD(first_dir, text_lcd_buffer);
	SHIFT_TEXT_LCD(second_dif, text_lcd_buffer + 16);
	unsigned short int text_lcd_value;

	for(j = 0 ; j < 33 ; j+=2)
		text_lcd_value = (text_lcd_buffer[j] & 0xFF) << 8 | (text_lcd_buffer[i + 1] & 0xFF);
	outw(text_lcd_value,(unsigned int)iom_fpga_text_lcd_addr + j);

	//DOT write
	unsigned short int dot_value;
	int length = sizeof(fpga_number[current_number/i]);
	for(j = 0; j < length;j++)
	{
		dot_value = fpga_number[current_number/i][j] & 0x7F;
		outw(dot_value,(unsigned int)iom_fpga_dot_addr + j*2);
	}


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
