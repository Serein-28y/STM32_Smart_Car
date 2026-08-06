#ifndef __CAR_H__
#define __CAR_H__

#include "stm32f10x.h"

#define SPEED_LOW     30
#define SPEED_MEDIUM  75
#define SPEED_HIGH    90
#define SPEED_MAX     100

void Car_Init(void);
void Car_Forward(uint8_t speed);
void Car_Backward(uint8_t speed);
void Car_TurnLeft(uint8_t speed);
void Car_TurnRight(uint8_t speed);
void Car_SpinLeft(uint8_t speed);
void Car_SpinRight(uint8_t speed);
void Car_Stop(void);
void Car_SetSpeed(int16_t leftSpeed, int16_t rightSpeed);
int16_t Car_GetLeftSpeed(void);
int16_t Car_GetRightSpeed(void);

#endif
