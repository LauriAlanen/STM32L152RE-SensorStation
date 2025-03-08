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
	//DHT22_init();
	//ADC_init();

	while (1)
	{
		MODBUS_ProcessFrame();
		delay_ms(1);
		//NSL19M51_Reading nsl19m51_reading;
		//NSL19M51_read(&nsl19m51_reading);
		//snprintf(buffer, 100, "NSL19M51 Lux %d.%d", nsl19m51_reading.lux_int, nsl19m51_reading.lux_dec);
		//USART2_write_buffer(buffer);

		//LMT84LP_Reading lmt84lp_reading;
		//LMT84LP_read(&lmt84lp_reading);
		//snprintf(buffer, 100, "LMT84LP Temperature %d.%d", lmt84lp_reading.temperature_int, lmt84lp_reading.temperature_dec);
		//USART2_write_buffer(buffer);

		//DHT22_Reading dht22_reading;
		//if(!(DHT22_read(&dht22_reading)))
		//{
		//	snprintf(buffer, 100, "DHT22 Humidity %d.%d", dht22_reading.humidity_int, dht22_reading.humidity_dec);
		//	USART2_write_buffer(buffer);
		//	snprintf(buffer, 100, "DHT22 Temperature %d.%d", dht22_reading.temperature_int, dht22_reading.temperature_dec);
		//	USART2_write_buffer(buffer);
		//}
	}
	return 0;
}
