/*
 * usart.h
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#ifndef PERIPHERALS_USART_H_
#define PERIPHERALS_USART_H_

#include "modbus.h"
#include "stm32l1xx.h"

void USART1_init();
void USART1_write(uint8_t data);
char USART1_read();
void USART1_write_buffer(uint8_t* buffer);

void USART2_init();
void USART2_write(char data);
char USART2_read();
void USART2_write_buffer(uint8_t* buffer);


#endif /* PERIPHERALS_USART_H_ */
