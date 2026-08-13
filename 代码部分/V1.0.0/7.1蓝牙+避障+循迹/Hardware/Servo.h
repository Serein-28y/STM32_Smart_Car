#ifndef __SERVO_H__
#define	__SERVO_H__

#include "stm32f10x.h"

void Servo_Init(void);
void Servo_SetAngle(float Angle);
uint8_t Servo_GetAngle(void);
#endif

