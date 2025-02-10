/*
 * dht22.h
 *
 *  Created on: 2 Feb 2025
 *      Author: e2203130
 */

#ifndef SENSORS_DHT22_H_
#define SENSORS_DHT22_H_

#include "stm32l1xx.h"

void DHT22_init();
int DHT22_read(char *buffer, int buffer_size);
void DHT22_start();
int DHT22_wait_response();
int DHT22_read_response(uint8_t *data);
#endif /* SENSORS_DHT22_H_ */
