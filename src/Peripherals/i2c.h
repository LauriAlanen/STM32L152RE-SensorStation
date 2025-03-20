/*
 * i2c.h
 *
 *  Created on: 20 Mar 2025
 *      Author: lauri
 */

#ifndef PERIPHERALS_I2C_H_
#define PERIPHERALS_I2C_H_

#include "stm32l1xx.h"

void I2C1_Init(void);
void I2C1_Write(uint8_t address, uint8_t command, int n, uint8_t* data);
void I2C1_ByteWrite(uint8_t address, uint8_t command);
void I2C1_Read(uint8_t address, uint8_t command, int n, uint8_t* data);

#endif /* PERIPHERALS_I2C_H_ */
