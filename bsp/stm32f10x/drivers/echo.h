#ifndef __ECHO_H__
#define __ECHO_H__


#include "stm32f10x.h"
#include "rtthread.h"

////�Ƿ�ʹ�ú��⴫��������
//#define USING_MLX90621

////�Ƿ�ʹ��CO����������
//#define USING_CO_ZE07

////�Ƿ�ʹ����ʪ�ȴ�����
//#define USING_TS_SHT20

//�Ƿ�ʹ����������������
#define USING_VOICE

//�Ƿ�ʹ��������������
#define USING_SMOKE_FOG

//�Ƿ�ʹ�ö������崫��������
#define USING_TOSIC_GAS

rt_err_t uart_input(rt_device_t dev, rt_size_t size);
void usr_echo_thread_entry(void* parameter);
void usr_echo_init( void );




#endif

