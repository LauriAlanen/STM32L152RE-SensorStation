/*
 * dht22.h
 *
 *  Created on: 2 Feb 2025
 *      Author: e2203130
 */

#ifndef SENSORS_DHT22_H_
#define SENSORS_DHT22_H_

void DHT22_init();
void DHT22_read(char *buffer, int buffer_size);

#endif /* SENSORS_DHT22_H_ */
