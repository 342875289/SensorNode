#include "esp8266.h"

int rn_count = 0;//�Խ�����Ϣ�еĻ��з��������յ�3��\r\n��Ϊ��������
char wifi_feedback[200];//���ڴ�Ž������ݵ�����
int data_i=0;//�����������������
int time_wait=3;//�������ݵȴ���ʱ��(3��)
int receive_state=0;//Ϊ0��ʾ��û���յ�����  Ϊ1��ʾ���ڽ������� Ϊ2��ʾ�������ݵĹ�����������
u8 fac_us=72000000/8000000;	
u16 fac_ms=72000000/8000000*1000;

//��������WiFi�������ɹ�����1��ʧ�ܷ���0
int Connect_master()
{
	//�ȴ�ģ�������������
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	
	
	//�ر��������
	printf("ATE0\r\n");
	if(Re(10)==1){;}
	else{return 0;}
	delay_ms(1000);
	
	
	//�ӻ�����AP�ȵ�
	printf("AT+CWJAP=\"lift-project\",\"labcat127\"\r\n");
	if(Re(10)==1){;}
	else{return 0;}
	delay_ms(1000);
	
	//�ӻ�����STAģʽ�µ�IP
	printf("AT+CIPSTA=\"192.168.4.3\"\r\n");
	if(Re(10)==1){;}
	else{return 0;}
	delay_ms(1000);
	
	//�ӻ�����Ϊ������
	printf("AT+CIPMUX=0\r\n");
	if(Re(10)==1){;}
	else{return 0;}
	delay_ms(1000);
	
	//�ӻ���������
	printf("AT+CIPSTART=\"TCP\",\"192.168.4.1\",8088\r\n");
	if(Re(10)==1)
	{
		//delay_ms(1000);delay_ms(1000);delay_ms(1000);
		/* Clear the Overrun Error interrupt pending bit */
		//USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�;
	}
	else{return 0;}
	delay_ms(1000);
	
	/*
	//�ӻ�����������ָ���ֽ�������Ϣ
	printf("AT+CIPSEND=XX\r\n");
	if(Re(10)==1){;}
	else{return 0;}
	delay_ms(1000);
	*/
	
	/*
	//�ӻ��Ͽ�������������
	printf("AT+CIPCLOSE\r\n");
	if(Re(10)==1){;}
	else{return 0;}
	delay_ms(1000);
	delay_ms(1000);
	*/
	
	
	return 1;
	
	/*
	
	printf("\r\n");
	printf("\r\n");
	printf("\r\n");*/
}
	
void Net_Init()
{
	;
	
}

int Re(int seconds)
{
	u32 temp;
	u8 data;
	u16 nms=1000;
	int times=seconds;
	data_i=0;
	
	while( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == RESET );//�ȴ���һ���ֽ�
	
	while(times--)
	{
		SysTick->LOAD=(u32)nms*fac_ms;				//ʱ�����(SysTick->LOADΪ24bit)
		SysTick->VAL =0x00;							//��ռ�����
		SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����  
		do
		{
			if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
			{
				data = USART_ReceiveData(USART3);
				wifi_feedback[data_i]=data ;
				//OK
				if( wifi_feedback[data_i] == 'K' && data_i>=1 && 
					wifi_feedback[data_i-1] == 'O' )
				{
					SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
					SysTick->VAL =0X00;       					//��ռ�����	
					return 1;//���سɹ���Ϣ
				}
				//ERROR
				if( wifi_feedback[data_i] == 'R' && data_i>=4 && 
					wifi_feedback[data_i-1] == 'O' &&
					wifi_feedback[data_i-2] == 'R' &&
					wifi_feedback[data_i-3] == 'R' &&
					wifi_feedback[data_i-4] == 'E' 
				)
				{
					SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
					SysTick->VAL =0X00;       					//��ռ�����	
					return 0;//����ʧ����Ϣ
				}
				
				//���ճɹ�-����װ�ص���ʱ����
				times=seconds;
				data_i++;
			}
			
			
			temp=SysTick->CTRL;
		}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
		SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
		SysTick->VAL =0X00;       					//��ռ�����	
	}
	return 0;
}



void USART3_IRQHandler(void)                	//����3�жϷ������
{
	u32 temp;
	u8 data;
	u16 nms=1000;
	short az;
	int times=10;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		//ʧ�ܽ����ж�--׼�����������ַ�
		USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
		while(times--)
		{
			SysTick->LOAD=(u32)nms*fac_ms;				//ʱ�����(SysTick->LOADΪ24bit)
			SysTick->VAL =0x00;							//��ռ�����
			SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����  
			do
			{
				if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
				{
					data = USART_ReceiveData(USART3);
					//R--�յ���������������
					if( data == 'R')
					{
						SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
						SysTick->VAL =0X00;       					//��ռ�����	
						//���سɹ���Ϣ
						//����������Ϣ����
						printf("AT+CIPSEND=6\r\n");
						if(Re(10)==1)
						{
							while(1)//�ȴ��������͵�ָ��
							{
								if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
								{if( USART_ReceiveData(USART3) == '>'){break;}}
							}
							//����������Z����ٶ�����
							if( MPU_Get_Accelerometer_Z(&az)==0 )
							{
								printf("OK");
								//���ٶȸ�8λ
								USART3->DR = (u8)(az>>8);
								while((USART3->SR&0X40)==0);//ѭ������,ֱ���������
								//���ٶȵ�8λ
								USART3->DR = (u8)(az&0xff);
								while((USART3->SR&0X40)==0);//ѭ������,ֱ���������
								printf("\r\n");
								if(Re(10)==1){;}//LCD_Print(0,6,"Send-Request--OK");}
								else{;}//LCD_Print(0,6,"ERROR");}
							
							}
							else
							{;}//printf("ERROR");}
						}
						USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
						return;
					}
					//���ճɹ�-����װ�ص���ʱ����
					times=10;
				}
				
				temp=SysTick->CTRL;
			}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
			SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
			SysTick->VAL =0X00;       					//��ռ�����	
		}
		
     } 
	//���Ӵӻ�ʧ��--����ʹ�ܽ����ж�--׼���´�����
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
} 