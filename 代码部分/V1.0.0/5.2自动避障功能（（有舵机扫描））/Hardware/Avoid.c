#include "stm32f10x.h"                  // STM32F103设备头文件
#include "Delay.h"                      // 延时模块
#include "Car.h"                        // 小车运动控制
#include "Servo.h"                      // 舵机角度控制
#include "HC_SR04.h"                    // HC-SR04超声波测距
#include "Avoid.h"                      // 自动避障参数和接口

uint16_t Avoid_Distance;                 // 当前正前方距离，单位：cm
static uint8_t Avoid_LastTurn = AVOID_TURN_NONE; // 保存上一次转向方向

/**
  * @brief  在当前舵机角度连续测量多次距离
  * @return 有效测量结果的平均值；没有有效回波时返回0
  */
static uint16_t Avoid_MeasureAverage(void)
{
	uint32_t sum = 0;                     // 有效距离之和
	uint8_t validCount = 0;               // 有效测量次数
	uint8_t i;
	uint16_t distance;

	for(i = 0; i < AVOID_MEASURE_COUNT; i++)
	{
		HC_SR04_Start();                    // 发送一次超声波触发脉冲
		Delay_ms(AVOID_MEASURE_TIME);       // 等待回波捕获完成

		/* 无回波、0 cm或超过HC-SR04量程的数据不参与平均 */
		if(HC_SR04_GetDistance(&distance) == 1 && distance > 0 && distance < 400)
		{
			sum += distance;
			validCount++;
		}
	}

	/* 返回0时，上层算法会将当前方向按不可通行处理 */
	return validCount ? (uint16_t)(sum / validCount) : 0;
}

/**
  * @brief  按左、右顺序扫描，并保存两个方向的距离
  * @param  leftDistance  左侧距离保存地址
  * @param  rightDistance 右侧距离保存地址
  */
static void Avoid_ScanSides(uint16_t *leftDistance, uint16_t *rightDistance)
{
	Servo_SetAngle(AVOID_SERVO_LEFT_ANGLE); // 舵机转向左侧
	Delay_ms(AVOID_SERVO_WAIT_TIME);        // 等待舵机稳定
	*leftDistance = Avoid_MeasureAverage(); // 多次测量左侧距离

	Servo_SetAngle(AVOID_SERVO_RIGHT_ANGLE);// 舵机转向右侧
	Delay_ms(AVOID_SERVO_WAIT_TIME);        // 等待舵机稳定
	*rightDistance = Avoid_MeasureAverage();// 多次测量右侧距离
}

/**
  * @brief  根据左右距离和历史方向选择转向方向
  * @return AVOID_TURN_LEFT、AVOID_TURN_RIGHT或AVOID_TURN_NONE
  */
static uint8_t Avoid_SelectTurn(uint16_t leftDistance, uint16_t rightDistance)
{
	/* 只有左侧满足通行距离，选择左转 */
	if(leftDistance > AVOID_SIDE_DISTANCE && rightDistance <= AVOID_SIDE_DISTANCE)
	{
		return AVOID_TURN_LEFT;
	}

	/* 只有右侧满足通行距离，选择右转 */
	if(rightDistance > AVOID_SIDE_DISTANCE && leftDistance <= AVOID_SIDE_DISTANCE)
	{
		return AVOID_TURN_RIGHT;
	}

	/* 左右都能通行时，优先选择距离更大的一侧 */
	if(leftDistance > AVOID_SIDE_DISTANCE && rightDistance > AVOID_SIDE_DISTANCE)
	{
		if(leftDistance > rightDistance)
		{
			return AVOID_TURN_LEFT;
		}
		if(rightDistance > leftDistance)
		{
			return AVOID_TURN_RIGHT;
		}

		/* 左右距离相等时，选择上一次转向的反方向 */
		return (Avoid_LastTurn == AVOID_TURN_LEFT) ? AVOID_TURN_RIGHT : AVOID_TURN_LEFT;
	}

	/* 左右都不满足通行距离，判定为死路 */
	return AVOID_TURN_NONE;
}

/**
  * @brief  按固定时间执行原地转向，并记录本次转向方向
  */
static void Avoid_Turn(uint8_t direction)
{
	if(direction == AVOID_TURN_LEFT)
	{
		Car_SpinLeft(AVOID_SPEED);          // 小车原地左转
		Avoid_LastTurn = AVOID_TURN_LEFT;  // 保存左转历史
	}
	else
	{
		Car_SpinRight(AVOID_SPEED);         // 小车原地右转
		Avoid_LastTurn = AVOID_TURN_RIGHT; // 保存右转历史
	}

	Delay_ms(AVOID_TURN_TIME);             // 固定时间转向
	Car_Stop();                            // 转向结束后停车
}

/**
  * @brief  初始化自动避障状态
  */
void Avoid_Init(void)
{
	Servo_SetAngle(AVOID_SERVO_ANGLE);     // 舵机回到正前方
	Avoid_LastTurn = AVOID_TURN_NONE;      // 清除转向历史
	Car_Stop();                            // 小车保持停止
}

/**
  * @brief  执行一次自动避障判断和动作
  */
void Avoid_Run(void)
{
	uint16_t leftDistance;                 // 左侧扫描距离
	uint16_t rightDistance;                // 右侧扫描距离
	uint8_t turnDirection;                 // 本次转向决策

	/* 第一步：舵机保持正前方，进行中间方向测距 */
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	Avoid_Distance = Avoid_MeasureAverage();

	/* 正前方距离足够，小车继续向前巡航 */
	if(Avoid_Distance > AVOID_DISTANCE)
	{
		Car_Forward(AVOID_SPEED);
		return;
	}

	/* 第二步：前方有障碍，停车后扫描左侧和右侧 */
	Car_Stop();
	Delay_ms(AVOID_STOP_TIME);
	Avoid_ScanSides(&leftDistance, &rightDistance);

	/* 第三步：左右扫描完成，舵机回正并重新测量中间 */
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	Delay_ms(AVOID_SERVO_WAIT_TIME);
	Avoid_Distance = Avoid_MeasureAverage();

	/* 第四步：舵机回正、测距结束后，小车后退固定时间 */
	Car_Backward(AVOID_SPEED);
	Delay_ms(AVOID_BACK_TIME);
	Car_Stop();

	/* 第五步：比较左右距离并参考历史方向作出选择 */
	turnDirection = Avoid_SelectTurn(leftDistance, rightDistance);

	/* 第六步：左右均不可通行时，额外后退并重新扫描 */
	if(turnDirection == AVOID_TURN_NONE)
	{
		Car_Backward(AVOID_SPEED);
		Delay_ms(AVOID_DEAD_BACK_TIME);
		Car_Stop();

		Avoid_ScanSides(&leftDistance, &rightDistance); // 后退后复测左右
		Servo_SetAngle(AVOID_SERVO_ANGLE);              // 复测完成后回正
		Delay_ms(AVOID_SERVO_WAIT_TIME);
		turnDirection = Avoid_SelectTurn(leftDistance, rightDistance);
	}

	/* 第七步：复测仍无通道时，向上一次转向的反方向掉头 */
	if(turnDirection == AVOID_TURN_NONE)
	{
		Avoid_Turn((Avoid_LastTurn == AVOID_TURN_LEFT) ?
			AVOID_TURN_RIGHT : AVOID_TURN_LEFT);
		return;
	}

	/* 存在可通行方向，按固定时间执行原地转向 */
	Avoid_Turn(turnDirection);
}
