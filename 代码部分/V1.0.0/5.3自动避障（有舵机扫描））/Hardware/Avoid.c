#include "stm32f10x.h"                  // STM32F103设备头文件
#include "Delay.h"                      // 延时模块
#include "Car.h"                        // 小车运动控制
#include "Servo.h"                      // 舵机角度控制
#include "HC_SR04.h"                    // HC-SR04超声波测距
#include "Avoid.h"                      // 自动避障参数和接口

uint16_t Avoid_Distance;                 // 当前正前方距离，单位：cm
uint16_t Avoid_ScanDistances[AVOID_SCAN_ANGLE_COUNT]; // 多角度扫描结果缓存
static uint8_t Avoid_LastTurn = AVOID_TURN_NONE; // 保存上一次转向方向

/**
  * @brief  在当前舵机角度连续测量多次距离
  * @return 有效测量结果的平均值；没有有效回波时返回0
  */
static uint8_t Avoid_MeasureAverage(uint16_t *result)
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
	if(validCount == 0)
	{
		*result = 0;
		return 0;                       // 无有效回波
	}

	*result = (uint16_t)(sum / validCount);
	return 1;                           // 测距有效
}

/**
  * @brief  按顺序扫描所有预设角度，并将距离结果存入全局数组
  *          扫描顺序：165°(物理左) → 130°(左前) → 90°(正前) → 50°(右前) → 15°(物理右)
  *          扫描完成后舵机自动回正到90°
  */
static void Avoid_ScanAll(void)
{
	static const uint8_t angles[AVOID_SCAN_ANGLE_COUNT] = {165, 130, 90, 50, 15};
	uint8_t i;

	for(i = 0; i < AVOID_SCAN_ANGLE_COUNT; i++)
	{
		Servo_SetAngle(angles[i]);             // 舵机转到当前扫描角度
		Delay_ms(AVOID_SERVO_WAIT_TIME);       // 等待舵机到达目标位置
		if(Avoid_MeasureAverage(&Avoid_ScanDistances[i]) == 0)
		{
			/* 无回波说明该方向开阔，按最大量程处理 */
			Avoid_ScanDistances[i] = AVOID_NO_ECHO_DISTANCE;
		}
	}

	Servo_SetAngle(AVOID_SERVO_ANGLE);         // 扫描完毕，舵机回正
	Delay_ms(AVOID_SERVO_WAIT_TIME);           // 等待舵机回正稳定
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
static void Avoid_Turn(uint8_t direction, uint16_t turnTime)
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

	Delay_ms(turnTime);                    // 按动作类型固定时间转向
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
  *          流程：测前 → 停车后退 → 扫5角度 → 前通则直行 / 不通则选方向转 / 死路掉头
  */
void Avoid_Run(void)
{
	uint16_t leftDistance;                 // 左侧扫描距离
	uint16_t rightDistance;                // 右侧扫描距离
	uint8_t turnDirection;                 // 本次转向决策
	uint8_t frontValid;                    // 正前方测距是否有效

	/* 第一步：舵机保持正前方，进行中间方向测距 */
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	frontValid = Avoid_MeasureAverage(&Avoid_Distance);

	/* 测距无效时只停车重试，不能把无效值0当成障碍 */
	if(frontValid == 0)
	{
		Car_Stop();
		return;
	}

	/* 正前方距离足够，小车继续向前巡航 */
	if(Avoid_Distance > AVOID_DISTANCE)
	{
		Car_Forward(AVOID_SPEED);
		return;
	}

	/* 第二步：前方有障碍，停车并先后退 */
	Car_Stop();
	Delay_ms(AVOID_STOP_TIME);
	Car_Backward(AVOID_SPEED);
	Delay_ms(AVOID_BACK_TIME);
	Car_Stop();

	/* 第三步：后退完成后扫描全部5个角度 */
	Avoid_ScanAll();

	/* 从扫描结果中提取左、右两方向的距离 */
	leftDistance  = Avoid_ScanDistances[0];  // 165° 物理左侧
	rightDistance = Avoid_ScanDistances[4];  // 15° 物理右侧

	/* 扫描后正前方已经畅通，直接前进 */
	if(Avoid_ScanDistances[2] > AVOID_DISTANCE)
	{
		Car_Forward(AVOID_SPEED);
		return;
	}

	/* 第四步：比较左右距离并参考历史方向作出选择 */
	turnDirection = Avoid_SelectTurn(leftDistance, rightDistance);

	/* 死路时，向上一次转向的反方向掉头 */
	if(turnDirection == AVOID_TURN_NONE)
	{
		Avoid_Turn((Avoid_LastTurn == AVOID_TURN_LEFT) ?
			AVOID_TURN_RIGHT : AVOID_TURN_LEFT, AVOID_U_TURN_TIME);
		return;
	}

	/* 存在可通行方向，按固定时间执行原地转向 */
	Avoid_Turn(turnDirection, AVOID_TURN_TIME);
}
