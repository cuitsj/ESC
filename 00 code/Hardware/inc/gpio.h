#ifndef __GPIO_H__
#define __GPIO_H__

#include "include.h"	  	

//LED
#define LED_ON  	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
#define LED_OFF  	GPIO_SetBits(GPIOC, GPIO_Pin_13);

//µ÷ÊÔ¹Ü½Å
#define DEBUG_PIN_HIGH		GPIO_SetBits(GPIOC, GPIO_Pin_14);
#define DEBUG_PIN_LOW			GPIO_ResetBits(GPIOC, GPIO_Pin_14);

void GPIO_COM_Init(void);
void LED_Blink(u8 cnt);

#endif
