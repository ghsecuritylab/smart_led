
#include "zet07.h"


float CO_value;//COŨ��

//����CO�ʴ�ʽģʽ����
uint8_t CO_ASKMODE[10] = {0xff,0xff,0x01,0x78,0x41,0x00,0x00,0x00,0x00,0x46};
//����CO����ʽģʽ����
uint8_t CO_ACTIVEMODE[10] = {0xff,0xff,0x01,0x78,0x40,0x00,0x00,0x00,0x00,0x47};
//����CO���ջ���
uint8_t CO_RxBuffer[CO_BUFFER_SIZE];

