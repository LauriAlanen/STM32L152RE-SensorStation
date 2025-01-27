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

#define T_MAX 150.0f
#define T_MIN -50.0f
#define U_MIN 1.299f
#define U_MAX 0.183f
#define ADC_MAX 4096
#define ADC_STEP_SIZE_U 0.000805664f // 3.3 / 4096


void LMT84LP_init()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER0;
}

void LMT84LP_read(char *buffer, int buffer_size)
{
	int adc_result = 0;

	ADC1->SQR5 |= 0; // Start at channel zero
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;

	while(!(ADC1->SR & ADC_SR_EOC)){}

	adc_result = ADC1->DR;

	ADC1->CR2 &= ~ADC_CR2_ADON;

	float voltage = ADC_STEP_SIZE_U * adc_result;
	float temperature = ((voltage - U_MIN) / (U_MAX - U_MIN)) * (T_MAX - T_MIN) + T_MIN;

	snprintf(buffer, buffer_size,"Temperature : %.2f && Voltage: %.2f" , temperature, voltage);
}
