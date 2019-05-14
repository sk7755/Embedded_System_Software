#ifndef DEV_DRIVER_H
#define DEV_DRIVER_H

#include <linux/ioctl.h>


#define IOM_DEV_DRIVER_MAJOR 242
#define IOM_DEV_DRIVER_NAME "dev_driver"

#define IOCTL_SET_TIMER _IOR(IOM_DEV_DRIVER_MAJOR, 0, unsigned int)

#define ENCODE_DATA(interval, count, start) ((interval) << 24) + ((count) << 16 ) + (start)
#define DECODE_INTERVAL(data) (data) >> 24
#define DECODE_COUNT(data) ((data) << 8 ) >> 24
#define DECODE_START(data) ((start) << 16) >> 16;

#define DIR_TO_INDEX(dir) (15 * (dir) + 15)/2

#define SHIFT_TEXT_LCD(dir,text_lcd) do{\
	for(int i = DIR_TO_INDEX(dir) ; i != DIR_TO_INDEX(-dir);i -= dir){\
		(text_lcd)[i] = (text_lcd)[i-(dir)]; \
	}\
	(text_lcd)[i] = ' ';\
}while(0)

#define IOM_FPGA_FND_ADDRESS 0x08000004
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090
#define IOM_FPGA_LED_ADDRESS 0x08000016
#define IOM_FPGA_DOT_ADDRESS 0x08000210

#endif
