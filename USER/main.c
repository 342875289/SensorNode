#include "stm32f10x.h"
#include "usart.h"
#include "esp8266.h"
#include "time.h"
#include "delay.h"
#include "mpu6050-STM.h"
/************************************************

************************************************/
void Led_init(void);
int net_state=0;//��־�Ƿ����ӵ�WiFi����
short accel_x,accel_y,accel_z;//ˮƽ����ʱ��z�����1��g�ļ��ٶ�
short gyro_x,gyro_y,gyro_z; 
short ac_state,gr_state;
short az;
 int main(void)
{	
	u8 data;
	
	//LEDָʾ�Ƴ�ʼ��
	Led_init();
	//����NVIC�жϷ���2 : 2λ��ռ���ȼ�(�ж�Ƕ�״���)��2λ��Ӧ���ȼ�(�ж���Ӧ�Ⱥ�˳��)
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	
	//��ʱ������ʼ��	
	delay_init();
	//��ʼ�� ����USART3  ������921600 8λ����λ 1λֹͣλ 0λУ��λ
	UART_Init(921600);
	//MPU6050��ʼ��
	MPU_Init(); 
		
	//��������--ʧ�ܺ�ÿ��2�����³���
	while( !(Connect_master()) )
	{
		delay_ms(1000);delay_ms(1000);
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);//LED����
		delay_ms(1000);delay_ms(1000);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);//LED����
	}
	
	//LED����--��ʾ���������ɹ�
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	while(1)
	{
		//MPU_Get_Accelerometer_Z(&accel_z);
		/*
		ac_state=MPU_Get_Accelerometer(&accel_x,&accel_y,&accel_z);	//�õ����ٶȴ���������
		gr_state=MPU_Get_Gyroscope(&gyro_x,&gyro_y,&gyro_z);	//�õ�����������
		delay_ms(200);
		*/
		
		/*
		//������־���빤��״̬
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);//LED����
		delay_ms(500);
		GPIO_SetBits(GPIOC, GPIO_Pin_13);//LED����
		delay_ms(500);*/
		
		if( USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET)
		{
			GPIO_SetBits(GPIOC, GPIO_Pin_13);//LED����
			data = USART_ReceiveData(USART3);
			//R--�յ���������������
			if( data == 'R')
			{
				//����������Ϣ����
				printf("AT+CIPSEND=6\r\n");
				if(Re(10)==1)
				{
					while(1)//�ȴ������͵�ָ��
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
					else{;}//printf("ERROR");}
				}
			}
			GPIO_ResetBits(GPIOC, GPIO_Pin_13);//LED����
		}
	}
 }
void Led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 ;		//PC.13
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	//����
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}
