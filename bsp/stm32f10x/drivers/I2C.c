

/* Includes ------------------------------------------------------------------*/
#include "I2C2.h"
#include "I2C_init.h"
#include "string.h"
#include "math.h"

unsigned char BEIGHBOR = 1;
unsigned char THREDHOLD = 20;

//���Ȳ�������
#define ALL_ZERO  1     //ȫ0 ����û��
#define NOT_ALL_ZERO 2  //��1 ��������
#define DATA_CHANDE     3//���ݺ��ϴ��б仯

void BackgroundInit(void);
//extern void USART2_Send_IRdata(void);
//extern void USART_Send_IRdata2(void);
//extern void USART2_Send_Byte(u16 Data);
//extern	void delay_ms(unsigned int nTime);
/* Private variables ---------------------------------------------------------*/
u8 u8stCmd[5];
u8 u8Data_eeprom[256];
u8 u8OscValue[2];
u8 u8ConfigValue[2];
u8 u8PTATValue[2];
u8 u8IRValue[128];
u8 u8CompstPiexl[2];

char  IR_MASK = 0x0B;
extern  u8 Res;
double  Ai[NROWS][NCOLS];
double  Bi[NROWS][NCOLS];
double  Alpha[NROWS][NCOLS];
float   TA = 26;
short   IMAGE[NROWS][NCOLS];  // temperature data image
short   BackgroundTemp[NROWS][NCOLS];
short   NewMinusOld[NROWS][NCOLS];
short   Binaryzation[NROWS][NCOLS];
char Time_cnt = 0;//���ƶ�÷�һ��
char Time_flag = 0;//���Ȳ�������

char last_Binaryzation_8byte[8] = {0};

char Binaryzation_8byte[8] = {0};
// for Ta coeff
unsigned int Vth = 0;
unsigned  int Kt1 = 0;
unsigned int Kt2 = 0;
double VTHF = 0;
double KT1F = 0;
double KT2F = 0;   
int scale1,scale2,avr=0;
int min,max;
int avrNum=0;     //��ʼ��20֡���ݵļ�����
short mean=0;     //ͬһ֡�����ж�Ϊ����ʱ�����ص�ĸ����¶ȵ�ƽ��ֵ
//short flag=0;

// fixed coeff for To
double Ke,KsTa_0,alpha;
int resolution = 3;    //  both config bits on, some calcuations need adaptation for this parameter
double kt1Scale,kt2Scale,deltaOSScale,iros_scale,cyclops_alpha;
double Vth0,kt1,kt2,scale_com_alpha,scale_alpha,common_alpha,cyclops_ai,cyclops_bi;
double TGC_0,commonAi,ai,bi,Tobj,cyclopsData;

// variables In Timer
u16 u16PORDelay5ms = 0;
u16 u16PORDelay2ms = 0; // MLX90621_WorkInit   Get_sensor_data

stI2CWR stI2CWriteRead;


void structInit(void)
{
	u8 i;
	
	memset(&stI2CWriteRead,0,sizeof(stI2CWriteRead));
	
	for(i=0;i<9;i++) { stI2CWriteRead.u16OperationDelay[i] = 100*i+100;}

	stI2CWriteRead.bOperationFlag = 10;	
} 



/*******************************************************************************
*  I2C_writeOscTrime
*******************************************************************************/
bool I2C_writeOscTrim(void) //0x0056
{
	u8 lu8WriteAddr=0;
	//u8 lu8OscData = 0;
	
	lu8WriteAddr = 0x60 << 1;  //IIC Slave address 60 HEX
	
	//lu8OscData = u8Data_eeprom[0xf7] > 0xAA ? (u8Data_eeprom[0xf7] - 0xAA) :(u8Data_eeprom[0xf7]+0xFF+0x01-0xAA);
  /*
	u8stCmd[0] = 0x04; 
  u8stCmd[1] = 0xAC;//lu8OscData;    
  u8stCmd[2] = 0x56;//u8Data_eeprom[0xf7];       
  u8stCmd[3] = 0x56;//0xFF+0x01-0xAA;//0x00 - 0xAA;    
  u8stCmd[4] = 0x00;
  */
	/*
	u8stCmd[0] = 0x04; 
  u8stCmd[1] = (u8Data_eeprom[0xf7]) - 0xAA;  //lu8OscData;    
  u8stCmd[2] = (u8Data_eeprom[0xf7]);         //u8Data_eeprom[0xf7];       
  u8stCmd[3] = 0x56;  //0xFF+0x01-0xAA;//0x00 - 0xAA;    
  u8stCmd[4] = 0x00;
*/	
	
	u8stCmd[0] = 0x04; 
  u8stCmd[1] = (u8Data_eeprom[0xf7]& 0x01) - 0xAA;  //lu8OscData;    
  u8stCmd[2] = (u8Data_eeprom[0xf7]& 0x01);         //u8Data_eeprom[0xf7];       
  u8stCmd[3] = (u8Data_eeprom[0xf7]& 0xF0) - 0xAA;  //0xFF+0x01-0xAA;//0x00 - 0xAA;    
  u8stCmd[4] = (u8Data_eeprom[0xf7]& 0xF0);	
  
	return I2C_write(u8stCmd, lu8WriteAddr, 5);
}

/*******************************************************************************
*  I2C_writeDevConfig
*******************************************************************************/  
bool I2C_writeDevConfig(void)    //0x463E
{
	u8 lu8WriteAddr=0;
	//u8 lu8DevConfigL = 0,lu8DevConfigH = 0;
	lu8WriteAddr = 0x60 << 1;  //IIC Slave address 60 HEX
	
	//lu8DevConfigL = u8Data_eeprom[0xf5] > 0x55 ? (u8Data_eeprom[0xf5] - 0x55) :(u8Data_eeprom[0xf5]+0xFF+0x01-0x55);   	
	//lu8DevConfigH = u8Data_eeprom[0xf6] > 0x55 ? (u8Data_eeprom[0xf6] - 0x55) :(u8Data_eeprom[0xf6]+0xFF+0x01-0x55);   	
    
  	u8stCmd[0] = 0x03;                       
    u8stCmd[1] = 0xE6;//lu8DevConfigL;
    u8stCmd[2] = 0x3B;       //ˢ����8HZ �������� 18λADC һ�����ģʽ
    u8stCmd[3] = 0xF1;//lu8DevConfigH;
    u8stCmd[4] = 0x46;
	  
	  /*
		u8stCmd[0] = 0x03;                       
    u8stCmd[1] = 0xE5;//lu8DevConfigL;
    u8stCmd[2] = 0x3A;       
    u8stCmd[3] = 0xF1;//lu8DevConfigH;
    u8stCmd[4] = 0x46;
	  */
	
	return I2C_write(u8stCmd, lu8WriteAddr, 5);
}	

/*******************************************************************************
* I2C_rwad_Ram()
*******************************************************************************/  
bool I2C_read_Ram(u8 lu8AddrSt,          //address
                  u8* lpu8StoreBuffer,   //stroe data
                  u8 lu8Size ,           //size
                  u8 lu8Cmd,             //command
                  u8 lu8ReadStep)        //step

{
	u8 AddrPRead = 0,i;
	AddrPRead = (0x60 << 1);  //IIC Slave address 60 HEX
	if(!I2C_Start()) return FALSE;  // 0
	
	I2C_SendByte(AddrPRead);  //
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	
	I2C_SendByte(lu8Cmd);  //
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	
	I2C_SendByte(lu8AddrSt);   //start Address
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	} 	
	
	I2C_SendByte(lu8ReadStep);  //Address Step
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	} 	

	I2C_SendByte(lu8Size>>1);  //Number of read  
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	

	AddrPRead = (0x60 << 1)|0x01; //
	if(!I2C_Start()) return FALSE;
	
	I2C_SendByte(AddrPRead);   //	
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
		
	for(i=0;i < lu8Size;i++)
	{
		lpu8StoreBuffer[i] = I2C_ReceiveByte();
		I2C_Ack();		
	}
		
	I2C_Stop();
	
	return TRUE;
}

/*******************************************************************************
*I2C_readIRData
*******************************************************************************/  
bool I2C_readIRData(void)
{	  
	return I2C_read_Ram(0x00, u8IRValue, 0x40<<1, 0x02, 0x01);
}

/*******************************************************************************
* I2C_readPixelCompensation
*******************************************************************************/ 
bool I2C_readPixelCompensation(void) 
{	  
	return I2C_read_Ram(0x41, u8CompstPiexl, 2, 0x02, 0x00);
}

/*******************************************************************************
*  I2C_readPTAT
*******************************************************************************/ 
bool I2C_readPTAT(void)
{	  
	return I2C_read_Ram(0x40, u8PTATValue, 2, 0x02, 0x00);
}

/*******************************************************************************
* I2C_readOscTrim
*******************************************************************************/ 
bool I2C_readOscTrim(void)
{	  
	return I2C_read_Ram(0x93, u8OscValue, 2, 0x02, 0x00);
}

/*******************************************************************************
* I2C_readDevConfig
*******************************************************************************/
bool I2C_readDevConfig(void)
{
	return I2C_read_Ram(0x92, u8ConfigValue, 2, 0x02, 0x00);
}

/*******************************************************************************
* I2C_read
*******************************************************************************/
bool I2C_read(u8 lu8Addr, 
              u8* lpu8EepromBuffer, 
              u16 lu16Size,
              u8 lu8Cmd)
{
	if(!I2C_Start()) return FALSE;
	I2C_SendByte(lu8Addr<<1);  
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}
	I2C_SendByte(lu8Cmd);  
	
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}	
	if(!I2C_Start()) return FALSE;
	I2C_SendByte((lu8Addr<<1) | 0x01);  
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return FALSE;
	}	
	while(lu16Size)
	{
		*lpu8EepromBuffer = I2C_ReceiveByte();			
		if(lu16Size == 0x01)I2C_NoAck();			
		else I2C_Ack(); 
		lu16Size--;
		lpu8EepromBuffer++;
	}	
	I2C_Stop();	
	return TRUE;
}
/*******************************************************************************
* I2C_readEeprom
*******************************************************************************/
bool I2C_readEeprom(void)
{
	return I2C_read(0x50, u8Data_eeprom, 256 ,0x00);
}

/*******************************************************************************
*  PrepareCoeff_TA_621 
*******************************************************************************/
void PrepareCoeff_TA_621(void)
{                                                                                                                                           
    scale2 = (int)(u8Data_eeprom[0xd2]&0x0f);         //KT2_SCALE in EEPROM address 0xD2[3:0]                            
    scale1 = (int)((u8Data_eeprom[0xd2]&0xf0) >> 4);  //KT2_SCALE in EEPROM address 0xD2[7:4]                          
                        
    // Vth 25000                                                          
    //-----------                                                         
    Vth = 256*u8Data_eeprom[0xdb]+u8Data_eeprom[0xda];                                  
    if ( Vth > 32767) {Vth = Vth-65536;}                                                                                                             
    VTHF = (double)(Vth);                                                                                   
                                                                          
    // Kt1 ~ 0.003                                                        
    //-------------                                                       
    Kt1 = 256*u8Data_eeprom[0xdd]+u8Data_eeprom[0xdc];                                  
    if ( Kt1 > 32767) {Kt1 = Kt1-65536;}                                                                                                                                           
    KT1F = (double)(Kt1)/ VTHF/pow((double)2.0,(double)scale1);                                               
                                                                          
    // Kt2 ~ xe-7                                                         
    //------------                                                        
    Kt2 = 256*u8Data_eeprom[0xdf]+u8Data_eeprom[0xde];                                  
    if ( Kt2 > 32767) Kt2 = Kt2- 65536;                                   
    KT2F = (double)Kt2/VTHF/pow((double)2.0,(double)scale2+10.0);   

}  

/*******************************************************************************
* calc_TA_621
*******************************************************************************/
void calc_TA_621(void)
{
    int ta = u8PTATValue[1]*256 +u8PTATValue[0]; 
	  double tmp = KT1F*KT1F-4*KT2F*(1-ta/VTHF);
    tmp = pow(tmp,0.5);
    tmp = ( -KT1F+tmp)/(2*KT2F);
    tmp = tmp+25.0;
    TA = (float) tmp;
}

/*******************************************************************************
* fixed_coeff_621
*******************************************************************************/

void fixed_coeff_621(void)
{
	double bi,alpha;    
	u8 i,j;
	
    KsTa_0 = (double)u8Data_eeprom[230] + 256 * (double)u8Data_eeprom[231];
    if(KsTa_0 > 32767) { KsTa_0 = KsTa_0 - 65536;}
    KsTa_0 = KsTa_0 / pow(2.0,20.0);
    
    Ke = (double)u8Data_eeprom[228] + (double)256 * (double)u8Data_eeprom[229];
    Ke = Ke / 32768.0;

    Vth0 = (double)u8Data_eeprom[218] + 256 * (double)u8Data_eeprom[219];
    if (Vth0 > 32767) { Vth0 = Vth0 - 65536;}

    kt1Scale = (double)floor(u8Data_eeprom[210] / 16.0);
    kt1 = (double)u8Data_eeprom[220] + 256 * (double)u8Data_eeprom[221];
	  if(kt1 > 32767) { kt1 = kt1 - 65536;}

    kt1 = kt1 / Vth0 / pow(2, kt1Scale);    /// 2 ^ (3 - resolution)
    kt2Scale = 10 + (double)u8Data_eeprom[210] - 16 * kt1Scale;
    kt2 = (double)u8Data_eeprom[222] + 256 * (double)u8Data_eeprom[223];
    if(kt2 > 32767) { kt2 = kt2 - 65536;}

    kt2 = kt2 / Vth0 / pow(2,kt2Scale) ;   /// 2 ^ (3 - resolution)
    scale_com_alpha = (double)u8Data_eeprom[226];
    scale_alpha = (double)u8Data_eeprom[227];
    common_alpha = (double)u8Data_eeprom[224] + 256 * (double)u8Data_eeprom[225];
    deltaOSScale = floor((double)u8Data_eeprom[217] / 16);
    iros_scale = (double)u8Data_eeprom[217] - 16 * deltaOSScale;
    cyclops_alpha = (256 * (double)u8Data_eeprom[215] + (double)u8Data_eeprom[214]) / pow(2,scale_com_alpha); // / 2 ^ (3 - resolution)
    cyclops_ai = (double)u8Data_eeprom[211] + (double)256 * (double)u8Data_eeprom[212];
    if(cyclops_ai > 32767.0){ cyclops_ai = cyclops_ai - 65536.0;}
    cyclops_bi = (double)u8Data_eeprom[213];
    if(cyclops_bi > 127.0 ){cyclops_bi = cyclops_bi - 256.0;}
    cyclops_bi = cyclops_bi / pow(2.0 , iros_scale); // / 2 ^ (3 - resolution)    
    TGC_0 = (double)u8Data_eeprom[216];
    if(TGC_0 > 127.0) { TGC_0 = TGC_0 - 256.0;}
    TGC_0 = TGC_0 / 32.0;
    commonAi = (double)u8Data_eeprom[208] + (double)256 * (double)u8Data_eeprom[209];
    if (commonAi > 32767.0) {commonAi = commonAi - 65536.0;}

   
	for (j=0; j<16; j++)
	{
		for (i=0; i<4; i++) 
		{
			ai = commonAi + (double)u8Data_eeprom[i+(j*4)] * pow(2.0,deltaOSScale); //  / 2 ^ (3 - resolution)
			Ai[i][j] = ai;

			bi = (double)u8Data_eeprom[0x40+i+(j*4)];
			if(bi > 127.0) { bi = bi - 256.0;}
			bi = bi / pow(2,iros_scale); // / 2 ^ (3 - resolution)
			Bi[i][j] = bi;
			alpha = (double)u8Data_eeprom[0x80+(i+j*4)];
			alpha = alpha / pow(2.0,scale_alpha) + common_alpha / pow(2,scale_com_alpha); // / 2 ^ (3 - resolution)
			Alpha[i][j] = alpha;      
		}
	}
} 



/*******************************************************************************
* calc_IR_621
*******************************************************************************/
int calc_IR_621(short x,short y)
{     
  double S2,S1;
	float To;
	
	if((x<0) || (x>3)) { return 0; }
  if((y<0) || (y > 15)) { return 0;}

	ai = Ai[x][y];
	bi = Bi[x][y];
  alpha =Alpha[x][y];
  cyclopsData = (double)256*(double)u8CompstPiexl[1]+(double)u8CompstPiexl[0]; 
  if(cyclopsData > 32767.0) {cyclopsData = cyclopsData - 65536.0; }
  Tobj = (double)u8IRValue[(2*x+y*8)]+((double)u8IRValue[((2*x+y*8))+1])*256.0; 
	if ( Tobj > 32767) { Tobj = Tobj-65536.0; }
  S1 = (Tobj - (ai + bi * ((double)TA - (double)25.0))) - TGC_0 * (cyclopsData - (cyclops_ai + cyclops_bi * ((double)TA - (double)25.0)));     
  S1 = S1/Ke;
	S2 = S1/((1 + KsTa_0 * ((double)TA - (double)25.0)) * (alpha - TGC_0 * cyclops_alpha)); 
  S2 = S2 +pow(((double)TA+(double)273.15),4);
     
  S1=pow(S2,(double)0.25);
  S1= S1-(double)273.15;
  To = (float)S1;   
  IMAGE[x][y]=(short)(To*10);
//	if(IMAGE[x][y]>290) Binaryzation[x][y]=1;
//	else Binaryzation[x][y]=0;
//  IMAGE[x][y]=(short)(To*10+400);   // store temp x by 10 for 0.1 deg accur. in short   
  return 1;                 
} 

/*******************************************************************************
* MLX_WorkInit
*******************************************************************************/
void MLX90621_WorkInit(void)
{
	structInit();
	GPIO_Configuration_I2C();
	MLX90621_PowerOn();	
	
//	while(u16PORDelay2ms <= 5)
//        {
//            u16PORDelay2ms++;
//            //u16PORDelay5ms++;
//            if(u16PORDelay5ms >= 5) { stI2CWriteRead.bOperationFlag = 1;}
//            
//            if(stI2CWriteRead.bOperationFlag == 0) { u16PORDelay5ms++;}
//            delay_ms(5);
//            
//        };  
	u16PORDelay5ms = 0;	
	
	I2C_writeOscTrim();  		
	I2C_writeDevConfig();	
	I2C_readEeprom();
	I2C_readPixelCompensation();	
	PrepareCoeff_TA_621();
	fixed_coeff_621();
//	BackgroundInit();
}

/*******************************************************************************
* Get_sensor_data
*******************************************************************************/
void Get_sensor_data(void)  
{
	u8 i,j;

	I2C_readDevConfig();
	if ( ((u8ConfigValue[1] & 0x04)) ) { MLX90621_WorkInit();}	
	
	I2C_readPTAT();
	calc_TA_621();
	I2C_readIRData();
	I2C_readPixelCompensation();	 
		
	for (j=0; j<16; j++) { for (i=0; i<4; i++) { calc_IR_621(i,j);} } 	
}

/******************************************************
���룺��ǰ����֡����
�����������ж�ĳ���ص��Ƿ�����
********************************************************/
void Humen_detect(short image[4][16])
{
	int i=0,j=0,flag=0,nonHumenNum=0;
	min=image[0][0];
	max=image[0][0];
	//mean=image[2][8];
	
	for(i=0;i<4;i++)
	{
		for(j=0;j<16;j++)
		{
			NewMinusOld[i][j]=image[i][j]-BackgroundTemp[i][j];//�õ���ǰ����ֵ�뱳���¶ȵĲ�ֵ
                        
                        if(j<=1)
                        {
                             if(NewMinusOld[i][j]>40)//�����ֵ�������ȣ��ж�Ϊ�������ˣ���ʱ�������Ʊ�õ���1
                            {
                                            Binaryzation[i][j]=1;                  
                            }else
                            {
                                            Binaryzation[i][j]=0;   //������0
                                            mean+=image[i][j];       //��û���˵����ص��¶���ͣ��Ա���ƽ��ֵ
                                            nonHumenNum++;           //��û���˵����ص���м���
                            }  
                        }else if(j>=14 || j==2)
                        {
                          if(NewMinusOld[i][j]>30)//�����ֵ�������ȣ��ж�Ϊ�������ˣ���ʱ�������Ʊ�õ���1
                            {
                                            Binaryzation[i][j]=1;                  
                            }else
                            {
                                            Binaryzation[i][j]=0;   //������0
                                            mean+=image[i][j];       //��û���˵����ص��¶���ͣ��Ա���ƽ��ֵ
                                            nonHumenNum++;           //��û���˵����ص���м���
                            }  
                        
                        }else
                        {
                            if(NewMinusOld[i][j]>THREDHOLD)//�����ֵ�������ȣ��ж�Ϊ�������ˣ���ʱ�������Ʊ�õ���1
                            {
                                            Binaryzation[i][j]=1;                  
                            }else
                            {
                                            Binaryzation[i][j]=0;   //������0
                                            mean+=image[i][j];       //��û���˵����ص��¶���ͣ��Ա���ƽ��ֵ
                                            nonHumenNum++;           //��û���˵����ص���м���
                            }
                        }
			
		 if(image[i][j]>max) max=image[i][j];     //�ҳ���֡���ݵ���Сֵ�����ֵ
		 if(image[i][j]<min) min=image[i][j];
		 avr+=image[i][j];		
		}
	}
	if(nonHumenNum!=0)
	{
			mean/=nonHumenNum;           //��û���˵����ص��ƽ��ֵ
	}else
	{
			mean=image[2][8];
	}
	
  avr/=64;
	
	
//	for(i=0;i<4;i++)
//	{
//		for(j=0;j<16;j++)
//		{
//			if(image[i][j]>max) max=image[i][j];     //�ҳ���֡���ݵ���Сֵ�����ֵ
//			if(image[i][j]<min) min=image[i][j];
//			sum+=image[i][j];
//			Binaryzation[i][j]=0;                     //����ֵ�����ʼ��Ϊ0
//		}
//	}
//	avr=sum/64+10;
//	if(max>min+50)           //������ֵ��Сֵ֮���������5�ȱ�ʾ���ˣ�5��ͨ��ʵ��۲�����
//	{
		 for(i=0;i<4;i++)      //�жϸ����ص���Χ�Ƿ����ˣ�������ˣ�֤���õ�ȷ�����˶���������
		{
			for(j=0;j<16;j++)
			{
					if(i==0)       //��0���ж��Ƿ�����
					{
							if(j==0)
							{
//								if(image[i][j]>avr && image[i][j+1]>avr && image[i+1][j]>avr)  Binaryzation[i][j]=1;
//								continue;
								  if(Binaryzation[i][j] == 1)//�жϸ����ص��Ƿ����ˣ�����������жϸ���4�����Ƿ���2�����ϵ����ص��������
									{
										flag=0;
										Binaryzation[i+1][j]==1 ? flag++:0;
										Binaryzation[i][j+1]==1 ? flag++:0;
                                                                                Binaryzation[i+1][j+1]==1 ? flag++:0;
										if(flag>=BEIGHBOR) 
										{
											Binaryzation[i][j]=1;    //����и�����������ȷ��������ص�϶����ˣ��������Ʊ���1
										}else
										{
											Binaryzation[i][j]=0;     //����������0���ж�Ϊ���ˣ�֮ǰ��������������������
										}
									}
							}
							if(Binaryzation[i][j] == 1)
							{
								flag=0;
								Binaryzation[i][j-1]==1 ? flag++:0;
                                                                Binaryzation[i+1][j-1]==1 ? flag++:0;
                                                                Binaryzation[i+1][j+1]==1 ? flag++:0;
								Binaryzation[i+1][j]==1 ? flag++:0;
								Binaryzation[i][j+1]==1 ? flag++:0;
								if(flag>=BEIGHBOR) 
								{
									Binaryzation[i][j]=1;
								}else
								{
									Binaryzation[i][j]=0;
								}
							}
							if(j==15)     //������ж�
							{
								if(Binaryzation[i][j] == 1)
								{
									flag=0;
									Binaryzation[i][j-1]==1 ? flag++:0;
									Binaryzation[i+1][j]==1 ? flag++:0;
                                                                        Binaryzation[i+1][j-1]==1 ? flag++:0;
									if(flag>=BEIGHBOR) 
									{
										Binaryzation[i][j]=1;
									}else
									{
										Binaryzation[i][j]=0;
									}
								}
							}
//							if(image[i][j]>avr && image[i][j+1]>avr && image[i+1][j]>avr && image[i-1][j]>avr)  
//							{
//								Binaryzation[i][j]=1;continue;
//							}
//							if(j==3)
//							{
//								if(image[i][j]>avr && image[i][j-1]>avr && image[i+1][j]>avr)  Binaryzation[i][j]=1;
//								continue;
//							}
					}

					if(i==3)
					{
							if(j==0)
							{
								 if(Binaryzation[i][j] == 1)
									{
										flag=0;
										Binaryzation[i-1][j]==1 ? flag++:0;
                                                                                Binaryzation[i-1][j+1]==1 ? flag++:0;
										Binaryzation[i][j+1]==1 ? flag++:0;
										if(flag>=BEIGHBOR) 
										{
											Binaryzation[i][j]=1;
										}else
										{
											Binaryzation[i][j]=0;
										}
									}
								
							}
							if(Binaryzation[i][j] == 1)
							{
								flag=0;
								Binaryzation[i][j-1]==1 ? flag++:0;
                                                                Binaryzation[i-1][j-1]==1 ? flag++:0;
                                                                Binaryzation[i-1][j+1]==1 ? flag++:0;
								Binaryzation[i-1][j]==1 ? flag++:0;
								Binaryzation[i][j+1]==1 ? flag++:0;
								if(flag>=BEIGHBOR) 
								{
									Binaryzation[i][j]=1;
								}else
								{
									Binaryzation[i][j]=0;
								}
							}
							if(j==15)
							{
								if(Binaryzation[i][j] == 1)
								{
									flag=0;
									Binaryzation[i][j-1]==1 ? flag++:0;
									Binaryzation[i-1][j]==1 ? flag++:0;
                                                                        Binaryzation[i-1][j-1]==1 ? flag++:0;
									if(flag>=BEIGHBOR) 
									{
										Binaryzation[i][j]=1;
									}else
									{
										Binaryzation[i][j]=0;
									}
								}
							}
					}
					if(Binaryzation[i][j] == 1)
					{
						flag=0;
						Binaryzation[i][j-1]==1 ? flag++:0;
                                                Binaryzation[i][j+1]==1 ? flag++:0;
                                                Binaryzation[i-1][j-1]==1 ? flag++:0;
						Binaryzation[i-1][j]==1 ? flag++:0;
						Binaryzation[i-1][j+1]==1 ? flag++:0;
						Binaryzation[i+1][j]==1 ? flag++:0;
                                                Binaryzation[i+1][j-1]==1 ? flag++:0;
                                                Binaryzation[i+1][j+1]==1 ? flag++:0;
						if(flag>=BEIGHBOR) 
						{
							Binaryzation[i][j]=1;
						}else
						{
							Binaryzation[i][j]=0;
						}
					}
					
			}
		}
//	}
//	BackgroundUpdate(image);//���±����¶�
}
/**************************************************
����������ȡǰ20֡������ƽ��ֵ��Ϊ�����¶�
*************************************************/
void BackgroundInit(void)
{
	int i=0,j=0;
	while(avrNum<20)
	{
		
		if(stI2CWriteRead.bOperationFlag)  
		{
			avrNum++;
			Get_sensor_data();
			for(i=0;i<4;i++)
			{
				for(j=0;j<16;j++)
				{
					BackgroundTemp[i][j]+=IMAGE[i][j];
				}
			
			}
			
		}
	}
	for(i=0;i<4;i++)
	{
		for(j=0;j<16;j++)
		{
			BackgroundTemp[i][j]/=20;
		}
	
	}	
}
/***********************************************
���������������¶ȵĸ���
���룺��ǰ��������֡
***********************************************/
void BackgroundUpdate(short image[4][16])
{
	int i=0,j=0;
	for(i=0;i<4;i++)
	{
		for(j=0;j<16;j++)
		{
			if(NewMinusOld[i][j]>20)      
			{
					BackgroundTemp[i][j]=BackgroundTemp[i][j]*0.99+mean*0.01;//�������¹�ʽ
			}else
			{
					BackgroundTemp[i][j]=BackgroundTemp[i][j]*0.99+image[i][j]*0.01;
			}		
		}
	
	}	
		
}
void MLX90621IR_ReadSead(void)
{	
  u8 i,j;
	if(stI2CWriteRead.bOperationFlag)  
	{
		Get_sensor_data();
//		USART_Send_IRdata2();
		Humen_detect(IMAGE);
//		if(Res == 0x01)
//		{
                 //��64λ������ѹ����8λ
                for(i = 0; i < 4; i++) 
                {
                    for(j = 0; j < 16; j++)
                    {
                        if(j < 8)
                        {
                            Binaryzation_8byte[2 * i] = Binaryzation_8byte[2 * i] << 1;
                            Binaryzation_8byte[2 * i] += Binaryzation[i][j];
                        }
                        else if(j < 16)
                        {
                            Binaryzation_8byte[2 * i + 1] = Binaryzation_8byte[2 * i + 1] << 1;
                            Binaryzation_8byte[2 * i + 1] += Binaryzation[i][j];
                        }
                    }
                }
                
//		USART2_Send_IRdata();
                //�ж��Ƿ���ϴ�����һ��
//                for(i = 0; i < 8; i++)
//                {
//                    if(Binaryzation_8byte[i] != last_Binaryzation_8byte[i])
//                    {
//                        Time_flag = DATA_CHANDE;
//                        break;
//                    }
//                }
//                
//                //������ݺ��ϴ�һ��
//                if(Time_flag != DATA_CHANDE)
//                {
//                    //�ж������Ƿ��ǲ���ȫ��
//                    for(i = 0; i < 8; i++)
//                    {
//                        if(Binaryzation_8byte[i] != 0)
//                        {
//                            Time_flag = NOT_ALL_ZERO;
//                            break;
//                        }
//                        else
//                        {
//                            Time_flag = ALL_ZERO;
//                        }
//                    }
//                }
//                
//                switch(Time_flag)
//                {
//                    case ALL_ZERO: Time_cnt++; break;
//                    case NOT_ALL_ZERO: Time_cnt += 15;break;
//                    case DATA_CHANDE: Time_cnt = 30;break;
//                    default :Time_cnt++;break;
//                }
//                            
//                if(Time_cnt >= 30)
//                {
////                    USART2_Send_IRdata();
//                    Time_cnt = 0;
//                    Time_flag = 0;
//                    
//                    //�����ݸ�ֵ��last������
//                    for(i = 0; i < 8; i++)
//                    {
//                          last_Binaryzation_8byte[i] = Binaryzation_8byte[i];
//                    }
//                }            
                
//		}
//		USART2_Send_Byte(Res);
                
                
//		stI2CWriteRead.bOperationFlag = 0;

	}
	
}


