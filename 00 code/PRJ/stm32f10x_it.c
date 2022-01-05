/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/******************************************************************************/
/*以下为用户编写的所有中断服务函数                                              */
/******************************************************************************/

/***************PPM******************/
static u8 Rise_flag = 0;
static u16 Hightime_us = 0;
u16 PPM_Duty = 0,DELAY_CTL=0,START_FLAG=0;//PPM油门范围0-2000
/***************PPM******************/

//motorcontrol
void TIM1_UP_IRQHandler(void)
{
	static u8 ED360_CNT=0;
	TIM_TimeBaseInitTypeDef 	TIM_TimeBaseStructure;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//		if(MC.DUTY>=1000)//duty>=50%,设置上溢中断
//		{
//			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned2;//PWM中央对其模式，设置上溢中断
//			TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);			
//		}
//		else if(MC.DUTY<1000)//duty<=50%,设置下溢中断
//		{
//			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;//PWM中央对其模式，设置下溢中断
//			TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);			
//		}
		//电机控制代码
		if(MC.STATE==1)//电机已经启动成功，开始检测相应反电动势
		{
			DEBUG_PIN_HIGH;
			if(MC.CCW)//反转
				{
					switch(MC.PHA_CNT)
					{
						#if OPP 	//开启缺相保护
						//AB相导通，检测C相BEMF
						case 0: ZCD_C_DOWN(); MC.AI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 1: ZCD_B_UP(); 	MC.AI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 2: ZCD_A_DOWN(); MC.BI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 3: ZCD_C_UP(); 	MC.BI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 4: ZCD_B_DOWN(); MC.CI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 5: ZCD_A_UP(); 	MC.CI=(ADC_Data[BIT_I]-OFFSET_I); break;
						default: break;
						#else 
						//AB相导通，检测C相BEMF
						case 0: ZCD_C_DOWN(); break;
						case 1: ZCD_B_UP(); 	break;
						case 2: ZCD_A_DOWN(); break;
						case 3: ZCD_C_UP(); 	break;
						case 4: ZCD_B_DOWN(); break;
						case 5: ZCD_A_UP(); 	break;
						default: break;
						#endif
					}
				}
				else//正转
				{
					switch(MC.PHA_CNT)
					{
						#if OPP		//开启缺相保护
						case 5: ZCD_C_UP(); 	MC.AI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 4: ZCD_B_DOWN(); MC.AI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 3: ZCD_A_UP(); 	MC.BI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 2: ZCD_C_DOWN(); MC.BI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 1: ZCD_B_UP(); 	MC.CI=(ADC_Data[BIT_I]-OFFSET_I); break;
						case 0: ZCD_A_DOWN(); MC.CI=(ADC_Data[BIT_I]-OFFSET_I); break;
						default: break;
						#else
						case 5: ZCD_C_UP(); 	break;
						case 4: ZCD_B_DOWN(); break;
						case 3: ZCD_A_UP(); 	break;
						case 2: ZCD_C_DOWN(); break;
						case 1: ZCD_B_UP(); 	break;
						case 0: ZCD_A_DOWN(); break;
						default: break;
						#endif
					}
				}
		}
		if(MC.STATE==2)//开始延时然后换相
		{
			DEBUG_PIN_LOW;	
			MC.DELAY30--;
			if (MC.DELAY30 == 0)
			{
				MC.PHA_CNT++;
				if(MC.PHA_CNT == 6)
				{
					ED360_CNT++;//转过360度电角度计数
					if(ED360_CNT==POLES)//机械角度转过360度
					{
						ED360_CNT=0;
						MC.CYCLES++;
					}
					MC.PHA_CNT = 0;
				}
				phase_change();
				MC.STATE=1;
			}
		}
	}
}

//PPM
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_CC2) != RESET)//捕获到了上升沿
	{
		if(Rise_flag == 1)//上次捕获到上升沿，这次为下降沿
		{
			Rise_flag=0;//上升沿标志清零
			Hightime_us=TIM_GetCapture2(TIM2);//记录3通道计数值
			//计算PPM控制量的占空比
			if(Hightime_us<960)
			{
				PPM_Duty=80;
			}
			else if (Hightime_us>2100)
			{
				PPM_Duty=2000;
			}
			else
			{
				PPM_Duty=Hightime_us-900;
			}
			if (PPM_Duty > 500)
			{
				PPM_Duty=500;
			}
			if (DELAY_CTL == 1)
			{
				MC.DUTY = PPM_Duty;
			}
			else
			{
				START_FLAG = 1;
			}
			if (MC.DUTY <= 120)
			{
				MC.ADJ30=0.7;
			}
			else if (MC.DUTY > 120 && MC.DUTY<=130)
			{
				MC.ADJ30=0.6;
			}
			else if (MC.DUTY>130 && MC.DUTY <=140)
			{
				MC.ADJ30=0.5;
			}
			else if (MC.DUTY>140 && MC.DUTY <=150)
			{
				MC.ADJ30=0.4;
			}
			else if (MC.DUTY>150 && MC.DUTY <=160)
			{
				MC.ADJ30=0.3;
			}
			else if (MC.DUTY>160 && MC.DUTY <=170)
			{
				MC.ADJ30=0.3;
			}
			else if (MC.DUTY>170 && MC.DUTY <=200)
			{
				MC.ADJ30=0.2;
			}
			else
			{
				MC.ADJ30=0.2;
			}
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising);//设置为上升沿捕获
		}
		else//第一次捕获到上升沿
		{
			Rise_flag=1;//上升沿标志置位
			Hightime_us=0;//高电平持续时间归零
			TIM_SetCounter(TIM2,0);//计数器的值设置为0
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Falling);//设置为下降沿捕获
		}
	}
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update|TIM_IT_CC2);//清中断标志
}

//PID和转速测量
void TIM3_IRQHandler(void)
{
	static u8 cnt_2s=0,cnt1s=0;
	u16	BUFF=0;//电机转速缓存
	
	#if (LOOP==0 || LOOP==1)
	s16 iErr=0;	//当前电流误差
	s16 buff_duty=0; //输出缓存
	static s16 sum_iErr=0;	//累计电流误差
	static s16 last_iErr=0;	//上次电流误差
	#endif
	#if (LOOP==1)
	s16 sErr=0;//当前速度误差
	static s16 last1_sErr=0;//上次速度误差
	static s16 last2_sErr=0;//上上次速度误差
	#endif
	
	
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)//判断是否是TIM3中断
	{	
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);//清中断标志
		
		/*****************计算转速*****************/
		cnt_2s++;
		if(cnt_2s==200)//2秒时间已到
		{
			cnt_2s=0;
			BUFF=MC.SPEED;
			MC.SPEED = MC.CYCLES*30;
			FirstOrder_LPF(BUFF,MC.SPEED,0.8);//对测得的转速进行一阶低通滤波
			MC.CYCLES=0;
		}
		//延时PPM控制
		if (START_FLAG == 1)
		{
			START_FLAG = 0;
			cnt1s++;
		}
		if (cnt1s == 255)
		{
			cnt1s=0;
			DELAY_CTL = 1;
		}
		/*****************计算转速*****************/
		
//		
//		#if (LOOP == 0)//0：电流环
//		/*****************电流环*****************/
//			//1、计算当前误差
//			iErr = PPM_Duty - (ADC_Data[BIT_I]-OFFSET_I);
//			
//			//2、计算累计误差并限幅
//			sum_iErr += iErr;
//			if (sum_iErr > 10000)
//			{
//				sum_iErr = 10000;
//			}
//			else if (sum_iErr < -10000)
//			{
//				sum_iErr = -10000;
//			}
//			
//			//3、位置式PID
//			buff_duty =  iErr * 100 + sum_iErr * 0.5 + (iErr - last_iErr) * 0;
//			
//			//4、PID输出限幅
//			if (buff_duty>2000)
//			{
//				buff_duty = 2000;
//			}
//			else if (buff_duty < 0)
//			{
//				buff_duty = 0;
//			}
//			
//			//5、输出
//			MC.DUTY = buff_duty;
//			
//			//6、记录当前误差
//			last_iErr = iErr;
//		/*****************电流环*****************/
//		
//		#elif (LOOP == 1)//1：速度环串电流环
//		/*****************速度环+电流环*****************/
//			//1、计算当前速度误差
//			sErr = PPM_Duty - MC.SPEED;
//			
//			//2、增量式PID
//			buff_duty += 1*(sErr-last1_sErr)+1*sErr+1*(sErr-2*last1_sErr+last2_sErr);
//			
//			//3、记录当前误差
//			last2_sErr = last1_sErr;
//			last1_sErr = sErr;
//			
//			//4、计算当前误差
//			iErr = buff_duty - (ADC_Data[BIT_I]-OFFSET_I);
//			
//			//5、计算累计误差并限幅
//			sum_iErr += iErr;
//			if (sum_iErr > 10000)
//			{
//				sum_iErr = 10000;
//			}
//			else if (sum_iErr < -10000)
//			{
//				sum_iErr = -10000;
//			}
//			
//			//6、位置式PID
//			buff_duty =  iErr * 100 + sum_iErr * 0.5 + (iErr - last_iErr) * 0;
//			
//			//7、PID输出限幅
//			if (buff_duty>2000)
//			{
//				buff_duty = 2000;
//			}
//			else if (buff_duty < 0)
//			{
//				buff_duty = 0;
//			}
//			
//			//8、输出
//			MC.DUTY = buff_duty;
//			
//			//9、记录当前误差
//			last_iErr = iErr;
//		/*****************速度环+电流环*****************/
//		#endif
		
		/*****************过欠压保护*****************/
		#if OUP
		if(ADC_Data[BIT_U] > MC.OVER_VOL)//母线电压大于13V
		{
			ALLMOS_OFF;
			MC.FAULT = 13;
			LED_Blink(3);
			while(1);
		}
		else if (ADC_Data[BIT_U] < MC.UNDER_VOL)//母线电压小于11V
		{
			ALLMOS_OFF;
			MC.FAULT = 14;
			LED_Blink(4);
			while(1);
		}
		#endif
		#if OCP
		/*****************过流保护*****************/
		if (ADC_Data[BIT_I] > 2850)//超过45A则启动电流保护,MOS耐电流50A，2212最大电流19A，主要是保护MOS
		{
			ALLMOS_OFF;
			MC.FAULT = 15;
			LED_Blink(5);
			while(1);
		}
		#endif
		#if MSLP
		/*****************堵转保护*****************/
		if(MC.DUTY>321&&MC.SPEED<123&&(ADC_Data[BIT_I]-OFFSET_I)>321)//控制量大，速度慢，且电流大，则判断为堵转
		{
			ALLMOS_OFF;
			MC.FAULT = 16;
			LED_Blink(6);
			while(1);
		}
		#endif
		#if OPP
		/*****************缺相保护*****************/
		//某相电流小于其他任意一相电流的一半，则认为是缺相，只能检测缺一相或者两相
		if(MC.AI<(MC.BI/2) || MC.AI<(MC.CI/2) || MC.BI<(MC.AI/2) || MC.BI<(MC.CI/2) || MC.CI<(MC.BI/2) || MC.CI<(MC.AI/2))
		{
			ALLMOS_OFF;
			MC.FAULT = 17;
			LED_Blink(7);
			while(1);
		}
		#endif
	}
}

//串口接收中断
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) //接收到数据
	{	  
		USART_ClearFlag(USART1, USART_FLAG_RXNE);     //清除中断标志
	}
}

/******************************************************************************/
/*以上为用户编写的所有中断服务函数                                              */
/******************************************************************************/

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
