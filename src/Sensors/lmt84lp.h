/*
 * lmt84lp.h
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#ifndef SENSORS_LMT84LP_H_
#define SENSORS_LMT84LP_H_

#include "stm32l1xx.h"
#include "modbus.h"

#define LMT84LP_MODBUS_ADDRESS 0x01

void LMT84LP_init();
void LMT84LP_read(MODBUS_Reading *reading);
void LMT84LP_ModbusHander(MODBUS_Reading *reading);

#endif /* SENSORS_LMT84LP_H_ */
