#include "adc.h"

volatile uint16_t ADC_Data[6];    //存储6个ADC数据的变量,0:C相电压|1:B相电压|2:中性点电压|3:A相电压|4:母线电流|5:母线电压

void ADC_DMA_Init(void)
{
	//定义三个结构体
	ADC_InitTypeDef       ADC_InitStructure;	
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;
	
	//使能相应时钟源
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
	
	//设置ADC分频因子为6分频，72M/6=12M,因为ADC的输入时钟不得超过14MHz
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);  
	
	//配置6个ADC通道的采样管脚
	//C相电压|B相电压|中性点电压|A相电压|母线电流|母线电压
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//模拟输入模式
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//ADC初始化
	ADC_DeInit(ADC1);  //复位ADC1
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//ADC工作模式:ADC1和ADC2工作相互独立
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;//多个通道转换需要使能扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//ADC数据右对齐,0x0FFF
	ADC_InitStructure.ADC_NbrOfChannel = 6;//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);

	//设置ADC各个通道的采样顺序和采样时间，时间越短误差越大
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_7Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_7Cycles5);  
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_7Cycles5); 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_7Cycles5); 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_7Cycles5); 

	ADC_Cmd(ADC1, ENABLE);//使能指定的ADC1
	ADC_DMACmd(ADC1, ENABLE); //开启ADC的DMA支持

	//DMA初始化
	DMA_DeInit(DMA1_Channel1);//将DMA1通道1的寄存器设为默认值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&(ADC1->DR);//DMA读取的ADC数据寄存器起始地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_Data;//DMA写入的内存起始地址 
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//方向(从外设到内存)

	DMA_InitStructure.DMA_BufferSize = 6;//传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//内存地址不固定，有6个变量的地址
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据宽度为16位，ADC一个数据的宽度
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//内存数据宽度

	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//禁止内存到内存的传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	ADC_ResetCalibration(ADC1);//使能复位校准
	while(ADC_GetResetCalibrationStatus(ADC1));//等待复位校准结束	
	ADC_StartCalibration(ADC1);//开启AD校准
	while(ADC_GetCalibrationStatus(ADC1));//等待校准结束
	ADC_SoftwareStartConvCmd(ADC1, ENABLE); //软件触发开始转换	
}
