#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
uint8_t KeyNum;
uint16_t Distance;

int main(void)
{
	OLED_Init();
	Car_Init();
	Servo_Init();
	Serial_Init();
	HC_SR04_Init();
	OLED_ShowString(1,1,"LeftSpeed: 000");
	OLED_ShowString(2,1,"RightSpeed:000");
	OLED_ShowString(3,1,"Angle:000");
	OLED_ShowString(4,1,"Distance:000cm");


	while(1)
	{
		OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
		OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
		OLED_ShowSignedNum(3,7,Servo_GetAngle(),3);
		HC_SR04_Start();
		Delay_ms(200);
		if(HC_SR04_GetDistance(&Distance)==1)
		{
			OLED_ShowNum(4,10,Distance,3);
		}
	}



}

