#include "dht22.h"
#include "timing.h"

// PIN PA7

#define TIMEOUT_85_US 2720
#define TIMEOUT_30_US 960

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

    DHT22_SWITCH_MODE_INPUT();

    uint8_t current_byte = 0;
    uint8_t byte_list[5] = {0};

    for (uint8_t bit = 0; bit < 40; bit++)
    {
        if (!(bit & 7))
        {
            byte_list[(bit / 8)] = current_byte;
            current_byte = 0;
        }
        
        while (!(GPIOA->IDR & GPIO_IDR_IDR_7));

        SysTick->LOAD = TIMEOUT_30_US - 1; // Set time wait to 30µs
        SysTick->VAL = 0;
        SysTick->CTRL = 5;
        
        while (GPIOA->IDR & GPIO_IDR_IDR_7);
        
        if ((SysTick->CTRL) & 0x10000)
        {
            current_byte = (current_byte << 1) | 1; // If its longer than 30µs its a 1
        }

        else
        {
            current_byte = (current_byte << 1) | 0;
        }
    }
    byte_list[4] = current_byte;
    SysTick->CTRL = 0;
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
	SysTick->CTRL = 5;
    while (GPIOA->IDR & GPIO_IDR_IDR_7)
    {
        if ((SysTick->CTRL) & 0x10000)
        {
            return;
        }
    }

    SysTick->CTRL = 0;
}
