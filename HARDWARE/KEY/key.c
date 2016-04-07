#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "oled.h"
#include "stdio.h"	
#include "usart.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////  
								    
//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PORTA,PORTEʱ��

	/*GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//KEY0-KEY2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4*/
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3;//ֻ��ʼ��KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

	//��ʼ�� WK_UP-->GPIOA.0	  ��������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0

}

extern float sinFre;//���Ҳ�Ƶ��50Hz
extern int N;//���Ҳ������ڲ�����
extern int U;//���Ҳ���ѹ����
extern float a;//���Ʊ�

extern int outputstate;//����Ƿ����������ģʽ
extern int is_impulse;//����Ƿ��������ģʽ
extern int impulse_i;//������� 1����λΪһ�����Ҳ����� ��50��Ϊ����һ�������
extern int impulse_num;//������� 1����λΪһ�����Ҳ����� ��50��Ϊ����һ�������
extern int impulse_step;//���������������

//������̳�ʼ��
void MatrixKey_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF,ENABLE);
	//FΪ�����  EΪ������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6 ;			//F
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 |GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6;			//E
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //��������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
}
/*
	//������̳����
	1-->D  2-->#  3-->0  4-->*
	5-->C  6-->9  7-->8  8-->7
	9-->B  10->6  11->5  12->4
	13->A  14->3  15->2  16->1
	*/
int MatrixKey_Scan(void)
{
	
	
	if( 
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0 ||
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0 ||
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0 ||
			GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0
		)
	{
		delay_ms(10);//ȥ���� 
		if( 
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0 ||
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0 ||
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0 ||
				GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0
		)
			{
				
				GPIO_SetBits(GPIOF, GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
				GPIO_ResetBits(GPIOF, GPIO_Pin_0);//��һ��
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 1;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 2;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 3;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 4;}
				GPIO_SetBits(GPIOF, GPIO_Pin_0 );
				
				GPIO_ResetBits(GPIOF, GPIO_Pin_2);//�ڶ���
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 5;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 6;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 7;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 8;}
				GPIO_SetBits(GPIOF, GPIO_Pin_2 );
				
				GPIO_ResetBits(GPIOF, GPIO_Pin_4);//������
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 9;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 10;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 11;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 12;}
				GPIO_SetBits(GPIOF, GPIO_Pin_4 );
			
				GPIO_ResetBits(GPIOF, GPIO_Pin_6);//������
				if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 13;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 14;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 15;}
				else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0){while(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)==0);GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);	return 16;}

				GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
			}
			
	}   
	GPIO_ResetBits(GPIOF, GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6);
	return 0;
}

/*
������̰���֮��ִ�еĺ���
*/
void keyfunc(int key)
{
	switch(key)
	{
		case  1:{/*step-*/impulse_step--;if(impulse_step<=1){impulse_step=1;}}break;
		case  2:{/*step+*/impulse_step++;if(impulse_step>=50){impulse_step=50;}}break;
		case  3:{/*num_impulse-*/impulse_num-=impulse_step;if(impulse_num<=1){impulse_num=1;}}break;
		case  4:{/*num_impulse+*/impulse_num+=impulse_step;if(impulse_num>=500){impulse_num=500;}}break;
		case  5:{/**/}break;
		case  6:{/**/}break;
		case  7:{/**/}break;
		case  8:{/*��������ģʽ*/if(outputstate==0){is_impulse=1;LCD_Print(35,0,"����");TIM_CtrlPWMOutputs(TIM1, ENABLE);TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);}}break;
		case  9:{/*����WIFI*/printf("AT+CWJAP=\"Mon-PC\",\"317000000\"\r\n");}break;
		case 10:{/*�˳�WIFI*/printf("AT+CWQAP\r\n");}break;
		case 11:{/*�ر����*/TIM_CtrlPWMOutputs(TIM1, DISABLE);TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE );outputstate=0;LCD_Print(35,0,"ֹͣ");}break;
		case 12:{/*�������*/TIM_CtrlPWMOutputs(TIM1, ENABLE);TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE );outputstate=1;LCD_Print(35,0,"����");}break;
		case 13:{/*f-*/sinFre-=0.5;if(sinFre<=10){sinFre=0.5;}}break;
		case 14:{/*f+*/sinFre+=0.5;if(sinFre<=60){sinFre=60;}}break;
		case 15:{/*U-*/U-=5;if(U<=10){U=10;}}break;
		case 16:{/*U+*/U+=5;if(U<=250){U=250;}}break;		
	}
}








//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��KEY3���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>KEY3!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY1==0||WK_UP==1))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY1==0)return KEY1_PRES;
		else if(WK_UP==1)return WKUP_PRES;
	}else if(KEY1==1&&WK_UP==0)key_up=1; 	    
 	return 0;// �ް�������
}
