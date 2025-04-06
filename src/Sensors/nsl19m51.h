/*
 * nsl19m51.h
 *
 *  Created on: 27 Jan 2025
 *      Author: e2203130
 */

#ifndef SENSORS_NSL19M51_H_
#define SENSORS_NSL19M51_H_

#include "stm32l1xx.h"
#include "modbus.h"
#include <stdio.h>

#define NSL19M51_MODBUS_ADDRESS 0x4

void NSL19M51_init();
void NSL19M51_read(MODBUS_Reading *reading);
void NSL19M51_ModbusHandler(MODBUS_Reading *reading);

#endif /* SENSORS_NSL19M51_H_ */
