#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//��ʱ�� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/4
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//********************************************************************************
void TIM1_SPWM_Init(unsigned int Prescaler);
void 	TIM8_PWM_Init(void );
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM3_SPWM_Init(u16 squareFrequence ,u16 psc);
void TIM_MasterSlaveModeInit(void);
void LED_GPIO_Config(void);
void TIM2_Configuration(void);
void TIM2_NVIC_Configuration(void);
#endif
