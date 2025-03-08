/*
 * modbus.c
 *
 *  Created on: 17 Feb 2025
 *      Author: lauri
 */

#include "modbus.h"

#define DEBUG 1

uint8_t mFlag = 0;
uint8_t frame_ready = 0;

// Ring buffer
volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t buffer_OVF = 0;
volatile uint16_t rx_head = 0, rx_tail = 0;

// Must be in the order of addresses from lower to higher
uint8_t MODBUS_Slaves[SLAVE_COUNT] = {LMT84LP_MODBUS_ADDRESS, NSL19M51_MODBUS_ADDRESS, DHT22_MODBUS_ADDRESS};
volatile uint8_t selected_slave = 0;

//parameter wLenght = how my bytes in your frame?
//*nData = your first element in frame array
uint16_t CRC16(uint8_t *nData, uint16_t wLength)
{
	static const uint16_t wCRCTable[] = {
		0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
		0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
		0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
		0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
		0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
		0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
		0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
		0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
		0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
		0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
		0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
		0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
		0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
		0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
		0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
		0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
		0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
		0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
		0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
		0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
		0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
		0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
		0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
		0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
		0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
		0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
		0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
		0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
		0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
		0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
		0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
		0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
	};

	uint8_t nTemp;
	uint16_t wCRCWord = 0xFFFF;

	while (wLength--)
	{
	  nTemp = *nData++ ^ wCRCWord;
	  wCRCWord >>= 8;
	  wCRCWord ^= wCRCTable[nTemp];
	}

	return wCRCWord;
}

MODBUS_Status MODBUS_VerifyCRC(uint8_t *MODBUS_Frame)
{
	uint16_t MODBUS_FrameCRC = 0;
	uint8_t buffer[100];

	MODBUS_FrameCRC = CRC16(MODBUS_Frame, MODBUS_FRAME_SIZE - 2); // Exclude the CRC itself

	uint8_t CRC_lsb = (MODBUS_FrameCRC >> 8) == MODBUS_Frame[MODBUS_FRAME_SIZE - 1];
	uint8_t CRC_msb = (MODBUS_FrameCRC & 0x00FF) == MODBUS_Frame[MODBUS_FRAME_SIZE - 2];
	if (CRC_lsb && CRC_msb)
	{
		return MODBUS_CRC_VALID;
	}

	return MODBUS_CRC_INVALID;
}

MODBUS_Status MODBUS_CheckAddress(uint8_t address)
{
    for (int i = 0; i < SLAVE_COUNT; ++i)
    {
        if (MODBUS_Slaves[i] == address)
        {
            GPIOA->ODR |= GPIO_ODR_ODR_5;
            return MODBUS_ADDR_VALID;
        }
    }

    GPIOA->ODR &= ~GPIO_ODR_ODR_5;
    return MODBUS_ADDR_INVALID;
}

void MODBUS_BuildFrame(uint8_t *MODBUS_Frame)
{
	static uint8_t frame_index = 0;
	uint8_t byte;

    while (MODBUS_RingBufferRead(&byte) == 0)
    {
    	MODBUS_Frame[frame_index++] = byte;
    	if (frame_index >= MODBUS_FRAME_SIZE)
    	{
    		frame_ready = 1;
    		frame_index = 0;
		}
    }
}

MODBUS_Status MODBUS_ReadSensor(uint8_t *MODBUS_Frame)
{

}

void MODBUS_ProcessFrame(void)
{
	static uint8_t MODBUS_Frame[MODBUS_FRAME_SIZE];
    MODBUS_BuildFrame(MODBUS_Frame);

    if (!frame_ready)
    {
        return;
    }

    MODBUS_Status status = MODBUS_CheckAddress(MODBUS_Frame[0]);

    if (status == MODBUS_ADDR_VALID)
    {
        MODBUS_ProcessValidFrame(MODBUS_Frame);
    }

    else
    {
        MODBUS_ProcessInvalidFrame();
    }

    frame_ready = 0;
}

void MODBUS_ProcessValidFrame(uint8_t *MODBUS_Frame)
{
	if (MODBUS_VerifyCRC(MODBUS_Frame) == MODBUS_CRC_INVALID)
	{
#ifdef DEBUG
	    char debugBuffer[100];
		snprintf(debugBuffer, 20, "%s", "Checksum error!");
		USART2_write_buffer(debugBuffer);
#endif
		return;
	}

    MODBUS_ReadSensor(MODBUS_Frame);

#if DEBUG == 1
    char debugBuffer[100];
    snprintf(debugBuffer, sizeof(debugBuffer), "Generated frame:");
    USART2_write_buffer(debugBuffer);

    for (int i = 0; i < MODBUS_FRAME_SIZE; ++i)
    {
        snprintf(debugBuffer, sizeof(debugBuffer), "%.2x ", MODBUS_Frame[i]);
        USART2_write_buffer(debugBuffer);
    }
#endif
}

void MODBUS_ProcessInvalidFrame(void)
{
#if DEBUG == 1
    char debugBuffer[100];
    snprintf(debugBuffer, sizeof(debugBuffer), "Invalid address!");
    USART2_write_buffer(debugBuffer);
#endif
}

uint8_t MODBUS_RingBufferRead(uint8_t *data)
{
    if (rx_tail == rx_head)
    {
        return -1;
    }

    *data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    return 0;
}

void MODBUS_IRQHandler()
{
    if (USART2->SR & USART_SR_RXNE)
    {
        uint8_t data = USART2->DR;
        uint16_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;

        if (next_head != rx_tail)
        {
            rx_buffer[rx_head] = data;
            rx_head = next_head;
        }

        else
        {
        	buffer_OVF = 1;
        }
    }
}
