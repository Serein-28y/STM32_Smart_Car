#ifndef __SERIAL_H__
#define	__SERIAL_H__

#include "stdio.h"

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t* Array,uint16_t Length);
void Serial_SendString(char* String);
uint32_t Pow(uint32_t x,uint32_t Y);
void Serial_SendNum(uint32_t Number,uint8_t Length);
uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);
uint8_t Serial_ReadByte(uint8_t *data);
void Serial_ClearRxBuffer(void);
uint8_t Serial_IsExitRequested(void);
void Serial_ClearExitRequest(void);
#endif

