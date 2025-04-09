
 /*
 * nsl19m51.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#include "nsl19m51.h"
#include "adc.h"
#include <math.h>

// PIN PA1

void NSL19M51_init()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER1;
}

void NSL19M51_read(MODBUS_Reading *reading)
{
	ADC1->SQR5 |= 1;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;

	while(!(ADC1->SR & ADC_SR_EOC)){}

	reading->raw_reading[0] = ADC1->DR;

	ADC1->CR2 &= ~ADC_CR2_ADON;
}

void NSL19M51_ModbusHandler(MODBUS_Reading *reading)
{
	NSL19M51_read(reading);
}
