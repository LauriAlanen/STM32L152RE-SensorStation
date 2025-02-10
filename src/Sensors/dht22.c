#include "dht22.h"
#include "timing.h"
#include "usart.h"
#include <stdio.h>

// PIN PA7
#define TIMEOUT_20_MS 640000
#define TIMEOUT_90_US 2880
#define TIMEOUT_50_US 1600
#define TIMEOUT_28_US 896

void DHT22_SWITCH_MODE_OUTPUT()
{
	GPIOA->MODER &= ~GPIO_MODER_MODER7;
	GPIOA->MODER |= GPIO_MODER_MODER7_0;
}

void DHT22_SWITCH_MODE_INPUT()
{
	GPIOA->MODER &= ~GPIO_MODER_MODER7;
}

void DHT22_init()
{
    DHT22_SWITCH_MODE_OUTPUT();
}

int DHT22_read(char *buffer, int buffer_size)
{
    char usart_buffer[100];
    DHT22_start();

    if (DHT22_wait_response())
    {
        USART2_write_buffer("DHT22 Not ready to send data!");
        return 1;
    }

    uint8_t current_byte = 0;
    uint8_t byte_list[5] = {0};

    for (uint8_t bit = 0; bit < 40; bit++)
    {
        if (!(bit & 7))
        {
            byte_list[(bit / 8)] = current_byte;
            current_byte = 0;
        }
        
        SysTick->LOAD = TIMEOUT_90_US - 1; // Set maximum allowable wait time
        SysTick->VAL = 0;
        SysTick->CTRL = 5;

        while (!(GPIOA->IDR & GPIO_IDR_IDR_7))
        {
            if ((SysTick->CTRL) & 0x10000)
            {
                snprintf(usart_buffer, 100, "Timeout error when waiting for line to go high at bit position %d", (int)bit);
                USART2_write_buffer(usart_buffer);
                SysTick->CTRL = 0;
                return 1;
            }
        }

        SysTick->LOAD = TIMEOUT_28_US - 1; // Set maximum allowable wait time
        SysTick->VAL = 0;
        SysTick->CTRL = 5;
        uint8_t bit_is_high = 0;

        while (GPIOA->IDR & GPIO_IDR_IDR_7)
        {
            if ((SysTick->CTRL) & 0x10000)
            {
                bit_is_high = 1;
                break;
            }
        }

        if (bit_is_high)
        {
            USART2_write('1');
            current_byte = (current_byte << 1) | 1; // If it's longer than 30µs, it's a 1
        }

        else
        {
            USART2_write('0');
            current_byte = (current_byte << 1) | 0; // Otherwise, it's a 0
        }

        SysTick->CTRL = 0;
    }

    byte_list[4] = current_byte; // Save last byte

    snprintf(buffer, buffer_size, "Temperature %u.%u and Humidity %u.%u", byte_list[2], byte_list[3], byte_list[0], byte_list[1]);
    return 0;
}

void DHT22_start()
{
	// MCU PULL LOW ~20ms
    DHT22_SWITCH_MODE_OUTPUT();
    GPIOA->ODR &= ~GPIO_ODR_ODR_7; 
    delay_ms(20);

    // MCU RELEASE LINE 20-40µs
    GPIOA->ODR |= GPIO_ODR_ODR_7;
    DHT22_SWITCH_MODE_INPUT();
	delay_us(20);

    SysTick->CTRL = 0;
}

int DHT22_wait_response()
{
    DHT22_SWITCH_MODE_INPUT();

    SysTick->LOAD = TIMEOUT_90_US - 1; // Set maximum allowable wait time
	SysTick->VAL = 0;
	SysTick->CTRL = 5;

    while (!(GPIOA->IDR & GPIO_IDR_IDR_7))
    {
        if ((SysTick->CTRL) & 0x10000)
        {
    		USART2_write_buffer("Timeout error when waiting for DHT22 response PULL LOW");
            return 1;
        }
    }

    SysTick->LOAD = TIMEOUT_90_US - 1; // Set maximum allowable wait time 85Âµs
	SysTick->VAL = 0;
	SysTick->CTRL = 5;
    while (GPIOA->IDR & GPIO_IDR_IDR_7)
    {
        if ((SysTick->CTRL) & 0x10000)
        {
    		USART2_write_buffer("Timeout error when waiting for DHT22 response GET READY");
            return 1;
        }
    }

    SysTick->CTRL = 0;

    return 0;
}
