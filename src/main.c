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
	// CMSIS Initializations

	SetSysClock();
	SystemCoreClockUpdate();

	// Utils Initializations

	// Peripheral Initializations
	GPIO_init();
	USART2_init();
	TIM2_Init();
    sensirion_i2c_init();

	// Sensor Initializations
	//LMT84LP_init();
	//NSL19M51_init();
	DHT22_init();
	//ADC_init();

	USART2_write_buffer("Test");

	int16_t probe;
    while (1) {
        probe = sgp30_probe();

        if (probe == STATUS_OK)
            break;

        if (probe == SGP30_ERR_UNSUPPORTED_FEATURE_SET)
        	USART2_write_buffer("Not working");

    	USART2_write_buffer("Not working");
        sensirion_sleep_usec(1000000);
    }

	USART2_write_buffer("It works");

	while (1)
	{
		MODBUS_ProcessFrame();
		delay_ms(1);
	}
	return 0;
}
