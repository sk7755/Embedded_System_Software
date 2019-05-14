#include <linux/kernel.h>

asmlinkage long sys_make_data(int interval, int count, int start_number)
{
	return (interval << 24) + (count << 16) + (start_number);
}
