#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"

int main(void)
{
	
	Car_Init();
	Servo_Init();
	Serial_Init();
	Avoid_Init();
	HC_SR04_Init();
	
	while(1)
	{
		Avoid_Run();
		
	}



}

