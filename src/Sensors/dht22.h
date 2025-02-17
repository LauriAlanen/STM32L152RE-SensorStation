/*
 * dht22.h
 *
 *  Created on: 2 Feb 2025
 *      Author: e2203130
 */

#ifndef SENSORS_DHT22_H_
#define SENSORS_DHT22_H_

#include "stm32l1xx.h"

#define DHT22_MODBUS_ADDRESS 0x6

typedef struct DHT22_Reading {
	uint8_t humidity_int;
	uint8_t humidity_dec;
	uint8_t temperature_int;
	uint8_t temperature_dec;
} DHT22_Reading;

void DHT22_init();
int DHT22_read(DHT22_Reading *reading);
uint8_t DHT22_read_byte();
uint8_t DHT22_read_bit();
void DHT22_start();
int DHT22_wait_response();
int DHT22_read_response(uint8_t *data);
void DHT22_IRQHandler();
void DHT22_decode_pulses(volatile uint8_t *pulses, uint8_t *byte_list);

#endif /* SENSORS_DHT22_H_ */
