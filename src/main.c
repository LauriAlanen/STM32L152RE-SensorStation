#include "stm32l1xx.h"
#define HSI_VALUE    ((uint32_t)16000000)
#include "nucleo152start.h"

// Atollic studio ei tykkää .git hakemistosta!

#include "adc.h"
#include "usart.h"
#include "lmt84lp.h"
#include "gpio.h"

#define BUFFER_SIZE 50

void delay_Ms(int delay);

int main(void)
{
	char buffer[BUFFER_SIZE];

	SetSysClock();
	SystemCoreClockUpdate();
	GPIO_init();
	USART2_init();
	LMT84LP_init();
	ADC_init();

	while (1)
	{
		GPIOA->ODR|=0x20; //0010 0000 set bit 5. p186
		LMT84LP_read(buffer, BUFFER_SIZE);
		USART2_write_buffer(buffer, BUFFER_SIZE);
		delay_Ms(500);
		GPIOA->ODR&=~0x20; //0000 0000 clear bit 5. p186
		delay_Ms(500);
	}
	return 0;
}

void delay_Ms(int delay)
{
	int i=0;
	for(; delay>0;delay--)
		for(i=0;i<2460;i++); //measured with oscilloscope
}
