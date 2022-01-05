#include "timer.h"
//TIM1用于PWM
//TIM2用于PPM
//TIM3用于PID


//pwm_freq为PWM的频率单位为KHz
void TIM1_PWM_Init(u16 pwm_freq)
{
	//定义相应结构体
	NVIC_InitTypeDef 					NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef 	TIM_TimeBaseStructure;
	TIM_OCInitTypeDef       	TIM_OCInitStructure;
	GPIO_InitTypeDef 					GPIO_InitStructure;  
	
	//1、使能IO和定时器时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); 
	
	//配置上桥臂的三路PWM引脚//PWM_AH | PWM_BH | PWM_CH
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;//复用开漏输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//配置下桥臂三路IO口//PWM_AL | PWM_BL | PWM_CL
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_OD;//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
			
	//2、定时器参数初始化
	//计算公式：PWM周期 = （(重装载值+1)*(预分频系数+1)/时钟频率）*2;
 	TIM_DeInit(TIM1);//将TIM1寄存器值设置为默认值
	TIM_TimeBaseStructure.TIM_Period = 36000 / pwm_freq - 1;//重装载值
  TIM_TimeBaseStructure.TIM_Prescaler = 0;//预分频系数
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;//PWM中央对其模式，设置上溢中断             
  TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim                 
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; //设置进入中断的次数，0每次都进入     
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	//初始化三个通道的输出比较参数
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//当计时器值小于比较器设定值时则TIMX输出脚此时输出有效电平。
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//比较输出使能
  TIM_OCInitStructure.TIM_Pulse = 0;//设置待装入捕获比较寄存器的脉冲值，PWM占空比，CCR初始值为0
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High ; //输出极性:TIM输出比较极性为高电平
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;//空闲时输出低电平
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);                         
  TIM_OC2Init(TIM1, &TIM_OCInitStructure);
  TIM_OC3Init(TIM1, &TIM_OCInitStructure);
  
  TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);//CH1预装载使能               
  TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);//CH2预装载使能
  TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);//CH3预装载使能
 
	//3、开启定时器中断
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);  
	TIM_ClearITPendingBit(TIM1, TIM_IT_Update);                            //清中断标志位
  TIM_ITConfig(TIM1,TIM_IT_Update ,ENABLE);                              //打开中断 
	
	//配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;//设置中断号
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//响应优先级
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//中断通道使能
  NVIC_Init(&NVIC_InitStructure);
	
	//4、使能定时器
	TIM_Cmd(TIM1, ENABLE);//使能TIM1
	TIM_CtrlPWMOutputs(TIM1, ENABLE);//使能PWM
	
	ALLMOS_OFF;
}

//该定时器用于捕获PPM信号
void TIM2_CAP_Init(u16 arr,u16 psc)	
{
	GPIO_InitTypeDef 					GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	TIM_ICInitTypeDef 				TIM_ICInitStructure;  
 	NVIC_InitTypeDef 					NVIC_InitStructure;
	
	//配置IO和定时器时钟源
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能TIM2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);  //使能GPIOB和重映射时钟
	
	//重映射为普通IO
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); 
	//复用为TIM2CH2
  GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);  
	
	//PB3是PPM接收管脚
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PB3 下拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//初始化定时器2
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM2_CH2输入捕获参数
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC2S=02 	选择输入端 IC2映射到TI1上
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM_ICInitStructure.TIM_ICFilter = 0x00;//配置输入滤波器 不滤波
  TIM_ICInit(TIM2, &TIM_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update|TIM_IT_CC2);//清中断标志	
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许捕获中断	
  TIM_Cmd(TIM2,ENABLE ); 	//使能定时器2
}


//定时器时间计算公式Tout = ((重装载值+1)*(预分频系数+1))/时钟频率;
//例如：1秒定时，重装载值=9999，预分频系数=7199
//该定时器只做定时中断使用
void TIM3_Init(u16 arr,u16 psc)	
{
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
 	NVIC_InitTypeDef 					NVIC_InitStructure;

	//1、使能定时器时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	
	
	//2、定时器参数初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //预分频器   
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//3、开启定时器中断配置NVIC
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//允许更新中断	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	TIM_Cmd(TIM3,ENABLE); 	//使能定时器3	
}
