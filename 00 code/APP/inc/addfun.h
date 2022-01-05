#ifndef __ADDFUN_H__
#define __ADDFUN_H__

//头文件
#include "include.h"

//宏定义
#define 	CHECK_DUTY 		200			//检测用的占空比为10%
#define 	SHORT_CURRENT 21845  		//MOS短路电流检测阈值
#define   POPEN_VOL			0					//上桥臂MOS开通电压检测阈值
#define 	NOPEN_VOL 		4095			//下桥臂MOS开通电压检测阈值


//变量声明
extern u16 OFFSET_A,OFFSET_B,OFFSET_C,OFFSET_N,OFFSET_I;

//函数声明
void MOSSHORT_Check(void);
void MOSOPEN_Check(void);
void OFFSET2BAT_Read(void);

#endif
