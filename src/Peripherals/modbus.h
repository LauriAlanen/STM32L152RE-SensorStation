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
#define RX_BUFFER_SIZE 128

extern uint8_t mFlag;

uint16_t CRC16(uint8_t *nData, uint16_t wLength);
void MODBUS_IRQHandler();
void MODBUS_ProcessFrame();
void MODBUS_ReadFrame(uint8_t *MODBUS_Frame);
void MODBUS_DiscardFrame();
uint8_t MODBUS_CheckAdress(uint8_t c);
uint8_t MODBUS_VerifyCRC(uint8_t *MODBUS_Frame);
uint8_t MODBUS_RingBufferRead(uint8_t *data);
void MODBUS_BuildFrame(uint8_t *MODBUS_Frame);

#endif /* PERIPHERALS_MODBUS_H_ */
