#include "SysTick.h"
//72M条件下这个定时器最长延时1.864135秒
//全局变量加上 static 关键字，它的作用域就变成了当前文件，在其它文件中就无效了。
static u8  cnt_us=0;							//SysTick计数一微秒的计数值			   
static u16 cnt_ms=0;							//SysTick计数一毫秒的计数值			

//初始化延迟函数
//HCLK时钟即为AHB时钟
//SystemCoreClock:系统时钟72M
void DELAY_Init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择AHB时钟的8分频即9MHz作为SysTick时钟，
	cnt_us=SystemCoreClock/8000000;				//9M计数9次为一微秒
	cnt_ms=(u16)cnt_us*1000;				//9M计数9000次为一毫秒
}

//延时nus
//nus为要延时的us数.	
//对72M条件下,nus<=1864135 
void delay_us(u32 nus)
{		
	u32 temp;	
	SysTick->LOAD=nus*cnt_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0x00;      					 //清空计数器	 
}

//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对72M条件下,nms<=1864 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		 
	SysTick->LOAD=(u32)nms*cnt_ms;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0x00;       					//清空计数器	  	    
} 


