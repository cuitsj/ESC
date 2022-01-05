#ifndef __TIMER_H__
#define __TIMER_H__

#include "include.h"

void TIM1_PWM_Init(u16 pwm_freq);
void TIM2_CAP_Init(u16 arr,u16 psc);
void TIM3_Init(u16 arr, u16 psc);

#endif
