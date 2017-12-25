#ifndef __ECHO_H__
#define __ECHO_H__


#include "stm32f10x.h"
#include "rtthread.h"

//是否使用红外传感器部分
#define USING_MLX90621

rt_err_t uart_input(rt_device_t dev, rt_size_t size);
void usr_echo_thread_entry(void* parameter);
void usr_echo_init( void );




#endif

