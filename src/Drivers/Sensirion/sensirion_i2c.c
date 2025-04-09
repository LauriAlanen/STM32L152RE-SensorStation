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
    int16_t probe;
    uint8_t init_status = 1;

    I2C1_Init();

	USART2_write_buffer("SGP30: Probing...");
    for (int i = 0; i < SGP30_CONN_RETRIES; ++i)
    {
        probe = sgp30_probe();

        if (probe == STATUS_OK)
        {
        	init_status = 0;
        	break;
        }
        USART2_write_buffer("...");
        sensirion_sleep_usec(1000000);
    }
    USART2_write_buffer("\n");

    if(init_status)
    {
    	USART2_write_buffer("SGP30: Init Error!\n");
    	return 1;
    }

    sgp30_iaq_init();
	USART2_write_buffer("SGP30: Init Success!\n");

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
