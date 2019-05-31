Written by Chung Jae Hoon
sk7755@naver.com
This is Homework 3 in Embedded System Software Cource
practice Interrupt Programming, Module programming and Timer interrupt

Driver Name 	:	stopwatch
Major Number 	:	242

1. app folder
make : create app program file
make clean : delete app program file
2. module folder
make : create module kernel file
make clean : delete module kernel file and other tmp files

if you want to test this application, follow the sequences
1. push files "app", "stopwatch.ko" to your ARM i.MX6Q Board
2. Please type "insmod stopwatch.ko" and "mknod /dev/stopwatch c 242 0"
3. Execute app program like this example "./app"
