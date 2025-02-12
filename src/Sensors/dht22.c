#include "dht22.h"
#include "timing.h"   // Must provide delay_us() and delay_ms()
#include "usart.h"
#include <stdio.h>

// Using PA7 for DHT22 data.
// You may need to adjust timeout values if your delay_us() isn’t super accurate.
#define TIMEOUT_US 100  // generic timeout for waiting state changes

char usart_buffer[100];

//---------------------------
// GPIO mode switching routines
//---------------------------
void DHT22_SWITCH_MODE_OUTPUT(void)
{
    // Clear the two bits for PA7, then set as output (01)
    GPIOA->MODER &= ~(3 << (7 * 2));
    GPIOA->MODER |= (1 << (7 * 2));
}

void DHT22_SWITCH_MODE_INPUT(void)
{
    // Clear the two bits for PA7 to set as input (00)
    GPIOA->MODER &= ~(3 << (7 * 2));
}

void DHT22_init(void)
{
    DHT22_SWITCH_MODE_OUTPUT();
    // Default high (if not driven otherwise)
    GPIOA->ODR |= (1 << 7);
}

//---------------------------
// Communication routines
//---------------------------

/*
 * DHT22_start()
 *
 * Pull the data line low for at least 18-20ms to send the start signal,
 * then release the line and switch to input mode. The sensor should then
 * respond with its low/high sequence.
 */
void DHT22_start(void)
{
    DHT22_SWITCH_MODE_OUTPUT();
    // Pull low for 20ms
    GPIOA->ODR &= ~(1 << 7);
    delay_ms(20);

    // Release line (pull high) then wait 20-40µs
    GPIOA->ODR |= (1 << 7);
    delay_us(30);

    // Switch to input mode to listen for the sensor’s response
    DHT22_SWITCH_MODE_INPUT();
}

/*
 * DHT22_wait_response()
 *
 * After releasing the line, the sensor will pull it low for ~80µs,
 * then high for ~80µs before starting the 40-bit data transmission.
 *
 * Here we wait for the following sequence:
 *  1. Wait for the sensor to pull the line low.
 *  2. Wait for the sensor to drive it high.
 *  3. Wait for the sensor to pull it low again (start of data bit).
 *
 * Returns 0 if the sequence is detected; 1 if a timeout occurred.
 */
int DHT22_wait_response(void)
{
    uint32_t count;

    // 1. Wait for sensor to pull the line low.
    count = 0;
    while (GPIOA->IDR & (1 << 7))  // expecting a transition from high to low
    {
        delay_us(1);
        if (++count > TIMEOUT_US)
        {
            USART2_write_buffer("Timeout: Sensor did not pull line low (start response)\n");
            return 1;
        }
    }

    // 2. Wait for sensor to drive the line high.
    count = 0;
    while (!(GPIOA->IDR & (1 << 7)))
    {
        delay_us(1);
        if (++count > TIMEOUT_US)
        {
            USART2_write_buffer("Timeout: Sensor did not pull line high (response low period too long?)\n");
            return 1;
        }
    }

    // 3. Wait for sensor to pull the line low again (this marks the end of the 80µs high period).
    count = 0;
    while (GPIOA->IDR & (1 << 7))
    {
        delay_us(1);
        if (++count > TIMEOUT_US)
        {
            USART2_write_buffer("Timeout: Sensor did not pull line low to start data\n");
            return 1;
        }
    }

    return 0;
}

/*
 * DHT22_read_bit()
 *
 * Each bit transmission starts with a 50µs low pulse. After that, the sensor
 * drives the line high for:
 *    ~26-28µs → bit value 0
 *    ~70µs    → bit value 1
 *
 * This routine waits for the line to go high (start of bit), then counts
 * microseconds until the line goes low.
 *
 * Returns:
 *    0 or 1 for a valid bit, or 0xFF if a timeout error occurred.
 */
uint8_t DHT22_read_bit(void)
{
    uint32_t count = 0;

    // Wait for the bit to start: sensor pulls the line high.
    while (!(GPIOA->IDR & (1 << 7)))
    {
        delay_us(1);
        if (++count > TIMEOUT_US)
        {
            USART2_write_buffer("Timeout: waiting for bit start (line high)\n");
            return 0xFF;  // error indicator
        }
    }

    // Now measure how long the line stays high.
    count = 0;
    while (GPIOA->IDR & (1 << 7))
    {
        delay_us(1);
        count++;
        if (count > 100)  // safety cap
            break;
    }

    // Choose threshold (here ~40µs): longer means a 1, shorter means a 0.
    return (count > 40) ? 1 : 0;
}

/*
 * DHT22_read_byte()
 *
 * Reads 8 bits to form one byte.
 */
uint8_t DHT22_read_byte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
    {
        uint8_t bit = DHT22_read_bit();
        if (bit == 0xFF)  // error indicator from DHT22_read_bit()
            return 0xFF;
        byte = (byte << 1) | bit;
    }
    return byte;
}

/*
 * DHT22_read()
 *
 * Initiates communication with the sensor, waits for the response, then reads
 * 5 bytes (humidity high, humidity low, temperature high, temperature low, checksum).
 * Checks the checksum and outputs a formatted string.
 *
 * Returns 0 on success or nonzero on error.
 */
int DHT22_read(char *buffer, int buffer_size)
{
    DHT22_start();
    if (DHT22_wait_response())
    {
        USART2_write_buffer("DHT22: Sensor did not respond properly\n");
        return 1;
    }

    uint8_t data[5];
    for (int i = 0; i < 5; i++)
    {
        data[i] = DHT22_read_byte();
        if (data[i] == 0xFF)
        {
            USART2_write_buffer("DHT22: Error reading byte\n");
            return 1;
        }
    }

    // Verify checksum: checksum = data[0] + data[1] + data[2] + data[3]
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4])
    {
        USART2_write_buffer("DHT22: Checksum invalid!\n");
        return 1;
    }

    // Format the result string.
    // (Typically, humidity is sent in data[0] and data[1], and temperature in data[2] and data[3].)
    snprintf(buffer, buffer_size, "Temperature: %u.%u, Humidity: %u.%u",
             data[2], data[3], data[0], data[1]);

    return 0;
}
