#include "test.h"

/*  ��������4�ֽڶ��� */
ALIGN(RT_ALIGN_SIZE)

/*  ��̬�̵߳� �̶߳�ջ*/
static rt_uint8_t led_stack[512];
static rt_uint8_t esp8266_stack[512];
/* ��̬�̵߳� �߳̿��ƿ� */
static struct rt_thread led_thread;
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


    /* ����led�߳� �� ���ȼ� 16 ��ʱ��Ƭ 5��ϵͳ�δ� */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry, RT_NULL,
                            (rt_uint8_t*)&led_stack[0], sizeof(led_stack), 16, 5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }

    /* ����key�߳� �� ���ȼ� 15 ��ʱ��Ƭ 5��ϵͳ�δ� */
    result = rt_thread_init(&esp8266_thread,
                            "key",
                            esp8266_thread_entry, RT_NULL,
                            (rt_uint8_t*)&esp8266_stack[0], sizeof(esp8266_stack), 15, 5);

    if (result == RT_EOK)
    {
        rt_thread_startup(&esp8266_thread);
    }
    return 0;
}


void led_thread_entry(void* paramete)
{
    vu8 led_state = 0;
    rt_hw_led_init();
    /* ����ѭ��*/
    while (1)
    {
        /* �����õȴ���ʽ��ȡ�ź���*/
        rt_sem_take(&lock_sem, RT_WAITING_FOREVER);
        /* ���õ��ź����Ժ���п���ִ���������*/
        led_state ^=1;
        if (led_state!=0)
        {
            rt_hw_led_on(0);
//            rt_kprintf(" get semaphore ok, led all on \r\n");
        }
        else
        {
            rt_hw_led_off(0);
//            rt_kprintf(" get semaphore ok, led all off \r\n");
        }
    }		
}

void esp8266_thread_entry(void* parameter)
{
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
		esp8826_hw_init();
	
	  while(1)
		{
				rt_thread_delay( RT_TICK_PER_SECOND/1 );
			
				rt_sem_release(&lock_sem);//�ͷ�һ���ź���
				
		}
}





