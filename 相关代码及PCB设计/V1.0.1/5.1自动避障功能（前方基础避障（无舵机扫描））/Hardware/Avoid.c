#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"

uint16_t Avoid_Distance;

void Avoid_Init(void)
{
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	Car_Stop();
}

void Avoid_Run(void)
{
	HC_SR04_Start();
	Delay_ms(AVOID_MEASURE_TIME);
	
	if(HC_SR04_GetDistance(&Avoid_Distance)==1)
	{
		if(Avoid_Distance>AVOID_DISTANCE)
		{
			Car_Forward(SPEED_MEDIUM);
		}
		else
		{
			Car_Stop();
			Delay_ms(AVOID_STOP_TIME);
			
			Car_Backward(SPEED_MEDIUM);
			Delay_ms(AVOID_BACK_TIME);
			
			Car_SpinLeft(SPEED_MEDIUM);
			Delay_ms(AVOID_TURN_TIME);
			
			Car_Stop();
		}
	}
	else
	{
		Car_Stop();
	}
}
