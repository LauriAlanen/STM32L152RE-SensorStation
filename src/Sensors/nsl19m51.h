/*
 * nsl19m51.h
 *
 *  Created on: 27 Jan 2025
 *      Author: e2203130
 */

#ifndef SENSORS_NSL19M51_H_
#define SENSORS_NSL19M51_H_

#include "stm32l1xx.h"

void NSL19M51_init();
void NSL19M51_read(char *buffer, int buffer_size);

#endif /* SENSORS_NSL19M51_H_ */
