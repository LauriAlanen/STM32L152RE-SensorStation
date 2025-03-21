/*
 * usart.c
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#include "usart.h"

void USART1_init(void)
{
	RCC->APB2ENR|=(1<<14);	 	//set bit 14 (USART1 EN) p.156
	RCC->AHBENR|=0x00000001; 	//enable GPIOA port clock bit 0 (GPIOA EN)
	GPIOA->AFR[1]=0x00000700;	//GPIOx_AFRL p.189,AF7 p.177 (AFRH10[3:0])
	GPIOA->AFR[1]|=0x00000070;	//GPIOx_AFRL p.189,AF7 p.177 (AFRH9[3:0])
	GPIOA->MODER|=0x00080000; 	//MODER2=PA9(TX)D8 to mode 10=alternate function mode. p184
	GPIOA->MODER|=0x00200000; 	//MODER2=PA10(RX)D2 to mode 10=alternate function mode. p184

	USART1->BRR = 0x00000D05;	//9600 BAUD and crystal 32MHz. p710, D05
	USART1->CR1 = 0x00000008;	//TE bit. p739-740. Enable transmit
	USART1->CR1 |= 0x00000004;	//RE bit. p739-740. Enable receiver
	USART1->CR1 |= 0x00002000;	//UE bit. p739-740. Uart enable

	USART1->CR1 |= USART_CR1_SBK; // SBK bit. Send break enabled

	USART1->CR2 = 0x00; // reset

	USART1->CR3 = 0;   // Set to default state
	USART1->CR3 |= 1;  // Enable error interrupt,  p744
	/* Error Interrupt Enable Bit is required to enable interrupt generation in case of a framing
	error, overrun error or noise flag (FE=1 or ORE=1 or NF=1 in the USART_SR register) in
	case of Multi Buffer Communication (DMAR=1 in the USART_CR3 register).
	 */
}

char USART1_read()
{
	char data = 0;

	while(!(USART1->SR & USART_SR_RXNE)){} 	//Bit 5 RXNE: Read data register not empty
		data=USART1->DR;			//p739
		return data;
}

void USART1_write(uint8_t data)
{
	while(!(USART1->SR & USART_SR_TXE)){} 	//TXE: Transmit data register empty. p736-737
	USART1->DR = (data);		//p739
}

void USART1_write_buffer(uint8_t* buffer)
{
	while(*(buffer) != '\0')
	{
		USART1_write(*buffer);
		buffer++;
	}
	USART1_write('\r');
	USART1_write('\n');
}

void USART1_IRQHandler(void)
{
	MODBUS_IRQHandler();
}

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
	//USART2->CR1 |= USART_CR1_RXNEIE;			//enable RX interrupt
	//NVIC_EnableIRQ(USART2_IRQn); 	//enable interrupt in NVIC
}

char USART2_read()
{
	char data = 0;

	while(!(USART2->SR & USART_SR_RXNE)){} 	//Bit 5 RXNE: Read data register not empty
		data=USART2->DR;			//p739
		return data;
}

void USART2_write(char data)
{
	while(!(USART2->SR & USART_SR_TXE)){} 	//TXE: Transmit data register empty. p736-737
	USART2->DR = (data);		//p739
}

void USART2_write_buffer(uint8_t* buffer)
{
	while(*(buffer) != '\0')
	{
		USART2_write(*buffer);
		buffer++;
	}
	USART2_write('\r');
	USART2_write('\n');
}

//void USART2_IRQHandler(void)
//{
//	MODBUS_IRQHandler();
//}
