#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
uint8_t KeyNum;
uint16_t Distance;

int main(void)
{
	
	Car_Init();
	Servo_Init();
	Serial_Init();
	HC_SR04_Init();
	printf("HC_SR04_Start\r\n");


	while(1)
	{
		
		HC_SR04_Start();
		Delay_ms(200);
		if(HC_SR04_GetDistance(&Distance)==1)
		{
			if(Distance>=2&&Distance<=400)
			{
				printf("Distance:%dcm\r\n",Distance);
			}
			else
			{
				printf("Out of Range\r\n");
			}
		}
		else
		{
			printf("No Echo\r\n");
		}
	}



}
 
