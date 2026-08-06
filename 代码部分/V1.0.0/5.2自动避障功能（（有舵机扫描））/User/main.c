#include "stm32f10x.h"                  // STM32F103设备头文件
#include "Delay.h"                      // 延时模块
#include "Serial.h"                     // 串口模块
#include "Car.h"                        // 小车电机控制
#include "Servo.h"                      // 舵机控制
#include "HC_SR04.h"                    // 超声波测距
#include "Avoid.h"                      // 自动避障算法

int main(void)
{
	/* 先初始化底层外设，再初始化避障状态 */
	Car_Init();                            // 初始化电机PWM和方向IO
	Servo_Init();                          // 初始化舵机PWM
	HC_SR04_Init();                        // 初始化HC-SR04触发和回波捕获
	Serial_Init();                         // 初始化串口，便于调试
	Avoid_Init();                          // 舵机回正，小车停车

	while(1)
	{
		/* 自动避障主循环 */
		Avoid_Run();
	}
}
