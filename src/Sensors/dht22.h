/*
 * dht22.h
 *
 *  Created on: 2 Feb 2025
 *      Author: e2203130
 */

#ifndef SENSORS_DHT22_H_
#define SENSORS_DHT22_H_

#include "stm32l1xx.h"
#include "modbus.h"
#include "timing.h"
#include "usart.h"
#include <stdio.h>

#define TIMEOUT_20_MS 640000
#define TIMEOUT_90_US 2880
#define TIMEOUT_50_US 1600
#define TIMEOUT_28_US 896

#define DHT_MEASURING 3
#define DHT_NOT_READY 2
#define DHT_ERROR 1
#define DHT_READY 0

#define BIT_COUNT 41
#define DHT22_MODBUS_ADDRESS 0x6

uint8_t DHT22_read_byte();
uint8_t DHT22_read_bit();
uint8_t DHT22_wait_response();
uint8_t DHT22_read(MODBUS_Reading* reading);
uint8_t DHT22_read_response(uint8_t *data);
void DHT22_init();
void DHT22_start();
void DHT22_IRQHandler();
void DHT22_decode_pulses(volatile uint8_t *pulses, uint8_t *byte_list);
void DHT22_ModbusHandler(MODBUS_Reading* reading);

#endif /* SENSORS_DHT22_H_ */
