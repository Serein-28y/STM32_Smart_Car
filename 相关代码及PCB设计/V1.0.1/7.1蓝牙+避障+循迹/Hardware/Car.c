#include "stm32f10x.h"
#include "Car.h"
#include "Motor.h"

static int16_t g_LeftSpeed = 0;
static int16_t g_RightSpeed = 0;
/* 上电默认锁止，只有明确进入运行模式后才能恢复电机输出。 */
static volatile uint8_t g_DriveLocked = 1;

/**
  * @brief  初始化电机驱动并使小车保持紧急锁止状态。
  * @param  无
  * @retval 无
  */
void Car_Init(void)
{
	Motor_Init();
	Car_EmergencyStop();
}

/**
  * @brief  获取左侧电机当前设定速度。
  * @param  无
  * @retval 左侧速度，正数前进、负数后退、0停止。
  */
int16_t Car_GetLeftSpeed(void)
{
	return g_LeftSpeed;
}

/**
  * @brief  获取右侧电机当前设定速度。
  * @param  无
  * @retval 右侧速度，正数前进、负数后退、0停止。
  */
int16_t Car_GetRightSpeed(void)
{
	return g_RightSpeed;
}

/**
  * @brief  设置左右电机速度，并统一执行驱动锁止保护。
  * @param  leftSpeed 左侧速度，建议范围-100~100。
  * @param  rightSpeed 右侧速度，建议范围-100~100。
  * @retval 无
  */
void Car_SetSpeed(int16_t leftSpeed, int16_t rightSpeed)
{
	/* 所有运动函数最终都经过此处，锁止后任何旧流程都不能重新启动电机。 */
	if(g_DriveLocked != 0)
	{
		g_LeftSpeed = 0;
		g_RightSpeed = 0;
		Motor_SetLeftSpeed(0);
		Motor_SetRightSpeed(0);
		return;
	}

	g_LeftSpeed = leftSpeed;
	g_RightSpeed = rightSpeed;
	Motor_SetLeftSpeed(leftSpeed);
	Motor_SetRightSpeed(rightSpeed);
}

/**
  * @brief  将左右电机速度设为0，但不改变驱动锁止状态。
  * @param  无
  * @retval 无
  */
void Car_Stop(void)
{
	Car_SetSpeed(0, 0);
}

/**
  * @brief  控制小车向前行驶，并应用左右轮直行补偿。
  * @param  speed 基础速度，范围0~100。
  * @retval 无
  */
void Car_Forward(uint8_t speed)
{
	int16_t leftSpeed = (int16_t)speed + CAR_FORWARD_LEFT_TRIM;
	int16_t rightSpeed = (int16_t)speed + CAR_FORWARD_RIGHT_TRIM;

	/* 限幅，避免补偿后超过有效 PWM 范围。 */
	if(leftSpeed < 0) leftSpeed = 0;
	if(leftSpeed > SPEED_MAX) leftSpeed = SPEED_MAX;
	if(rightSpeed < 0) rightSpeed = 0;
	if(rightSpeed > SPEED_MAX) rightSpeed = SPEED_MAX;

	Car_SetSpeed(leftSpeed, rightSpeed);
}

/**
  * @brief  立即清零电机输出并锁止后续所有运动命令。
  * @param  无
  * @retval 无
  */
void Car_EmergencyStop(void)
{
	g_DriveLocked = 1;
	g_LeftSpeed = 0;
	g_RightSpeed = 0;
	Motor_SetLeftSpeed(0);
	Motor_SetRightSpeed(0);
}

/**
  * @brief  解除驱动锁止，允许当前运行模式控制电机。
  * @param  无
  * @retval 无
  */
void Car_DriveEnable(void)
{
	g_DriveLocked = 0;
}

/**
  * @brief  查询电机驱动是否处于锁止状态。
  * @param  无
  * @retval 1表示锁止，0表示允许运动。
  */
uint8_t Car_IsDriveLocked(void)
{
	return g_DriveLocked;
}

/**
  * @brief  控制小车直线后退。
  * @param  speed 后退速度，范围0~100。
  * @retval 无
  */
void Car_Backward(uint8_t speed)
{
	Car_SetSpeed(-(int16_t)speed, -(int16_t)speed);
}

/**
  * @brief  通过降低左轮速度使小车向左转弯。
  * @param  speed 外侧轮速度，范围0~100。
  * @retval 无
  */
void Car_TurnLeft(uint8_t speed)
{
	Car_SetSpeed(speed / 2, speed);
}

/**
  * @brief  通过降低右轮速度使小车向右转弯。
  * @param  speed 外侧轮速度，范围0~100。
  * @retval 无
  */
void Car_TurnRight(uint8_t speed)
{
	Car_SetSpeed(speed, speed / 2);
}

/**
  * @brief  控制左右轮反向运动，使小车原地向左旋转。
  * @param  speed 旋转速度，范围0~100。
  * @retval 无
  */
void Car_SpinLeft(uint8_t speed)
{
	Car_SetSpeed(-(int16_t)speed, speed);
}

/**
  * @brief  控制左右轮反向运动，使小车原地向右旋转。
  * @param  speed 旋转速度，范围0~100。
  * @retval 无
  */
void Car_SpinRight(uint8_t speed)
{
	Car_SetSpeed(speed, -(int16_t)speed);
}

