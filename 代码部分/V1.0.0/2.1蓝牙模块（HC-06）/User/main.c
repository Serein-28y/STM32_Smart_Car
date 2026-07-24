#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "Car.h"
#include "Key.h"
uint8_t KeyNum;

int main(void)
{
	OLED_Init();
	Key_Init();
	Car_Init();
	Serial_Init();
	

	while(1)
	{
		
	}



}

