/*
 * lmt84lp.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */


#include "stm32l1xx.h"
#include "lmt84lp.h"
#include <stdio.h>
// Mittaa lämpötilaa väliltä -50-150C. -50C --> 1299mV ja 150C --> 183mV
// PIN PA0

void LMT84LP_init()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER0;
}

void LMT84LP_read(char *buffer, int buffer_size)
{
	int result = 0;
	ADC1->CR2|=0x40000000;
	while(!(ADC1->SR & 2)){}

	result=ADC1->DR;

	snprintf(buffer, buffer_size,"%d",result);
}
