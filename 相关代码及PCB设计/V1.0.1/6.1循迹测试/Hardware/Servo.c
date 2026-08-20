#include "stm32f10x.h"                  // Device header
#include "PWMServo.h"

/* 舵机角度范围为0~180，必须使用无符号8位或更大类型保存 */
static uint8_t g_Angle;
void Servo_Init(void)
{
	PWMServo_Init();
}
	
void Servo_SetAngle(float Angle)
{
	/* 限制输入范围，防止调参或串口输入异常角度 */
	if(Angle < 0)
	{
		Angle = 0;
	}
	if(Angle > 180)
	{
		Angle = 180;
	}

	/* 使用uint8_t保存0~180度，避免int8_t在145度以上溢出 */
	g_Angle=(uint8_t)Angle;
	PWMServo_SetCompare3(Angle/180*2000+500);


}

uint8_t Servo_GetAngle(void)
{
	return g_Angle;

}

