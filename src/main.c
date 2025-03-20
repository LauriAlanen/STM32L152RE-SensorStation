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
	USART2_init();
	TIM2_Init();

	// Sensor Initializations
	//LMT84LP_init();
	//NSL19M51_init();
	//DHT22_init();

    int16_t err;
    uint16_t tvoc_ppb, co2_eq_ppm;
    char buffer[128];

    sensirion_i2c_init();

    while (1) {
        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK)
        {
            sprintf(buffer, "tVOC  Concentration: %dppb\r\n", tvoc_ppb);
            USART2_write_buffer(buffer);
            sprintf(buffer, "CO2eq Concentration: %dppm\r\n", co2_eq_ppm);
            USART2_write_buffer(buffer);
        }

        else
        {
            USART2_write_buffer("error reading IAQ values\r\n");
        }

        delay_ms(1000);  // SGP30 measurement interval
    }
    return 0;
}
