#ifndef __ADC_H__
#define __ADC_H__

#include "include.h"

//定义ADC_Data数组的寻位宏，方便硬件改变ADC采样管脚后修改程序
#define 	BIT_A   	3
#define 	BIT_B			1
#define 	BIT_C			0
#define		BIT_N			2
#define 	BIT_I			4
#define 	BIT_U			5
extern volatile uint16_t ADC_Data[6];    //存储6个ADC数据的变量,0:C相电压|1:B相电压|2:中性点电压|3:A相电压|4:母线电流|5:母线电压

void ADC_DMA_Init(void);

#endif 
