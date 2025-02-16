#include "dht22.h"
#include "timing.h"
#include "usart.h"
#include <stdio.h>

// PIN PB3
#define TIMEOUT_20_MS 640000
#define TIMEOUT_90_US 2880
#define TIMEOUT_50_US 1600
#define TIMEOUT_28_US 896

#define BIT_COUNT 41

#define DHT_MEASURING 3
#define DHT_NOT_READY 2
#define DHT_ERROR 1
#define DHT_READY 0

static volatile uint8_t pulses[BIT_COUNT];
static volatile uint8_t dht_status = 0;
static uint8_t skip_bits = 0;


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
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI7_PA;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI7_PA;

    EXTI->IMR |= EXTI_IMR_MR7;
    EXTI->FTSR |= EXTI_FTSR_FT7;
    EXTI->RTSR |= EXTI_RTSR_RT7;

    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

int DHT22_read(char *buffer, int buffer_size)
{
    uint8_t humidity_int, humidity_dec, temp_int, temp_dec, checksum;
    uint8_t current_byte = 0;
    uint8_t byte_list[5] = {0};

    DHT22_start();

    if (DHT22_wait_response())
    {
        USART2_write_buffer("DHT22 Not ready to send data!");
        return DHT_ERROR;
    }

    SysTick->LOAD = TIMEOUT_20_MS - 1; // Set maximum allowable wait time
	SysTick->VAL = 0;
	SysTick->CTRL = 5;

    // Wait for data
    while(dht_status == DHT_MEASURING)
    {
        if ((SysTick->CTRL) & 0x10000)
        {
    		USART2_write_buffer("DHT22 measurement error :/");
            return DHT_ERROR;
        }
    }

	SysTick->CTRL = 0;

    if (dht_status == DHT_READY)
    {
	    EXTI->IMR &= ~EXTI_IMR_MR7;
    	dht_status = DHT_NOT_READY;

        for (int bit = 1; bit < BIT_COUNT; bit++)
        {
        	uint8_t buffer[100];

            if (pulses[bit] > 20 && pulses[bit] < 32)
            {
            	current_byte = (current_byte << 1) | 0;
            	USART2_write('0');
            }

            else
            {
            	current_byte = (current_byte << 1) | 1;
            	USART2_write('1');
            }

        	snprintf(buffer, 50, " Pulse widht is : %d", pulses[bit]);
        	USART2_write_buffer(buffer);

            if ((bit % 8) == 7)
            {
                byte_list[(bit / 8)] = current_byte;
                current_byte = 0;
            }
        }

		byte_list[4] = current_byte;

		humidity_int = byte_list[0];
		humidity_dec = byte_list[1];
		temp_int = byte_list[2];
		temp_dec = byte_list[3];
		checksum = byte_list[4];

		uint8_t expected_checksum = humidity_int + humidity_dec + temp_int + temp_dec;
		if (checksum != (expected_checksum & 0xFF))
		{
			snprintf(buffer, buffer_size, "Checksum ERROR! %.2X and %.2X", expected_checksum, checksum);
			USART2_write_buffer(buffer);
			snprintf(buffer, buffer_size, "%.2X, %.2X and %.2X, %.2X and %.2X", humidity_int, humidity_dec, temp_int, temp_dec, checksum);
			USART2_write_buffer(buffer);
		}

		snprintf(buffer, buffer_size, "Humidity %d,%d and Temperature %d,%d\n", humidity_int, humidity_dec, temp_int, temp_dec);

		return DHT_READY;
    }

    return DHT_ERROR;
}

void DHT22_start()
{
    EXTI->IMR &= ~EXTI_IMR_MR7;
    // MCU PULL LOW ~20ms
    DHT22_SWITCH_MODE_OUTPUT();
    GPIOA->ODR &= ~GPIO_ODR_ODR_7;
    delay_ms(20);

    // MCU RELEASE LINE 20-40us
    GPIOA->ODR |= GPIO_ODR_ODR_7;
    DHT22_SWITCH_MODE_INPUT();
	delay_us(20);
}

int DHT22_wait_response()
{
    SysTick->LOAD = TIMEOUT_90_US - 1; // Set maximum allowable wait time
	SysTick->VAL = 0;
	SysTick->CTRL = 5;

    while (!(GPIOA->IDR & GPIO_IDR_IDR_7))
    {
        if ((SysTick->CTRL) & 0x10000)
        {
    		USART2_write_buffer("Timeout error when waiting for DHT22 response PULL LOW");
            return DHT_ERROR;
        }
    }

    EXTI->IMR |= EXTI_IMR_MR7;

    SysTick->LOAD = TIMEOUT_90_US - 1; // Set maximum allowable wait time 85µs
	SysTick->VAL = 0;
	SysTick->CTRL = 5;
    while (GPIOA->IDR & GPIO_IDR_IDR_7)
    {
        if ((SysTick->CTRL) & 0x10000)
        {
    		USART2_write_buffer("Timeout error when waiting for DHT22 response GET READY");
            return DHT_ERROR;
        }
    }

    SysTick->CTRL = 0;

    return 0;
}

void DHT22_IRQHandler()
{
	static uint8_t index = 0;
	static uint16_t last_time = 0;
	uint16_t now = TIM2->CNT;
	uint16_t pulse_width;
	//uint16_t pulse_width = now - last_time;

	dht_status = DHT_MEASURING;

	if (GPIOA->IDR & GPIO_IDR_IDR_7) // Rising edge
	{
		last_time = now;
		GPIOA->ODR |= GPIO_ODR_ODR_5;
	}

	else // Falling edge
	{
		pulse_width = (now >= last_time) ? (now - last_time) : (0xFFFF - last_time + now);
		pulses[index] = pulse_width;
		index++;
		GPIOA->ODR &= ~GPIO_ODR_ODR_5;
	}

	if (index >= BIT_COUNT)
	{
		index = 0;
		dht_status = DHT_READY;
	}

	EXTI->PR = EXTI_PR_PR7;
}






