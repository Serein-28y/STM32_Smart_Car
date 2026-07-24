#include "stm32f10x.h"
#include "Motor.h"

static int16_t g_LeftSpeed = 0;
static int16_t g_RightSpeed = 0;

void Car_Init(void)
{
	Motor_Init();
}

int16_t Car_GetLeftSpeed(void)
{
	return g_LeftSpeed;
}

int16_t Car_GetRightSpeed(void)
{
	return g_RightSpeed;
}

void Car_SetSpeed(int16_t leftSpeed, int16_t rightSpeed)
{
	g_LeftSpeed = leftSpeed;
	g_RightSpeed = rightSpeed;
	Motor_SetLeftSpeed(leftSpeed);
	Motor_SetRightSpeed(rightSpeed);
}

void Car_Stop(void)
{
	Car_SetSpeed(0, 0);
}

void Car_Forward(uint8_t speed)
{
	Car_SetSpeed(speed, speed);
}

void Car_Backward(uint8_t speed)
{
	Car_SetSpeed(-(int16_t)speed, -(int16_t)speed);
}

void Car_TurnLeft(uint8_t speed)
{
	Car_SetSpeed(speed / 2, speed);
}

void Car_TurnRight(uint8_t speed)
{
	Car_SetSpeed(speed, speed / 2);
}

void Car_SpinLeft(uint8_t speed)
{
	Car_SetSpeed(-(int16_t)speed, speed);
}

void Car_SpinRight(uint8_t speed)
{
	Car_SetSpeed(speed, -(int16_t)speed);
}

