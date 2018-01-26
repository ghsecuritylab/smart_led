#include "echo.h"
#include "led.h"
#include "esp8266.h"
#include "rthw.h"
#include "I2C2.h"
#include "flash.h"

extern struct rt_semaphore send_lock_sem;//��������module�߳�
extern int PORT;

//ȫ�ֱ���
vu8 esp_rst_flag = 0;
vu8 esp_rst_cnt = 0;

//����OTA����
#define REV_DATA_LENGTH 5
#define TICK_DATA_LENGTH 2

//WIFI�йس���
//wifi���ơ�����
#define WIFI_CONNECT_CMD 	"AT+CWJAP_DEF=\"ANTIS1\",\"123456789\"\r\n"

//IP���˿�����
#define WIFI_IP_PORT  "AT+CIPSTART=\"TCP\",\"192.168.1.6\",9999\r\n"

//������λ����ֵ
const vu8 RST_WIFI_NUM = 3;//�������ATָ��δ��Ч������������WIFIģ��
vu8 rst_wifi_num = 0;







 
struct rx_msg
{
    rt_device_t dev;
    rt_size_t   size;
};
 
static struct rt_messagequeue  rx_mq;
static unsigned char uart_rx_buffer[150];
static char msg_pool[1024];

rt_uint32_t debug_length;
unsigned char OTA_buffer[150];
 
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
	
		rt_uint32_t rx_length_tmp = 128;
	
		//��ʼ��LED
//	  rt_hw_led_init();
		//��ʼ��8266
//		esp8826_hw_init();
        // ?RT???????1??
		//��ʼ����λ����
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(ESP_RST_rcc,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = ESP_RST_pin;
    GPIO_Init(ESP_RST_gpio, &GPIO_InitStructure);
		
		GPIO_SetBits(ESP_RST_gpio, ESP_RST_pin);
		
    device = rt_device_find("uart3");
    if (device != RT_NULL)
    {
                           // ?????????????
        rt_device_set_rx_indicate(device, uart_input);
                           // ?????????
        rt_device_open(device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
   
		//��ʼ��8266
		//WIFI���Ӵ�ѭ��
		while(0)
		{
				GPIO_ResetBits(ESP_RST_gpio, ESP_RST_pin);
				rt_thread_delay( RT_TICK_PER_SECOND / 5 );
				GPIO_SetBits(ESP_RST_gpio, ESP_RST_pin);
				
				rt_thread_delay( RT_TICK_PER_SECOND * 5 );//��λ����ʱһ��ʱ��		
				
				rt_thread_delay( RT_TICK_PER_SECOND/2 );
				
				rx_length_tmp = 128;
				rt_kprintf("AT\r\n");
				rt_thread_delay( RT_TICK_PER_SECOND/5 );
				result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
				if (result == RT_EOK)
				{
						rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
						uart_rx_buffer[rx_length_tmp] = '\0';
				}
				while(rx_length_tmp < 2 || uart_rx_buffer[rx_length_tmp - 4] != 'O' || uart_rx_buffer[rx_length_tmp - 3] != 'K')
				{
						rt_kprintf("AT\r\n");
						rt_thread_delay( RT_TICK_PER_SECOND/5 );
						result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
						if (result == RT_EOK)
						{
								rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
								uart_rx_buffer[rx_length_tmp] = '\0';
						}
				}
				
				rst_wifi_num = 0;
				rt_kprintf("AT+CWMODE_DEF=1\r\n");
				rt_thread_delay( RT_TICK_PER_SECOND/5 );
				result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
				if (result == RT_EOK)
				{
						rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
						uart_rx_buffer[rx_length_tmp] = '\0';
				}
				while(rx_length_tmp < 2 || uart_rx_buffer[rx_length_tmp - 4] != 'O' || uart_rx_buffer[rx_length_tmp - 3] != 'K')
				{
						rst_wifi_num++;
						if(rst_wifi_num >= RST_WIFI_NUM)
								break;
						rt_kprintf("AT+CWMODE_DEF=1\r\n");
						rt_thread_delay( RT_TICK_PER_SECOND/5 );
						result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
						if (result == RT_EOK)
						{
								rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
								uart_rx_buffer[rx_length_tmp] = '\0';
						}
				}
				if(rst_wifi_num >= RST_WIFI_NUM)
						continue;
				
				//����·����
				rst_wifi_num = 0;
				rx_length_tmp = 128;
				rt_kprintf(WIFI_CONNECT_CMD);
				rt_thread_delay( RT_TICK_PER_SECOND * 10 );
				result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
				if (result == RT_EOK)
				{
						rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
						uart_rx_buffer[rx_length_tmp] = '\0';
				}
				while(rx_length_tmp < 2 || uart_rx_buffer[rx_length_tmp - 4] != 'O' || uart_rx_buffer[rx_length_tmp - 3] != 'K')
				{
						rst_wifi_num++;
						if(rst_wifi_num >= RST_WIFI_NUM)
								break;
						rt_kprintf(WIFI_CONNECT_CMD);
						rt_thread_delay( RT_TICK_PER_SECOND * 10 );
						result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
						if (result == RT_EOK)
						{
								rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
								uart_rx_buffer[rx_length_tmp] = '\0';
						}
				}
				if(rst_wifi_num >= RST_WIFI_NUM)
						continue;
				
				//����IP���˿ں�
				rst_wifi_num = 0;
				rx_length_tmp = 128;
				rt_kprintf(WIFI_IP_PORT);
				rt_thread_delay( RT_TICK_PER_SECOND * 5 );
				result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
				if (result == RT_EOK)
				{
						rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
						uart_rx_buffer[rx_length_tmp] = '\0';
				}
				while(rx_length_tmp < 2 || uart_rx_buffer[rx_length_tmp - 4] != 'O' || uart_rx_buffer[rx_length_tmp - 3] != 'K')
				{
						rst_wifi_num++;
						if(rst_wifi_num >= RST_WIFI_NUM)
								break;
						rt_kprintf(WIFI_IP_PORT);
						rt_thread_delay( RT_TICK_PER_SECOND * 5 );
						result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
						if (result == RT_EOK)
						{
								rx_length_tmp = 128;
								rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
								uart_rx_buffer[rx_length_tmp] = '\0';
						}
				}		
				if(rst_wifi_num >= RST_WIFI_NUM)
						continue;
				
				rt_kprintf("AT+CIPMODE=1\r\n");
				rt_thread_delay( RT_TICK_PER_SECOND/10 );
			
				rt_kprintf("AT+CIPSEND\r\n");
				rt_thread_delay( RT_TICK_PER_SECOND/10 );
				
				break;
		
		}
		
		rt_sem_release(&send_lock_sem);//�ͷ�һ���ź���
    while(1)
    {
                           // ???????????????????????
        result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
        if (result == -RT_ETIMEOUT)
        {
            // timeout, do nothing
        }
       
        if (result == RT_EOK)
        {
            rt_uint32_t rx_length = 64;
						
            rx_length = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length);
            uart_rx_buffer[rx_length] = '\0';
									
						if((uart_rx_buffer[0] == 0x55) && (uart_rx_buffer[1] == 0xaa))//���յ�OTA���ݰ�
						{
								esp_rst_cnt = 0; //������������
							
								//���OTAֵ�����˱仯��������д��flash
								if(BEIGHBOR != uart_rx_buffer[3] || THREDHOLD != uart_rx_buffer[4])
										WriteFlashOneWord(0, uart_rx_buffer[0] + (uart_rx_buffer[1] << 8) + (uart_rx_buffer[3] << 16) + (uart_rx_buffer[4] << 24));
								//�޸���Ӧ����
								BEIGHBOR = uart_rx_buffer[3];
								THREDHOLD = uart_rx_buffer[4];
								//�ش�����
//								rt_device_write(device, 0, &uart_rx_buffer[0], REV_DATA_LENGTH);
								//OTA���ݻظ�ȷ��
//								rt_kprintf("ota ok,%x,%x\r\n",BEIGHBOR,THREDHOLD);
						}
						else if((rx_length != 0))//���յ�������
						{
								esp_rst_cnt = 0; //������������
						}
						
						while(esp_rst_cnt > 15)//�������10��û���յ���֤�������Ѿ��ж�                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
						{
								esp_rst_flag = 1;//�ر�wifi���Ͳ���
								//������������
								//1.��λwifi����
								GPIO_ResetBits(ESP_RST_gpio, ESP_RST_pin);
								rt_thread_delay( RT_TICK_PER_SECOND / 5 );
								GPIO_SetBits(ESP_RST_gpio, ESP_RST_pin);
								
								rt_thread_delay( RT_TICK_PER_SECOND * 5 );//��λ����ʱһ��ʱ��
								
//								rst_wifi_num = 0;
								rx_length_tmp = 128;
								rt_kprintf("AT\r\n");
								rt_thread_delay( RT_TICK_PER_SECOND/5 );
								result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
								if (result == RT_EOK)
								{
										rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
										uart_rx_buffer[rx_length_tmp] = '\0';
								}
								while(rx_length_tmp < 2 || uart_rx_buffer[rx_length_tmp - 4] != 'O' || uart_rx_buffer[rx_length_tmp - 3] != 'K')
								{
//										rst_wifi_num++;
//										if(rst_wifi_num >= RST_WIFI_NUM)
//												break;
										rt_kprintf("AT\r\n");
										rt_thread_delay( RT_TICK_PER_SECOND/5 );
										result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
										if (result == RT_EOK)
										{
												rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
												uart_rx_buffer[rx_length_tmp] = '\0';
										}
								}
//								if(rst_wifi_num >= RST_WIFI_NUM)
//										continue;
								
								//2.����ATָ������
		//						rt_kprintf("AT+CIPSTART=\"TCP\",\"192.168.2.83\",%d\r\n", PORT);
								rst_wifi_num = 0;
								rx_length_tmp = 128;
								rt_kprintf(WIFI_IP_PORT);
								rt_thread_delay( RT_TICK_PER_SECOND * 5 );
								result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
								if (result == RT_EOK)
								{
										rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
										uart_rx_buffer[rx_length_tmp] = '\0';
								}
								while(rx_length_tmp < 2 || uart_rx_buffer[rx_length_tmp - 4] != 'O' || uart_rx_buffer[rx_length_tmp - 3] != 'K')
								{
										rst_wifi_num++;
										if(rst_wifi_num >= RST_WIFI_NUM)
												break;
										rt_kprintf(WIFI_IP_PORT);
										rt_thread_delay( RT_TICK_PER_SECOND * 5 );
										result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
										if (result == RT_EOK)
										{
												rx_length_tmp = 128;
												rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
												uart_rx_buffer[rx_length_tmp] = '\0';
										}
								}		
								if(rst_wifi_num >= RST_WIFI_NUM)
										continue;
							
								rt_kprintf("AT+CIPMODE=1\r\n");
								rt_thread_delay( RT_TICK_PER_SECOND/10 );
							
								rt_kprintf("AT+CIPSEND\r\n");
								rt_thread_delay( RT_TICK_PER_SECOND/10 );
								
								//������ڻ������ڵķ�����Ϣ
								rx_length_tmp = 128;
								result = rt_mq_recv(&rx_mq, &msg, sizeof(struct rx_msg), 80);
								if (result == RT_EOK)
								{
										rx_length_tmp = rt_device_read(msg.dev, 0, &uart_rx_buffer[0], rx_length_tmp);
										uart_rx_buffer[rx_length_tmp] = '\0';
								}
								
								esp_rst_flag = 0;//��wifi���Ͳ���
								esp_rst_cnt = 0;//����ֵ����
								
								break;
						}

							
        }
				
				
				rt_thread_delay( RT_TICK_PER_SECOND * 3 );
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

