#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Car.h"
#include "Key.h"
uint8_t KeyNum;
int16_t Speed=0;
int main(void)
{
	OLED_Init();
	Key_Init();
	Car_Init();
	OLED_ShowString(1,1,"LeftSpeed: 000");
	OLED_ShowString(2,1,"RightSpeed:000");
	
	while(1)
	{
		
		//测试
		if(1==Key_GetNum())
		{
			
			Car_Forward(SPEED_MEDIUM);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_Backward(SPEED_MEDIUM);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_TurnLeft(SPEED_MEDIUM);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_TurnRight(SPEED_MEDIUM);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_SpinLeft(SPEED_MEDIUM);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_SpinRight(SPEED_MEDIUM);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_Stop();
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
			
			Car_SetSpeed(SPEED_MEDIUM,SPEED_MAX);
			OLED_ShowSignedNum(1,12,Car_GetLeftSpeed(),3);
			OLED_ShowSignedNum(2,12,Car_GetRightSpeed(),3);
			Delay_s(3);
		
		
		}
		
	}
}

