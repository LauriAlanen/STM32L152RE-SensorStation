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

int main(void) {
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

    uint16_t i = 0;
    int16_t err;
    uint16_t tvoc_ppb, co2_eq_ppm;
    uint32_t iaq_baseline;
    uint16_t ethanol_raw_signal, h2_raw_signal;
    char buffer[128];

    USART2_write_buffer("Starting SGP30");

    const char* driver_version = sgp30_get_driver_version();
    if (driver_version) {
        sprintf(buffer, "SGP30 driver version %s\r\n", driver_version);
        USART2_write_buffer(buffer);
    } else {
        USART2_write_buffer("fatal: Getting driver version failed\r\n");
        return -1;
    }

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without a sensor. */
    int16_t probe;
    while (1) {
        probe = sgp30_probe();

        if (probe == STATUS_OK)
            break;

        if (probe == SGP30_ERR_UNSUPPORTED_FEATURE_SET)
            USART2_write_buffer("Your sensor needs at least feature set version 1.0 (0x20)\r\n");

        USART2_write_buffer("SGP sensor probing failed\r\n");
        sensirion_sleep_usec(1000000);
    }

    USART2_write_buffer("SGP sensor probing successful\r\n");

    uint16_t feature_set_version;
    uint8_t product_type;
    err = sgp30_get_feature_set_version(&feature_set_version, &product_type);
    if (err == STATUS_OK) {
        sprintf(buffer, "Feature set version: %u\r\n", feature_set_version);
        USART2_write_buffer(buffer);
        sprintf(buffer, "Product type: %u\r\n", product_type);
        USART2_write_buffer(buffer);
    } else {
        USART2_write_buffer("sgp30_get_feature_set_version failed!\r\n");
    }

    uint64_t serial_id;
    err = sgp30_get_serial_id(&serial_id);
    if (err == STATUS_OK) {
        USART2_write_buffer(buffer);
    } else {
        USART2_write_buffer("sgp30_get_serial_id failed!\r\n");
    }

    /* Read gas raw signals */
    err = sgp30_measure_raw_blocking_read(&ethanol_raw_signal, &h2_raw_signal);
    if (err == STATUS_OK) {
        sprintf(buffer, "Ethanol raw signal: %u\r\n", ethanol_raw_signal);
        USART2_write_buffer(buffer);
        sprintf(buffer, "H2 raw signal: %u\r\n", h2_raw_signal);
        USART2_write_buffer(buffer);
    } else {
        USART2_write_buffer("error reading raw signals\r\n");
    }

    /* Initialize IAQ */
    err = sgp30_iaq_init();
    if (err == STATUS_OK) {
        USART2_write_buffer("sgp30_iaq_init done\r\n");
    } else {
        USART2_write_buffer("sgp30_iaq_init failed!\r\n");
    }
    /* (B) If a recent baseline is available, set it after sgp30_iaq_init() for faster start-up */

    /* Run periodic IAQ measurements at defined intervals */
    while (1) {
        /*
         * IMPLEMENT: get absolute humidity if needed and call
         * sgp30_set_absolute_humidity()
         */
        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK) {
            sprintf(buffer, "tVOC  Concentration: %dppb\r\n", tvoc_ppb);
            USART2_write_buffer(buffer);
            sprintf(buffer, "CO2eq Concentration: %dppm\r\n", co2_eq_ppm);
            USART2_write_buffer(buffer);
        } else {
            USART2_write_buffer("error reading IAQ values\r\n");
        }

        /* Persist the current baseline every hour */
        if (++i % 3600 == 3599) {
            err = sgp30_get_iaq_baseline(&iaq_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to persistent storage */
            }
        }

        /* The IAQ measurement must be triggered exactly once per second (SGP30) */
        delay_ms(1000);  // SGP30 measurement interval
    }
    return 0;
}
