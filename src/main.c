#include "stm32l1xx.h"
#define HSI_VALUE    ((uint32_t)16000000)
#include "nucleo152start.h"

#include "adc.h"
#include "usart.h"
#include "lmt84lp.h"
#include "nsl19m51.h"
#include "dht22.h"
#include "gpio.h"
#include "timing.h"
#include "timers.h"

#define BUFFER_SIZE 50

int main(void)
{
	char buffer[BUFFER_SIZE];

	// CMSIS Initializations

	SetSysClock();
	SystemCoreClockUpdate();

	// Utils Initializations

	// Peripheral Initializations
	GPIO_init();
	USART2_init();
	TIM2_Init();

	// Sensor Initializations
	LMT84LP_init();
	NSL19M51_init();
	DHT22_init();
	ADC_init();

	while (1)
	{
		//GPIOA->ODR |= GPIO_ODR_ODR_5; //0010 0000 set bit 5. p186

		//LMT84LP_read(buffer, BUFFER_SIZE);
		//USART2_write_buffer(buffer, BUFFER_SIZE);

		//NSL19M51_read(buffer, BUFFER_SIZE);
		//USART2_write_buffer(buffer, BUFFER_SIZE);

		if(!(DHT22_read(buffer, BUFFER_SIZE)))
		{
			USART2_write_buffer(buffer);
		}

		//GPIOA->ODR &= ~GPIO_ODR_ODR_5; //0000 0000 clear bit 5. p186
		delay_ms(1000);
	}
	return 0;
}
