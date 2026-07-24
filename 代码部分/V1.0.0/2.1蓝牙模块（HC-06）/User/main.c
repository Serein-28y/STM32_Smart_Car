#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Car.h"
#include "Key.h"
uint8_t KeyNum;
uint8_t RxData;
int main(void)
{
	OLED_Init();
	Car_Init();
	Serial_Init();
	Key_Init();
	
	

	while(1)
	{
		if(1==Serial_GetRxFlag())
		{
			RxData=Serial_GetRxData();
			switch(RxData)  
			{
				case 'F': Car_Forward(SPEED_MEDIUM); break;
				case 'B': Car_Backward(SPEED_MEDIUM); break;
				case 'L': Car_TurnLeft(SPEED_MEDIUM); break;
				case 'R': Car_TurnRight(SPEED_MEDIUM); break;
				case 'A': Car_SpinLeft(SPEED_MEDIUM); break;
				case 'D': Car_SpinRight(SPEED_MEDIUM); break;
				case 'S': Car_Stop(); break;
				case '1': Car_SetSpeed(SPEED_MEDIUM, SPEED_MEDIUM); break;
				default: Car_Stop(); break;
			}
			
		
		}
		
	}



