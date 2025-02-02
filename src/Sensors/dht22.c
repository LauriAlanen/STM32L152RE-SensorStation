#include "dht22.h"
#include "stm32l1xx.h"
#include "timing.h"

// PIN PA7

#define TIMEOUT_85_US 2720

#define DHT22_SWITCH_MODE_OUTPUT()  \
    do { \
        GPIOA->MODER &= ~GPIO_MODER_MODER7; \
        GPIOA->MODER |= GPIO_MODER_MODER7_0; \
    } while(0)

#define DHT22_SWITCH_MODE_INPUT() \
    do { \
        GPIOA->MODER &= ~GPIO_MODER_MODER7; \
    } while(0)


void DHT22_init()
{
	GPIOA->MODER &= GPIO_MODER_MODER7;
    DHT22_SWITCH_MODE_OUTPUT();
}

void DHT22_read(char *buffer, int buffer_size)
{
    DHT22_start();
    DHT22_wait_response();
}

void DHT22_start()
{
    DHT22_SWITCH_MODE_OUTPUT();
    GPIOA->ODR &= ~GPIO_ODR_ODR_7; 
    delay_ms(20);
    GPIOA->ODR |= GPIO_ODR_ODR_7; 
    delay_us(40);
}

void DHT22_wait_response()
{
    unsigned long timer;
    DHT22_SWITCH_MODE_INPUT();

    SysTick->LOAD = TIMEOUT_85_US - 1; // Set maximum allowable wait time 85µs
	SysTick->VAL = 0;
	SysTick->CTRL = 5;

    while (!(GPIOA->IDR & GPIO_IDR_IDR_7))
    {
        if ((SysTick->CTRL) & 0x10000)
        {
            return;
        }
    }

    SysTick->LOAD = TIMEOUT_85_US - 1; // Set maximum allowable wait time 85µs
	SysTick->VAL = 0;
    while (GPIOA->IDR & GPIO_IDR_IDR_7)
    {
        if ((SysTick->CTRL) & 0x10000)
        {
            return;
        }
    }
}