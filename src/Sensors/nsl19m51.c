
 /*
 * nsl19m51.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#include "nsl19m51.h"
#include "adc.h"
#include <stdio.h>
#include <math.h>

// PIN PA1

void NSL19M51_init()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER1;
}

void NSL19M51_read(char *buffer, int buffer_size)
{
	int adc_result = 0;

	ADC1->SQR5 |= 1;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;

	while(!(ADC1->SR & ADC_SR_EOC)){}

	adc_result = ADC1->DR;

	ADC1->CR2 &= ~ADC_CR2_ADON;

	float voltage = ADC_STEP_SIZE_U * adc_result;
	float lux = 1.9634f * exp(2.1281f * voltage);
	snprintf(buffer, buffer_size,"Lux : %.2f && Voltage: %.2f" , lux, voltage);
}
