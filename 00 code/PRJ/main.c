/*!
 *     COPYRIGHT NOTICE
 *     Copyright (c) 2021,一起学自控
 *     All rights reserved.
 *     技术讨论：QQ群877041899
 *
 *     除注明出处外，以下所有内容版权均属一起学自动所有，未经允许，不得用于商业用途，
 *		 版权所有，盗版必究，修改内容时必须保留一起学自控的版权声明。
 *
 * @file       main.c
 * @brief      主函数
 * @author     SuJin
 * @version    v1.0
 * @date       2021-08-16
 */
#include "include.h"

int main(void)
{
	//1设置中断优先级分组为2，两位抢占优先级和两位相应优先级
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	//2滴答计时器初始化
	DELAY_Init();
	//3电机控制各种参数初始化
	PARAM_Init();
	//4普通IO管脚初始化,LED DEBUG_PIN
	GPIO_COM_Init();
	//5初始化PWM频率为18KHz	
	TIM1_PWM_Init(18);
	//6ADC初始化	
	ADC_DMA_Init();
	//7PPM，单位计数时间为1微秒，计数65535微秒（65.535.毫秒）产生溢出中断
	TIM2_CAP_Init(0xFFFF,71);	
	//8PID，初始化定时器3，周期为99->10ms,	199->20ms,499->50ms,999->100ms	
	TIM3_Init(99,7199);	
	//9串口1初始化	
	USART1_Init(115200);
	//10检测MOS是否短路	
	MOSSHORT_Check();	
	//11检测MOS能否正常导通	
	MOSOPEN_Check();		
	//12ADC采样偏置和电池种类读取	
	OFFSET2BAT_Read();	
	//13电机开环启动	
	MOTOR_Start();	
  while(1)
	{
		
//		OPEN_Check();		
		u8 i=0;
		u16 t[6]={0};
	u16 I_BUS[5],IBUFF=0;
	
	for(i=0;i<5;i++)
	{
		I_BUS[i]=ADC_Data[BIT_I]-OFFSET_I;
		delay_ms(10);
	}
	IBUFF = (I_BUS[0]+I_BUS[1]+I_BUS[2]+I_BUS[3]+I_BUS[4])/5.0;
		
		t[0] = MC.SPEED>>8;
		t[1] = MC.SPEED;
		t[2] = MC.DUTY>>8;
		t[3] = MC.DUTY;
		t[4] = IBUFF>>8;
		t[5] = IBUFF;
		
		for (i=0; i<6; i++)
		{
			USART_SendData(USART1 ,t[i]); //发送单个数值
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); //检查发送中断标志位
		}
		delay_ms(50);
	}
}
