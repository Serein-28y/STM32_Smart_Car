#include "stm32f10x.h"                  // Device header
#include "PWMServo.h"

static int8_t g_Angle;
void Servo_Init(void)
{
	PWMServo_Init();
}
	
void Servo_SetAngle(float Angle)
{
	g_Angle=Angle;
	PWMServo_SetCompare3(Angle/180*2000+500);


}

int8_t Servo_GetAngle(void)
{
	return g_Angle;

}

