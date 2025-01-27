/*
 * usart.h
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#ifndef PERIPHERALS_USART_H_
#define PERIPHERALS_USART_H_

void USART2_init();

void USART2_write(char data);
void USART2_write_buffer(char* buffer, int buffer_size);

#endif /* PERIPHERALS_USART_H_ */
