/*
 * gpio.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */


#include "gpio.h"
#include "stm32l1xx.h"

void GPIO_init()
{
	  RCC->AHBENR|=1;
	  GPIOA->MODER|=0x400;
}
