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

typedef enum {
    MODBUS_ADDR_INVALID = 0,
    MODBUS_ADDR_VALID = 1,
	MODBUS_CRC_VALID = 2,
	MODBUS_CRC_INVALID = 3,
	MODBUS_SENSOR_READ_OK = 4,
	MODBUS_SENSOR_READ_FAIL = 5,
	MODBUS_FRAME_OK = 6,
	MODBUS_FRAME_FAIL = 7
} MODBUS_Status;


uint16_t CRC16(uint8_t *nData, uint16_t wLength);
void MODBUS_IRQHandler();
void MODBUS_ProcessFrame();
void MODBUS_ReadFrame(uint8_t *MODBUS_Frame);
void MODBUS_DiscardFrame();
MODBUS_Status MODBUS_CheckAddress(uint8_t address);
void MODBUS_ProcessValidFrame(uint8_t *MODBUS_Frame);
void MODBUS_ProcessInvalidFrame(void);
MODBUS_Status MODBUS_VerifyCRC(uint8_t *MODBUS_Frame);
uint8_t MODBUS_RingBufferRead(uint8_t *data);
void MODBUS_BuildFrame(uint8_t *MODBUS_Frame);
MODBUS_Status MODBUS_ReadSensor(uint8_t *MODBUS_Frame);

#endif /* PERIPHERALS_MODBUS_H_ */
