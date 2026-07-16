#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Motor.h"
#include "Key.h"
uint8_t KeyNum;
int16_t Speed=0;
int main(void)
{
	OLED_Init();
	Key_Init();
	Motor_Init();
	OLED_ShowString(1,1,"Speed:");

	while(1)
	{
		KeyNum=Key_GetNum();
		if(1==KeyNum)
		{
			Speed+=20;
			if(Speed>=100)
			{
				Speed=-100;
			}
			
		}
		Motor_SetSpeed(Speed);
		OLED_ShowSignedNum(1,7,Speed,3);
	}
}

