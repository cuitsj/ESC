#include "MC.h"

Motor_Control		MC=MC_DEFAULTS;

//电机控制各种参数初始化
void PARAM_Init(void)
{
	MC.CCW=0;      	//1：反转，0：正传
	MC.PHA_CNT=0; 	//换相计数
	MC.STATE=0;			//电机状态
	MC.ZCD_CNT=0;		//过零次数计数
	MC.BAT=0;				//电池种类，1代表2S，2代表3S，3代表4S
	MC.FAULT=0;			//故障记录，//1-6表示A+,B+,C+,A-,B-,C-的短路故障，7-12表示A+,B+,C+,A-,B-,C-不能开通故障，13-17:过压、欠压、过流、堵转、缺相,18:其他故障
	MC.DUTY=0;     	//占空比
	MC.DELAY30=0;		//延时30度计数
	MC.CYCLES=0;		//电机转过的圈数计数
	MC.SPEED=0;			//电机转速
	MC.AI=0;				//A相电流
	MC.BI=0;				//B相电流
	MC.CI=0;				//C相电流
	MC.OVER_VOL=0;	//过压值
	MC.UNDER_VOL=0;	//欠压值
	MC.ADJ30=0.7;		//延时30度调节系数0.7，不能完全延时前30度走过的时间，需要适当缩小,过小会降低电调效率，过大会错过换相区间导致电机卡顿
}
//电机启动函数
void MOTOR_Start(void)
{
	u16 step_length=300;//初始换相步长为300，固定不变
	u16 i;
	
	MC.DUTY=110;//预定位占空比，空载100，带负载120，110两者都能启动
	MOS_ABON();
	delay_ms(50);//预定位时间50毫秒比较合适
	while(1)
	{
		for(i=0;i<step_length;i++)
		{
			delay_us(100);//延时100us，固定不变
		}
		step_length -= step_length/30+1;//当电机启动困难可以加大除数，带负载30比较合适

		if(step_length < 25)
		{
			if(CLOSED_LOOP)
			{
				MC.STATE=1;//电机进入下一个状态
				MC.DUTY=80;//电机初始运行占空比
				break;
			}
			else
			{	
				step_length = 25;   //开环强制换向，一直开环拖动电机旋转
			}				
		}
		MC.PHA_CNT++;//强制换相，拖动电机
		if(MC.PHA_CNT == 6)
		{
			MC.PHA_CNT = 0;
		}
		phase_change();//六步换相函数
	}
}

//A相过零检测函数,A相BEMF由低变高
void ZCD_A_UP(void)
{
	//延时30度计数
	MC.DELAY30++;
	if(ADC_Data[BIT_A]>ADC_Data[BIT_N])//检测到过零
	{
		MC.ZCD_CNT++;
		if(MC.ZCD_CNT==8)//检测到3次过零信号
		{
			MC.ZCD_CNT=0;
			MC.DELAY30=MC.DELAY30*MC.ADJ30;
			MC.STATE=2;
		}
	}
}

//B相过零检测函数,B相BEMF由低变高
void ZCD_B_UP(void)
{
	//延时30度计数
	MC.DELAY30++;
	if(ADC_Data[BIT_B]>ADC_Data[BIT_N])//检测到过零
	{
		MC.ZCD_CNT++;
		if(MC.ZCD_CNT==8)//检测到3次过零信号
		{
			MC.ZCD_CNT=0;
			MC.DELAY30=MC.DELAY30*MC.ADJ30;
			MC.STATE=2;
		}
	}
}

//C相过零检测函数,C相BEMF由低变高
void ZCD_C_UP(void)
{
	//延时30度计数
	MC.DELAY30++;
	if(ADC_Data[BIT_C]>ADC_Data[BIT_N])//检测到过零
	{
		MC.ZCD_CNT++;
		if(MC.ZCD_CNT==8)//检测到3次过零信号
		{
			MC.ZCD_CNT=0;
			MC.DELAY30=MC.DELAY30*MC.ADJ30;
			MC.STATE=2;
		}
	}
}

//A相过零检测函数,A相BEMF由高变低
void ZCD_A_DOWN(void)
{
	//延时30度计数
	MC.DELAY30++;
	if(ADC_Data[BIT_A]<ADC_Data[BIT_N])//检测到过零
	{
		MC.ZCD_CNT++;
		if(MC.ZCD_CNT==8)//检测到3次过零信号
		{
			MC.ZCD_CNT=0;
			MC.DELAY30=MC.DELAY30*MC.ADJ30;
			MC.STATE=2;
		}
	}
}

//B相过零检测函数,B相BEMF由高变低
void ZCD_B_DOWN(void)
{
	//延时30度计数
	MC.DELAY30++;
	if(ADC_Data[BIT_B]<ADC_Data[BIT_N])//检测到过零
	{
		MC.ZCD_CNT++;
		if(MC.ZCD_CNT==8)//检测到3次过零信号
		{
			MC.ZCD_CNT=0;
			MC.DELAY30=MC.DELAY30*MC.ADJ30;
			MC.STATE=2;
		}
	}
}

//C相过零检测函数,C相BEMF由高变低
void ZCD_C_DOWN(void)
{
	//延时30度计数
	MC.DELAY30++;
	if(ADC_Data[BIT_C]<ADC_Data[BIT_N])//检测到过零
	{
		DEBUG_PIN_LOW;
		MC.ZCD_CNT++;
		if(MC.ZCD_CNT==8)//检测到3次过零信号
		{
			MC.ZCD_CNT=0;
			MC.DELAY30=MC.DELAY30*MC.ADJ30;
			MC.STATE=2;
		}
	}
}

//六步换相函数
void phase_change(void)
{
	if(MC.CCW)//反转
	{
		switch(MC.PHA_CNT)
		{
			case 0: MOS_ABON(); break;
			case 1: MOS_ACON(); break;
			case 2: MOS_BCON(); break;
			case 3: MOS_BAON(); break;
			case 4: MOS_CAON(); break;
			case 5: MOS_CBON(); break;
			default: break;
		}
	}
	else//正转
	{
		switch(MC.PHA_CNT)
		{
			case 5: MOS_ABON(); break;
			case 4: MOS_ACON(); break;
			case 3: MOS_BCON(); break;
			case 2: MOS_BAON(); break;
			case 1: MOS_CAON(); break;
			case 0: MOS_CBON(); break;
			default: break;
		}
	}
}

//AB
void MOS_ABON(void)
{
	MOSAP_PWM(MC.DUTY);
	MOSAN_OFF;
	MOSBP_OFF;
	MOSBN_ON;
	MOSCP_OFF;
	MOSCN_OFF;
}

//AC
void MOS_ACON(void)
{
	MOSAP_PWM(MC.DUTY);
	MOSAN_OFF;
	MOSBP_OFF;
	MOSBN_OFF;
	MOSCP_OFF;
	MOSCN_ON;
}

//BC
void MOS_BCON(void)
{
	MOSAP_OFF;
	MOSAN_OFF;
	MOSBP_PWM(MC.DUTY);
	MOSBN_OFF;
	MOSCP_OFF;
	MOSCN_ON;
}

//BA
void MOS_BAON(void)
{
	MOSAP_OFF;
	MOSAN_ON;
	MOSBP_PWM(MC.DUTY);
	MOSBN_OFF;
	MOSCP_OFF;
	MOSCN_OFF;
}

//CA
void MOS_CAON(void)
{
	MOSAP_OFF;
	MOSAN_ON;
	MOSBP_OFF;
	MOSBN_OFF;
	MOSCP_PWM(MC.DUTY);
	MOSCN_OFF;
}

//CB
void MOS_CBON(void)
{
	MOSAP_OFF;
	MOSAN_OFF;
	MOSBP_OFF;
	MOSBN_ON;
	MOSCP_PWM(MC.DUTY);
	MOSCN_OFF;
}

