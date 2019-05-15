Written by Chung Jae Hoon
sk7755@naver.com
This is Homework 2 in Embedded System Software Cource
practice System call Programming, Module programming and Timer interrupt

Driver Name 	:	dev_driver
Major Number 	:	242

1. app folder
make : create app program file
make clean : delete app program file
2. module folder
make : create module kernel file
make clean : delete module kernel file and other tmp files
3. kernel folder
this folder have same folder structure with original kernel folder "achroimx_kernel"
difference is that this folder has files which is modified
thus you should replace original file with this files in your ahcroimx_kernel folder

if you want to test this application, follow the sequences
1. rebuild kernel, make boot image, and replace boot image by using fastboot
2. push files "app", "dev_driver.ko" to your ARM i.MX6Q Board
3. Please type "insmod dev_driver.ko" and "mknod /dev/dev_driver c 242 0"
4. Execute app program like this example "./app 1 50 1000"
