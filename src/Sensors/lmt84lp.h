/*
 * lmt84lp.h
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#ifndef SENSORS_LMT84LP_H_
#define SENSORS_LMT84LP_H_

#include "stm32l1xx.h"

typedef struct LMT84LP_Reading {
	uint8_t temperature_int;
	uint8_t temperature_dec;
} LMT84LP_Reading;

void LMT84LP_init();
void LMT84LP_read(LMT84LP_Reading *reading);

#endif /* SENSORS_LMT84LP_H_ */
