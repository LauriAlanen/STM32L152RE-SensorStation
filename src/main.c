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
#include "modbus.h"
#include <stdio.h>

#define BUFFER_SIZE 50

int main(void)
{
	char buffer[BUFFER_SIZE];

	// CMSIS Initializations

	SetSysClock();
	SystemCoreClockUpdate();

	// Utils Initializationsd

	// Peripheral Initializations
	GPIO_init();
	USART2_init();
	TIM2_Init();

	// Sensor Initializations
	//LMT84LP_init();
	//NSL19M51_init();
	DHT22_init();
	//ADC_init();

	while (1)
	{
		MODBUS_ProcessFrame();
		delay_ms(1);
	}
	return 0;
}
