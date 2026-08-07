#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"
int main()
{
	Car_Init();
	Serial_Init();
	Servo_Init();
	Servo_SetAngle(90);
	HC_SR04_Init();
	Avoid_Init();
	
	

	while(1)
	{
		Avoid_Run();
	
	
	}





}
	
