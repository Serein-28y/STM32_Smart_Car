#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Car.h"
#include "Key.h"
#include "Servo.h"
uint8_t KeyNum;

int main(void)
{
	OLED_Init();
	Key_Init();
	Car_Init();
	Servo_Init();
	Serial_Init();
	OLED_ShowString(1,1,"LeftSpeed: 000");
	OLED_ShowString(2,1,"RightSpeed:000");
	OLED_ShowString(3,1,"Angle:000");


	while(1)
	{
		OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
		OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
		OLED_ShowSignedNum(3,7,Servo_GetAngle(),3);
	}



}

