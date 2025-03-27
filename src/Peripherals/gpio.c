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
	  RCC->AHBENR = RCC_AHBENR_GPIOAEN;
	  GPIOA->MODER |= 0x400;

	  GPIOA->MODER &= ~GPIO_MODER_MODER5; // RE_TE
	  GPIOA->MODER |= GPIO_MODER_MODER5_0;
}

void MODBUS_RE_TE_HIGH()
{
	GPIOA->ODR |= GPIO_ODR_ODR_5;
}

void MODBUS_RE_TE_LOW()
{
	GPIOA->ODR &= ~GPIO_ODR_ODR_5;
}

void LED_BUILTIN_HIGH()
{
    GPIOA->ODR |= GPIO_ODR_ODR_5;
}

void LED_BUILTIN_LOW()
{
    GPIOA->ODR &= ~GPIO_ODR_ODR_5;
}
