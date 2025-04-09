#include "stm32l1xx.h"
#define HSI_VALUE    ((uint32_t)16000000)
#include "nucleo152start.h"

#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "modbus.h"

#include "lmt84lp.h"
#include "nsl19m51.h"
#include "dht22.h"
#include "sgp30.h"

#include "timing.h"
#include "timers.h"

#include <stdio.h>

#define BUFFER_SIZE 50

int main(void)
{
	SetSysClock();
	SystemCoreClockUpdate();

	// Utils Initializations

	// Peripheral Initializations
	GPIO_init();
	USART1_init();
	USART2_init();
	TIM2_Init();
	ADC_init();

	// Sensor Initializations
	LMT84LP_init();
	//NSL19M51_init();
	//DHT22_init();
    sensirion_i2c_init();

	MODBUS_RE_TE_LOW();

    while (1)
    {
		MODBUS_ProcessFrame();
    }

    return 0;
}
