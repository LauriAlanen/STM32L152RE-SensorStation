#include "dht22.h"

#define DEBUG 1

static volatile uint8_t pulses[BIT_COUNT];
static volatile uint8_t dht_status = 0;

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

uint8_t DHT22_read(MODBUS_Reading *reading)
{
    uint8_t byte_list[5] = {0};
    uint8_t buffer[100];

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

    	DHT22_decode_pulses(pulses, byte_list);

		uint8_t humidity_int = byte_list[0];
		uint8_t humidity_dec = byte_list[1];
		uint8_t temperature_int = byte_list[2];
		uint8_t temperature_dec = byte_list[3];
		uint8_t checksum = byte_list[4];

		uint8_t expected_checksum = humidity_int + humidity_dec + temperature_int + temperature_dec;
		if (expected_checksum != checksum)
		{
			snprintf(buffer, 100, "DHT22: Invalid checksum expected %.2X got %.2X", expected_checksum, checksum);
			USART2_write_buffer(buffer);
		}

		reading->raw_reading[0] = humidity_int;
		reading->raw_reading[1] = humidity_dec;
		reading->raw_reading[2] = temperature_int;
		reading->raw_reading[3] = temperature_dec;

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

uint8_t DHT22_wait_response()
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

void DHT22_decode_pulses(volatile uint8_t *pulses, uint8_t *byte_list)
{
    uint8_t current_byte = 0;

    for (int bit = 1; bit <= BIT_COUNT; bit++)
    {
        if (pulses[bit] > 20 && pulses[bit] < 32)
        {
            current_byte = (current_byte << 1) | 0;
        }
        else
        {
            current_byte = (current_byte << 1) | 1;
        }

        if ((bit % 8) == 0)
        {
            byte_list[(bit / 8) - 1] = current_byte;
            current_byte = 0;
        }
    }
}

void DHT22_ModbusHandler(MODBUS_Reading* reading)
{
	DHT22_read(reading);
}

void DHT22_IRQHandler()
{
	static uint8_t index = 0;
	static uint16_t last_time = 0;
	uint16_t now = TIM2->CNT;
	uint16_t pulse_width;

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






