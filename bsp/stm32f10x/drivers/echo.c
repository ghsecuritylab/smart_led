#include "echo.h"
#include "led.h"
#include "esp8266.h"
#include "rthw.h"
#include "I2C2.h"
#include "flash.h"

extern struct rt_semaphore send_lock_sem;//��������mlx�߳�
extern int PORT;

//ȫ�ֱ���
vu8 esp_rst_flag = 0;
vu8 esp_rst_cnt = 0;

//����OTA����
#define REV_DATA_LENGTH 5
#define TICK_DATA_LENGTH 2

 
struct rx_msg
{
    rt_device_t dev;
    rt_size_t   size;
};
 
static struct rt_messagequeue  rx_mq;
static unsigned char uart_rx_buffer[150];
static char msg_pool[1024];
 
// ????????
rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    msg.dev = dev;
    msg.size = size;
   
        // ???????????
    rt_mq_send(&rx_mq, &msg, sizeof(struct rx_msg));
   
    return RT_EOK;
}
 
// ??????
void usr_echo_thread_entry(void* parameter)
{
    struct rx_msg msg;
		static vu8 led_state = 0;
   
    rt_device_t device;
    rt_err_t result = RT_EOK;
	
		//��ʼ��LED
//	  rt_hw_led_init();
		//��ʼ��8266
		rt_thread_delay( RT_TICK_PER_SECOND/2 );
		esp8826_hw_init();
   
        // ?RT???????1??
    device = rt_device_find("uart3");
    if (device != RT_NULL)
    {
                           // ?????????????
        rt_device_set_rx_indicate(device, uart_input);
                           // ?????????
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
   
		rt_sem_release(&send_lock_sem);//�ͷ�һ���ź���
    while(1)
    {
				rt_thread_delay( RT_TICK_PER_SECOND * 3 );
                           // ???????????????????????
        result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
        if (result == -RT_ETIMEOUT)
        {
            // timeout, do nothing
        }
       
        if (result == RT_EOK)
        {
            rt_uint32_t rx_length = REV_DATA_LENGTH;
						
           
//            rx_length = (sizeof(uart_rx_buffer) - 1) > msg.size ?
//                msg.size : sizeof(uart_rx_buffer) - 1;
           
            rx_length = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length);
            uart_rx_buffer[rx_length] = '\0';
            // ????????1
						
						//****************OTA����***********************************
//						if(rx_length != 0)
//								rt_kprintf("rx_length = %d, uart_rx_buffer[1] = %c\r\n", rx_length, uart_rx_buffer[1]);
//            rt_device_write(device, 0, &uart_rx_buffer[0], rx_length);
						if((uart_rx_buffer[0] == 0x55) && (uart_rx_buffer[1] == 0xaa) && (rx_length == REV_DATA_LENGTH))//���յ�OTA���ݰ�
						{
								esp_rst_cnt = 0; //������������
							
								//�޸���Ӧ����
								BEIGHBOR = uart_rx_buffer[3];
								THREDHOLD = uart_rx_buffer[4];
								//������д��flash
								WriteFlashOneWord(0, uart_rx_buffer[0] + (uart_rx_buffer[1] << 8) + (uart_rx_buffer[2] << 16) + (uart_rx_buffer[3] << 24));
						}
						else if((uart_rx_buffer[0] == 'O') && (uart_rx_buffer[1] == 'K') && (rx_length == TICK_DATA_LENGTH))//���յ�������
						{
								esp_rst_cnt = 0; //������������
						}
        }
				
				if(esp_rst_cnt > 10)//�������10��û���յ���֤�������Ѿ��ж�                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
				{
						esp_rst_flag = 1;//�ر�wifi���Ͳ���
						//������������
						//1.��λwifi����
						GPIO_ResetBits(ESP_RST_gpio, ESP_RST_pin);
						rt_thread_delay( RT_TICK_PER_SECOND / 5 );
						GPIO_SetBits(ESP_RST_gpio, ESP_RST_pin);
						
						rt_thread_delay( RT_TICK_PER_SECOND * 5 );//��λ����ʱһ��ʱ��
						//2.����ATָ������
						rt_kprintf("AT+CIPSTART=\"TCP\",\"192.168.1.10\",%d\r\n", PORT);
						rt_thread_delay( RT_TICK_PER_SECOND * 3 );
					
						rt_kprintf("AT+CIPMODE=1\r\n");
						rt_thread_delay( RT_TICK_PER_SECOND/10 );
					
						rt_kprintf("AT+CIPSEND\r\n");
						rt_thread_delay( RT_TICK_PER_SECOND/10 );
				
						esp_rst_flag = 0;//��wifi���Ͳ���
						esp_rst_cnt = 0;//����ֵ����
				}
    }
}
// ?????????
void usr_echo_init( void )
{
    rt_thread_t thread ;
   
    rt_err_t result; 
      // ??????,????????
    result = rt_mq_init(&rx_mq, "mqt", &msg_pool[0], 128 - sizeof(void*), sizeof(msg_pool), RT_IPC_FLAG_FIFO);
   
    if (result != RT_EOK) 
    { 
        rt_kprintf("init message queue failed.\n"); 
        return; 
    } 
    // ??????
    thread = rt_thread_create("devt",
        usr_echo_thread_entry, RT_NULL,
        1024, 14, 50);
    // ??????
    if (thread != RT_NULL)
        rt_thread_startup(thread);

}

