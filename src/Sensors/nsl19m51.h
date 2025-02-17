/*
 * nsl19m51.h
 *
 *  Created on: 27 Jan 2025
 *      Author: e2203130
 */

#ifndef SENSORS_NSL19M51_H_
#define SENSORS_NSL19M51_H_

#include "stm32l1xx.h"

typedef struct NSL19M51_Reading {
	uint8_t lux_int;
	uint8_t lux_dec;
} NSL19M51_Reading;

void NSL19M51_init();
void NSL19M51_read(NSL19M51_Reading *reading);

#endif /* SENSORS_NSL19M51_H_ */
