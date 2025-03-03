/*
 * modbus.h
 *
 *  Created on: 17 Feb 2025
 *      Author: lauri
 */

#ifndef PERIPHERALS_MODBUS_H_
#define PERIPHERALS_MODBUS_H_

#include "stm32l1xx.h"
#include "dht22.h"
#include "lmt84lp.h"
#include "nsl19m51.h"
#include "usart.h"
#include <stdio.h>

#define SLAVE_COUNT 3
#define MODBUS_FRAME_SIZE 8

extern uint8_t mFlag;

unsigned short int CRC16(char *nData, unsigned short int wLength);
void MODBUS_IRQHandler();
void MODBUS_ProcessFrame(uint8_t *selected_slave);
void MODBUS_ReadFrame(uint8_t *MODBUS_Frame);
void MODBUS_DiscardFrame();
uint8_t* MODBUS_CheckAdress(uint8_t *c);

#endif /* PERIPHERALS_MODBUS_H_ */
