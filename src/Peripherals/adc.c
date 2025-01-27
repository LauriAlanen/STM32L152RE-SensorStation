/*
 * adc.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#include "stm32l1xx.h"
#include "adc.h"

// This function needs to be called before using sensors!
void ADC_init()
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

	ADC1->SQR5 |= ADC_SQR5_SQ1; // Start at channel one
	ADC1->CR2 &= ~ADC_CR2_CONT; // Single conversion mode
	ADC1->SMPR3 |= ADC_SMPR3_SMP0; // 384 bit conversion rate
	ADC1->CR1 &= ~ADC_CR1_RES; // 12-Bit Resolution

	ADC1->CR2 |= ADC_CR2_ADON; // ADC on
}
