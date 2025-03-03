/*
 * usart.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#include "usart.h"

void USART2_init()
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; 	//set bit 17 (USART2 EN)
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 	//enable GPIOA port clock bit 0 (GPIOA EN)
	GPIOA->AFR[0] = 0x00000700;	//GPIOx_AFRL p.188,AF7 p.177
	GPIOA->AFR[0] |= 0x00007000;	//GPIOx_AFRL p.188,AF7 p.177
	GPIOA->MODER |= 0x00000020; 	//MODER2=PA2(TX) to mode 10=alternate function mode. p184
	GPIOA->MODER |= 0x00000080; 	//MODER2=PA3(RX) to mode 10=alternate function mode. p184

	USART2->BRR = 0x00000D05;	//9600 BAUD and crystal 32MHz. p710, 116
	USART2->CR1 |= USART_CR1_TE;	//TE bit. p739-740. Enable transmit
	USART2->CR1 |= USART_CR1_RE;	//RE bit. p739-740. Enable receiver
	USART2->CR1 |= USART_CR1_UE;	//UE bit. p739-740. Uart enable
	USART2->CR1 |= USART_CR1_RXNEIE;			//enable RX interrupt
	NVIC_EnableIRQ(USART2_IRQn); 	//enable interrupt in NVIC
}

char USART2_read()
{
	char data=0;
	//wait while RX buffer is data is ready to be read
	while(!(USART2->SR&0x0020)){} 	//Bit 5 RXNE: Read data register not empty
		data=USART2->DR;			//p739
		return data;
}

void USART2_write(char data)
{
	while(!(USART2->SR & 0x0080)){} 	//TXE: Transmit data register empty. p736-737
	USART2->DR = (data);		//p739
}

void USART2_write_buffer(char* buffer)
{
	while(*(buffer) != '\0')
	{
		USART2_write(*buffer);
		buffer++;
	}
	USART2_write('\r');
	USART2_write('\n');
}

void USART2_IRQHandler(void)
{
	MODBUS_IRQHandler();
}
