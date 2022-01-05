#ifndef __MC_H__
#define __MC_H__

//头文件
#include "include.h"

//宏定义
#define MOSAP_PWM(duty) 	TIM_SetCompare1(TIM1,duty);
#define MOSAN_ON					GPIO_SetBits(GPIOB, GPIO_Pin_15);
#define MOSBP_PWM(duty)		TIM_SetCompare2(TIM1,duty);
#define MOSBN_ON					GPIO_SetBits(GPIOB, GPIO_Pin_14);
#define MOSCP_PWM(duty)		TIM_SetCompare3(TIM1,duty);
#define MOSCN_ON					GPIO_SetBits(GPIOB, GPIO_Pin_13);
#define MOSAP_OFF 				TIM_SetCompare1(TIM1,0);
#define MOSAN_OFF 				GPIO_ResetBits(GPIOB, GPIO_Pin_15);
#define MOSBP_OFF					TIM_SetCompare2(TIM1,0);
#define MOSBN_OFF					GPIO_ResetBits(GPIOB, GPIO_Pin_14);
#define MOSCP_OFF					TIM_SetCompare3(TIM1,0);
#define MOSCN_OFF					GPIO_ResetBits(GPIOB, GPIO_Pin_13);

#define ALLMOS_OFF \
											do{													\
													TIM_SetCompare1(TIM1,0);\
													TIM_SetCompare2(TIM1,0);\
													TIM_SetCompare3(TIM1,0);\
													GPIO_ResetBits(GPIOB, GPIO_Pin_13);\
													GPIO_ResetBits(GPIOB, GPIO_Pin_14);\
													GPIO_ResetBits(GPIOB, GPIO_Pin_15);\
											}while(0);//关闭所有MOS
											
#define  	MC_DEFAULTS   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} 	// 初始化参数
#define 	CLOSED_LOOP 	1							//切入闭环标志
#define 	POLES 				7							//电机极对数，仅仅对计算转速有影响

#define FirstOrder_LPF(OLD, NEW, COE) do{NEW = (1-COE)*NEW + COE*OLD;}while(0); //一阶低通滤波
#define LOOP 	0 	//0：电流环，1：速度环+电流环，2：开环
#define OUP   0		//过欠压保护
#define OCP   0		//过流保护
#define OPP 	0		//1：开启缺相保护，0：关闭缺相保护
#define MSLP 	0		//1：开启堵转保护，0：关闭堵转保护

//变量声明										
typedef struct {
	u8		CCW;      //1：反转，0：正传
	u8		PHA_CNT;  //换相计数
	u8		STATE;		//电机状态
	u8		ZCD_CNT;	//过零次数计数
	u8		BAT;			//电池种类，1代表2S，2代表3S，3代表4S
	u8		FAULT;		//故障记录，//1-6表示A+,B+,C+,A-,B-,C-的短路故障，7-12表示A+,B+,C+,A-,B-,C-不能开通故障，13-17:过压、欠压、过流、堵转、缺相,18:其他故障
	u16		DUTY;     //占空比
	u16   DELAY30;	//延时30度计数
	u16		CYCLES;		//电机转过的圈数计数
	u16		SPEED;		//电机转速
	u16   AI;				//A相电流
	u16		BI;				//B相电流
	u16		CI;				//C相电流
	u16		OVER_VOL;	//过压值
	u16		UNDER_VOL;//欠压值
	float ADJ30;		//延时30度调节系数0.7，不能完全延时前30度走过的时间，需要适当缩小,过小会降低电调效率，过大会错过换相区间导致电机卡顿
}Motor_Control;

extern Motor_Control MC;

//函数声明
void PARAM_Init(void);
void MOTOR_Start(void);
void phase_change(void);
void MOS_ABON(void);
void MOS_ACON(void);
void MOS_BCON(void);
void MOS_BAON(void);
void MOS_CAON(void);
void MOS_CBON(void);


void ZCD_A_UP(void);
void ZCD_B_UP(void);
void ZCD_C_UP(void);
void ZCD_A_DOWN(void);
void ZCD_B_DOWN(void);
void ZCD_C_DOWN(void);

#endif
