#include "test.h"
#include <rthw.h>
#include "I2C2.h"
#include "echo.h"
#include "led.h"
#include "esp8266.h"
#include "zet07.h"
#include "bsp_sht2x.h"

/*  ��������4�ֽڶ��� */
ALIGN(RT_ALIGN_SIZE)

//�����뷢�����ݰ���ص���
#define SENSOR_ID  30
#define SORSOR_DATA_LENGTH 141

//����������
static char sensor_data_buff[150];

/*  ��̬�̵߳� �̶߳�ջ*/
static rt_uint8_t module_stack[512];
static rt_uint8_t esp8266_stack[512];
/* ��̬�̵߳� �߳̿��ƿ� */
static struct rt_thread module_thread;
static struct rt_thread esp8266_thread;

/* �ź������ƿ� */
struct rt_semaphore lock_sem;
struct rt_semaphore send_lock_sem;//��������mlx�߳�

//ȫ�ֱ���
extern vu8 esp_rst_flag;
extern vu8 esp_rst_cnt;
extern char Binaryzation_8byte[8];
extern short   IMAGE[NROWS][NCOLS];

#if defined(USING_TS_SHT20)//���ʹ����ʪ�ȴ�����
//��ʪ�ȴ������ṹ�嶨��
SHT_DATA_TYPE SHTxxVal;
#endif

struct CO_rx_msg
{
    rt_device_t dev;
    rt_size_t   size;
};
 
static struct rt_messagequeue  CO_rx_mq;
static unsigned char CO_uart_rx_buffer[150];
static char CO_msg_pool[1024];

rt_err_t CO_uart_input(rt_device_t dev, rt_size_t size)
{
    struct CO_rx_msg CO_msg;
    CO_msg.dev = dev;
    CO_msg.size = size;
   
        // ???????????
    rt_mq_send(&CO_rx_mq, &CO_msg, sizeof(struct CO_rx_msg));
   
    return RT_EOK;
}


rt_err_t demo_thread_creat(void)
{
    rt_err_t result;
		//��ʼ������CO����Ϣ����
		result = rt_mq_init(&CO_rx_mq, "CO_mqt", &CO_msg_pool[0], 128 - sizeof(void*), sizeof(CO_msg_pool), RT_IPC_FLAG_FIFO);
	
		if (result != RT_EOK) 
    { 
        rt_kprintf("init message queue failed.\n"); 
        return -1; 
    }
    /* ��ʼ����̬�ź�������ʼֵ��0 */
    result = rt_sem_init(&lock_sem, "locksem", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init locksem failed.\n");
        return -1;
    }
		
    /* ��ʼ����̬�ź�������ʼֵ��0 */
    result = rt_sem_init(&send_lock_sem, "sendlocksem", 0, RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init sendlocksem failed.\n");
        return -1;
    }		

    /* ����mlx�߳� �� ���ȼ� 16 ��ʱ��Ƭ 5��ϵͳ�δ� */
    result = rt_thread_init(&module_thread,
                            "module",
                            module_thread_entry, RT_NULL,
                            (rt_uint8_t*)&module_stack[0], sizeof(module_stack), 16, 20);

    if (result == RT_EOK)
    {
        rt_thread_startup(&module_thread);
    }
		
    /* ����esp8266�߳� �� ���ȼ� 15 ��ʱ��Ƭ 5��ϵͳ�δ� */
    result = rt_thread_init(&esp8266_thread,
                            "esp8266",
                            esp8266_thread_entry, RT_NULL,
                            (rt_uint8_t*)&esp8266_stack[0], sizeof(esp8266_stack), 15, 20);

    if (result == RT_EOK)
    {
        rt_thread_startup(&esp8266_thread);
    }
    return 0;
}


void module_thread_entry(void* paramete)
{	
	#if defined(USING_CO_ZE07)//���ʹ��CO������--����2
		uint8_t CO_ASK[9]={0xff,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};//�ʴ�ʽѯ�ʱ���
		rt_device_t device;
		rt_err_t result = RT_EOK;
		struct CO_rx_msg CO_msg;
		
		device = rt_device_find("uart2");
    if (device != RT_NULL)
    {
                           // ?????????????
        rt_device_set_rx_indicate(device, CO_uart_input);
                           // ?????????
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }		
	rt_thread_delay( RT_TICK_PER_SECOND * 10 );
	#endif
		/* �����õȴ���ʽ��ȡ�ź���*/
		rt_sem_take(&send_lock_sem, RT_WAITING_FOREVER);
	#if defined(USING_CO_ZE07)//CO��ȡ����
//		rt_device_write(device, 0, &CO_ACTIVEMODE[0], CO_BUFFER_SIZE+1);
//		rt_thread_delay( RT_TICK_PER_SECOND/10 );
//		rt_device_write(device, 0, &CO_ACTIVEMODE[1], CO_BUFFER_SIZE);
	#endif
	  while(1)
		{
				rt_thread_delay( RT_TICK_PER_SECOND/1 );
			
				rt_enter_critical();
			
		#if defined(USING_MLX90621)//�����ȡ����
				MLX90621IR_ReadSead();
		#endif
			
		#if defined(USING_CO_ZE07)//CO��ȡ����
				//�����ʴ�ʽ��ʽ��ȡ����
				//1.����ͨ�����ڷ����ʸ�ʽ����
//				rt_device_write(device, 0, &CO_ASK[0], CO_BUFFER_SIZE);
			
				//2.��ʱһС��ʱ��
				rt_thread_delay( RT_TICK_PER_SECOND/10 );//��ʱ20ms   1000/50==20ms
			
				//3.��ȡ�����������CO����
				result = rt_mq_recv(&CO_rx_mq, &CO_msg, sizeof(struct CO_rx_msg), 80);
				if (result == -RT_ETIMEOUT)
        {
            // timeout, do nothing
        }
				
				if (result == RT_EOK)
				{
						rt_uint32_t rx_length = 64;
						rt_uint8_t cnt;
						rx_length = rt_device_read(CO_msg.dev, 0, &CO_uart_rx_buffer[0], rx_length);
						CO_uart_rx_buffer[rx_length] = '\0';
						
						for(cnt = 0; cnt < rx_length; cnt++)
						{
								if(CO_uart_rx_buffer[cnt] != 0xFF)	continue;//�������0xff������
								else if(CO_uart_rx_buffer[cnt+1] == 0x04 && CO_uart_rx_buffer[cnt+7] == 0x88)//���Ϊ��ʽ����
								{
										CO_value = (CO_uart_rx_buffer[cnt+4]*256 + CO_uart_rx_buffer[cnt+5])*0.1;//����õ�CO��ֵ
										break;
								}
						}
//						if(CO_uart_rx_buffer[0] == 0xFF && CO_uart_rx_buffer[1] == 0x04 && CO_uart_rx_buffer[7] == 0x88)//�ж��Ƿ�Ϊ����CO���ݰ�
//						{
//								CO_value = (CO_uart_rx_buffer[4]*256 + CO_uart_rx_buffer[5])*0.1;//����õ�CO��ֵ
//						}
				}
		#endif
				
		#if defined(USING_TS_SHT20)//���ʹ����ʪ�ȴ�����
				Bsp_ShtMeasureOnce(&SHTxxVal);
		#endif
			
				rt_exit_critical();
				if(!esp_rst_flag)
						rt_sem_release(&lock_sem);//�ͷ�һ���ź���
				
		}
}

void esp8266_thread_entry(void* parameter)
{
		vu8 cnt = 0;

    rt_device_t device;

		device = rt_device_find("uart3");
    if (device != RT_NULL)
    {
                           // ?????????????
        rt_device_set_rx_indicate(device, uart_input);
                           // ?????????
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
    /* ����ѭ��*/
    while (1)
    {
        /* �����õȴ���ʽ��ȡ�ź���*/
        rt_sem_take(&lock_sem, RT_WAITING_FOREVER);
			
				rt_enter_critical();//����������
			
//				esp_rst_cnt++; //��λ������������
			
        /* ���õ��ź����Ժ���п���ִ���������*/
			
		#if defined(USING_MLX90621)//�����ȡ����
				//������ݰ�����
				for(cnt = 0; cnt < SORSOR_DATA_LENGTH; cnt++)
				{
						if(cnt == 0)
								sensor_data_buff[cnt] = SENSOR_ID / 10 + '0';
						else if(cnt == 1)
								sensor_data_buff[cnt] = SENSOR_ID % 10 + '0';
						else if(cnt == 2)
								sensor_data_buff[cnt] = SORSOR_DATA_LENGTH;
						else if(cnt < 11)
								sensor_data_buff[cnt] = Binaryzation_8byte[cnt - 3];
						else if(cnt < 139)
						{
								if(cnt % 2 == 1)
										sensor_data_buff[cnt] = *(&IMAGE[0][0] + (cnt - 1) / 2 - 5) / 256;
								else
										sensor_data_buff[cnt] = *(&IMAGE[0][0] + (cnt - 1) / 2 - 5) % 256;
						}		
						else if(cnt == 139)
								sensor_data_buff[cnt] = 0x5a;
						else if(cnt == 140)
								sensor_data_buff[cnt] = 0xa5;
								
				}
				sensor_data_buff[SORSOR_DATA_LENGTH] = '\0';
				//���ͺ��⴫�������ݰ�
				rt_device_write(device, 0, &sensor_data_buff[0], SORSOR_DATA_LENGTH);
		#endif
			
		#if defined(USING_CO_ZE07)
				//������ݰ�����
				
		#endif
				
				rt_exit_critical();

    }	

}





