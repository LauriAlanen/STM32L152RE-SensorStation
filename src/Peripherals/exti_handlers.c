/*
 * exti_handlers.c
 *
 *  Created on: 14 Feb 2025
 *      Author: lauri
 */

#include "exti_handlers.h"

void EXTI9_5_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR7)
	{
        DHT22_IRQHandler();
	}
}

void USART1_IRQHandler(void)
{
	MODBUS_IRQHandler();
}
