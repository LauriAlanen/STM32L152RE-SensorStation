/*
 * modbus.h
 *
 *  Created on: 17 Feb 2025
 *      Author: lauri
 */

#ifndef PERIPHERALS_MODBUS_H_
#define PERIPHERALS_MODBUS_H_

#include "stm32l1xx.h"

extern uint8_t mFlag = 0;

unsigned short int CRC16(char *nData, unsigned short int wLength);

void MODBUS_IRQHandler();

#endif /* PERIPHERALS_MODBUS_H_ */
