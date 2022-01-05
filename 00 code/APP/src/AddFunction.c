#include "addfun.h"

u16 OFFSET_A,OFFSET_B,OFFSET_C,OFFSET_N,OFFSET_I;//ADC∏˜Õ®µ¿µƒ∆´÷√÷µ

void MOSSHORT_Check(void)
{
	ALLMOS_OFF;
	MOSAN_ON;
	if(ADC_Data[BIT_I]>SHORT_CURRENT)//A+∂Ã¬∑π ’œ
	{
		ALLMOS_OFF;
		MC.FAULT=1;
		LED_Blink(1);//LED…¡À∏
		while(1);
	}
	
	ALLMOS_OFF;
	MOSBN_ON;
	if(ADC_Data[BIT_I]>SHORT_CURRENT)//B+∂Ã¬∑π ’œ
	{
		ALLMOS_OFF;
		MC.FAULT=2;
		LED_Blink(2);//LED…¡À∏
		while(1);
	}
	
	ALLMOS_OFF;
	MOSCN_ON;
	if(ADC_Data[BIT_I]>SHORT_CURRENT)//C+∂Ã¬∑π ’œ
	{
		ALLMOS_OFF;
		MC.FAULT=3;
		LED_Blink(3);//LED…¡À∏
		while(1);
	}
	
	ALLMOS_OFF;
	MOSAP_PWM(CHECK_DUTY);
	if(ADC_Data[BIT_I]>SHORT_CURRENT)//A-∂Ã¬∑π ’œ
	{
		ALLMOS_OFF;
		MC.FAULT=4;
		LED_Blink(4);//LED…¡À∏
		while(1);
	}
	
	ALLMOS_OFF;
	MOSBP_PWM(CHECK_DUTY);
	if(ADC_Data[BIT_I]>SHORT_CURRENT)//B-∂Ã¬∑π ’œ
	{
		ALLMOS_OFF;
		MC.FAULT=5;
		LED_Blink(5);//LED…¡À∏
		while(1);
	}
	
	ALLMOS_OFF;
	MOSCP_PWM(CHECK_DUTY);
	if(ADC_Data[BIT_I]>SHORT_CURRENT)//C-∂Ã¬∑π ’œ
	{
		ALLMOS_OFF;
		MC.FAULT=6;
		LED_Blink(6);//LED…¡À∏
		while(1);
	}
	ALLMOS_OFF;
}

void MOSOPEN_Check(void)
{
	u16 i=0;
	
	//A+
	ALLMOS_OFF;
	MOSBN_ON;
	MOSCN_ON;
	for (i=0;i<1000;i++)
	{
		MOSAP_PWM(CHECK_DUTY);
		delay_us(50);
		if(ADC_Data[BIT_A]<POPEN_VOL)
		{
			ALLMOS_OFF;
			MC.FAULT=7;
			LED_Blink(1);
			while(1);
		}
		MOSAP_OFF;
		delay_us(150);
	}
	ALLMOS_OFF;
	//B+
	ALLMOS_OFF;
	MOSAN_ON;
	MOSCN_ON;
	for (i=0;i<1000;i++)
	{
		MOSBP_PWM(CHECK_DUTY);
		delay_us(20);
		if(ADC_Data[BIT_B]<POPEN_VOL)
		{
			ALLMOS_OFF;
			MC.FAULT=8;
			LED_Blink(2);
			while(1);
		}
		MOSBP_OFF;
		delay_us(180);
	}
	//C+
	ALLMOS_OFF;
	MOSAN_ON;
	MOSBN_ON;
	for (i=0;i<1000;i++)
	{
		MOSCP_PWM(CHECK_DUTY);
		delay_us(30);
		if(ADC_Data[BIT_C]<POPEN_VOL)
		{
			ALLMOS_OFF;
			MC.FAULT=9;
			LED_Blink(3);
			while(1);
		}
		MOSCP_OFF;
		delay_us(170);
	}
	//A-
	ALLMOS_OFF;
	MOSBP_PWM(CHECK_DUTY);
	MOSCP_PWM(CHECK_DUTY);
	for (i=0;i<1000;i++)
	{
		MOSAN_ON;
		delay_us(20);
		if(ADC_Data[BIT_A]>NOPEN_VOL)
		{
			ALLMOS_OFF;
			MC.FAULT=10;
			LED_Blink(4);
			while(1);
		}
		MOSAN_OFF;
		delay_us(180);
	}
	//B-
	ALLMOS_OFF;
	MOSAP_PWM(CHECK_DUTY);
	MOSCP_PWM(CHECK_DUTY);
	for (i=0;i<1000;i++)
	{
		MOSBN_ON;
		delay_us(20);
		if(ADC_Data[BIT_B]>NOPEN_VOL)
		{
			ALLMOS_OFF;
			MC.FAULT=11;
			LED_Blink(5);
			while(1);
		}
		MOSBN_OFF;
		delay_us(180);
	}
	//C-
	ALLMOS_OFF;
	MOSAP_PWM(CHECK_DUTY);
	MOSBP_PWM(CHECK_DUTY);
	for (i=0;i<1000;i++)
	{
		MOSCN_ON;
		delay_us(20);
		if(ADC_Data[BIT_C]>NOPEN_VOL)
		{
			ALLMOS_OFF;
			MC.FAULT=12;
			LED_Blink(6);
			while(1);
		}
		MOSCN_OFF;
		delay_us(180);
	}
	ALLMOS_OFF;
}

void OFFSET2BAT_Read(void)
{
	u8 i=0;
	u16 A_VAL[5],B_VAL[5],C_VAL[5],N_VAL[5],I_BUS[5],U_BUS[5],V_BUS=0;
	
	ALLMOS_OFF;
	delay_ms(100);
	for(i=0;i<5;i++)
	{
		A_VAL[i]=ADC_Data[BIT_A];
		B_VAL[i]=ADC_Data[BIT_B];
		C_VAL[i]=ADC_Data[BIT_C];
		N_VAL[i]=ADC_Data[BIT_N];
		I_BUS[i]=ADC_Data[BIT_I];
		U_BUS[i]=ADC_Data[BIT_U];
		delay_ms(10);
	}
	OFFSET_A = (A_VAL[0]+A_VAL[1]+A_VAL[2]+A_VAL[3]+A_VAL[4])/5.0;
	OFFSET_B = (B_VAL[0]+B_VAL[1]+B_VAL[2]+B_VAL[3]+B_VAL[4])/5.0;
	OFFSET_C = (C_VAL[0]+C_VAL[1]+C_VAL[2]+C_VAL[3]+C_VAL[4])/5.0;
	OFFSET_N = (N_VAL[0]+N_VAL[1]+N_VAL[2]+N_VAL[3]+N_VAL[4])/5.0;
	OFFSET_I = (I_BUS[0]+I_BUS[1]+I_BUS[2]+I_BUS[3]+I_BUS[4])/5.0;
	V_BUS = (U_BUS[0]+U_BUS[1]+U_BUS[2]+U_BUS[3]+U_BUS[4])/5.0;
	
		/*
	2S:7.4-8.4
	Center:9.75
	3S:11.1-12.6
	Center:13.7
	3S:14.8-16.8
	*/
	//<7:«∑—π±£ª§
	//7-9.75:2S
	//9.75-13.7:3S
	//13.7-17:4S
	//>17:π˝—π±£ª§
	//≈–∂œµÁ—π∑∂Œß£¨≈–∂œµÁ≥ÿ÷÷¿‡
	if (V_BUS < 1639)//«∑—π
	{
		ALLMOS_OFF;
		MC.FAULT = 14;
		LED_Blink(4);
		while(1);
	}
	else if (V_BUS > 1639 && V_BUS < 2283)//2S
	{
		MC.BAT = 1;
		MC.OVER_VOL = 2107;//9V
		MC.UNDER_VOL = 1639;//7V
	}
	else if (V_BUS > 2283 && V_BUS < 3208)//3S
	{
		MC.BAT = 2;
		MC.OVER_VOL = 3044;//13V
		MC.UNDER_VOL = 2575;//11V
	}
	else if (V_BUS > 3208 && V_BUS < 3980)//4S
	{
		MC.BAT = 3;
		MC.OVER_VOL = 3161;//13.5V
		MC.UNDER_VOL = 3980;//17V
	}
	else if (V_BUS > 3980)//π˝—π
	{
		ALLMOS_OFF;
		MC.FAULT = 13;
		LED_Blink(3);
		while(1);
	}
	else
	{
		ALLMOS_OFF;
		MC.FAULT = 18;
		LED_Blink(10);
		while(1);
	}
}

