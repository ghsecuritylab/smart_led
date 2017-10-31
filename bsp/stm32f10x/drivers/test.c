#include "test.h"
#include <rthw.h>

/*  ��������4�ֽڶ��� */
ALIGN(RT_ALIGN_SIZE)

/*  ��̬�̵߳� �̶߳�ջ*/
static rt_uint8_t mlx_stack[512];
static rt_uint8_t esp8266_stack[512];
/* ��̬�̵߳� �߳̿��ƿ� */
static struct rt_thread mlx_thread;
static struct rt_thread esp8266_thread;

/* �ź������ƿ� */
struct rt_semaphore lock_sem;

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
		rt_thread_delay( RT_TICK_PER_SECOND * 5 );
	  while(1)
		{
				rt_thread_delay( RT_TICK_PER_SECOND/1 );
			
				rt_sem_release(&lock_sem);//�ͷ�һ���ź���
				
		}
}

void esp8266_thread_entry(void* parameter)
{
		vu8 led_state = 0;	
//		//��ʼ��8266
//		rt_thread_delay( RT_TICK_PER_SECOND/2 );
//		esp8826_hw_init();
	
		
    /* ����ѭ��*/
    while (1)
    {
        /* �����õȴ���ʽ��ȡ�ź���*/
        rt_sem_take(&lock_sem, RT_WAITING_FOREVER);
        /* ���õ��ź����Ժ���п���ִ���������*/
        led_state ^=1;
        if (led_state!=0)
        {
//            rt_hw_led_on(0);
            rt_kprintf(" get semaphore ok, take 1 \r\n");
        }
        else
        {
//            rt_hw_led_off(0);
            rt_kprintf(" get semaphore ok, take 0 \r\n");
        }
    }	

}





