/*
 * sensirion_i2c.c
 *
 *  Created on: 20 Mar 2025
 *      Author: Lauri
 */

#include "sensirion_i2c.h"
#include "timing.h"
#include "sgp30.h"
#include "usart.h"

#define SGP30_CONN_RETRIES 5

int16_t sensirion_i2c_select_bus(uint8_t bus_idx)
{
    return 1;
}

uint8_t sensirion_i2c_init(void)
{
    int16_t probe, err = 0;
    uint8_t init_status = 1;

    I2C1_Init();

    for (int i = 0; i < SGP30_CONN_RETRIES; ++i)
    {
        probe = sgp30_probe();

        if (probe == STATUS_OK)
        {
        	init_status = 0;
        	break;
        }

        if (probe == SGP30_ERR_UNSUPPORTED_FEATURE_SET)
            USART2_write_buffer("Your sensor needs at least feature set version 1.0 (0x20)\r\n");

        USART2_write_buffer("SGP sensor probing failed\r\n");
        sensirion_sleep_usec(1000000);
    }

    if(init_status)
    {
    	return 1; // Error occured during init
    }

    err = sgp30_iaq_init();
    if (err == STATUS_OK)
    {
        USART2_write_buffer("sgp30_iaq_init done\r\n");
    }

    else
    {
        USART2_write_buffer("sgp30_iaq_init failed!\r\n");
    }

    return 0;
}

void sensirion_i2c_release(void)
{
    // TODO: Implement function
}

int8_t sensirion_i2c_read(uint8_t address, uint8_t* data, uint16_t count)
{
    I2C1_Read(address, count, data);
    return 0;
}

int8_t sensirion_i2c_write(uint8_t address, const uint8_t* data, uint16_t count)
{
    I2C1_Write(address, count, data);

    return 0;
}

void sensirion_sleep_usec(uint32_t useconds)
{
	delay_us(useconds);
}
