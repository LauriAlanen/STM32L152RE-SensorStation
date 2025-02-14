/*
 * timers.c
 *
 *  Created on: 14 Feb 2025
 *      Author: lauri
 */

#include "timers.h"

void TIM2_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 32 - 1;
    TIM2->ARR = 0xFFFF;
    TIM2->CR1 |= TIM_CR1_CEN;
}

