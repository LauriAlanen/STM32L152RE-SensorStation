/*
 * lmt84lp.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */


#include "lmt84lp.h"
#include "adc.h"
#include <stdio.h>
#include "usart.h"

// Mittaa l�mp�tilaa v�lilt� -50-150C. -50C --> 1299mV ja 150C --> 183mV
// PIN PA0

#define LMT84LP_MODBUS_ADDRESS 0x1

#define T_MAX 150.0f
#define T_MIN -50.0f
#define U_MIN 1.299f
#define U_MAX 0.183f

void LMT84LP_init()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER0;
}

void LMT84LP_read(MODBUS_Reading *reading)
{
	ADC1->SQR5 &= ~CHANNEL_MASK;
	ADC1->SQR5 |= 0;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;

	while(!(ADC1->SR & ADC_SR_EOC)){}
	reading->raw_reading[0] = ADC1->DR;

	ADC1->CR2 &= ~ADC_CR2_ADON;
}

void LMT84LP_ModbusHander(MODBUS_Reading *reading)
{
	LMT84LP_read(reading);
}
