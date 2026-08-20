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
	Motor_SetLeftSpeed(0);
	Motor_SetRightSpeed(0);
	OLED_ShowString(1,1,"Stop");
	
	while(1)
	{
		if(1==Key_GetNum())
		{
			Motor_SetLeftSpeed(70);
			Motor_SetRightSpeed(70);
			OLED_ShowString(1,1,"Go_head");
		}
		

	}
}

