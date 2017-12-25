#include "test.h"
#include <rthw.h>
#include "I2C2.h"
#include "echo.h"
#include "led.h"
#include "esp8266.h"


/*  ��������4�ֽڶ��� */
ALIGN(RT_ALIGN_SIZE)

//�����뷢�����ݰ���ص���
#define SENSOR_ID  9
#define SORSOR_DATA_LENGTH 141

//����������
static char sensor_data_buff[150];

/*  ��̬�̵߳� �̶߳�ջ*/
static rt_uint8_t mlx_stack[512];
static rt_uint8_t esp8266_stack[512];
/* ��̬�̵߳� �߳̿��ƿ� */
static struct rt_thread mlx_thread;
static struct rt_thread esp8266_thread;

/* �ź������ƿ� */
struct rt_semaphore lock_sem;
struct rt_semaphore send_lock_sem;//��������mlx�߳�

//ȫ�ֱ���
extern vu8 esp_rst_flag;
extern vu8 esp_rst_cnt;
extern char Binaryzation_8byte[8];
extern short   IMAGE[NROWS][NCOLS];



rt_err_t demo_thread_creat(void)
{
    rt_err_t result;

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
    result = rt_thread_init(&mlx_thread,
                            "mlx",
                            mlx_thread_entry, RT_NULL,
                            (rt_uint8_t*)&mlx_stack[0], sizeof(mlx_stack), 16, 20);

    if (result == RT_EOK)
    {
        rt_thread_startup(&mlx_thread);
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


void mlx_thread_entry(void* paramete)
{	
		/* �����õȴ���ʽ��ȡ�ź���*/
		rt_sem_take(&send_lock_sem, RT_WAITING_FOREVER);
	  while(1)
		{
				rt_thread_delay( RT_TICK_PER_SECOND/1 );
			
				rt_enter_critical();
			
		#if defined(USING_MLX90621)
				MLX90621IR_ReadSead();
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
			
				esp_rst_cnt++; //��λ������������
			
        /* ���õ��ź����Ժ���п���ִ���������*/
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
				//���ʹ��������ݰ�
				rt_device_write(device, 0, &sensor_data_buff[0], SORSOR_DATA_LENGTH);
			
				rt_exit_critical();

    }	

}





