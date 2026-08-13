#include "stm32f10x.h"
#include "Delay.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"
#include "Serial.h"

uint16_t Avoid_Distance;
uint16_t Avoid_ScanDistances[AVOID_SCAN_ANGLE_COUNT];

/* 左右都合适且距离接近时，本次优先选择的方向；每次这种情况后交替。 */
static uint8_t Avoid_NextBothTurn = AVOID_INITIAL_BOTH_TURN;

/**
  * @brief  执行可被退出指令打断的毫秒延时。
  * @param  delayTime 计划延时时长，单位：ms。
  * @retval 1表示延时正常结束，0表示收到X并提前退出。
  */
static uint8_t Avoid_Delay(uint16_t delayTime)
{
	uint16_t step;

	while(delayTime > 0)
	{
		/* 将长延时拆成最多10ms的小段，保证X能够被及时检查。 */
		step = (delayTime > 10) ? 10 : delayTime;

		if(Serial_IsExitRequested() != 0)
		{
			return 0;
		}
		Delay_ms(step);
		delayTime -= step;
	}

	return (Serial_IsExitRequested() == 0);
}

/**
  * @brief  在当前舵机角度连续测量并取有效距离平均值。
  * @param  result 用于保存平均距离的地址，单位：cm。
  * @retval 1表示有有效结果，0表示无有效回波或收到退出指令。
  */
static uint8_t Avoid_MeasureAverage(uint16_t *result)
{
	uint32_t sum = 0;
	uint8_t validCount = 0;
	uint8_t i;
	uint16_t distance;

	for(i = 0; i < AVOID_MEASURE_COUNT; i++)
	{
		if(Serial_IsExitRequested() != 0)
		{
			return 0;
		}
		HC_SR04_Start();
		if(Avoid_Delay(AVOID_MEASURE_TIME) == 0)
		{
			return 0;
		}

		if(HC_SR04_GetDistance(&distance) == 1 && distance > 0 &&
			distance < AVOID_VALID_DISTANCE_MAX)
		{
			sum += distance;
			validCount++;
		}
	}

	/* 多次测量都没有有效回波时，不使用未初始化的距离参与决策。 */
	if(validCount == 0)
	{
		*result = 0;
		return 0;
	}

	*result = (uint16_t)(sum / validCount);
	return 1;
}

/**
  * @brief  只扫描左右两个侧向角度，完成后舵机回到正前方。
  *         注意：角度大于90度为物理左侧，小于90度为物理右侧。
  * @param  leftDistance 用于保存左侧距离的地址，单位：cm。
  * @param  rightDistance 用于保存右侧距离的地址，单位：cm。
  * @retval 1表示扫描完成，0表示收到退出指令并提前结束。
  */
static uint8_t Avoid_ScanSides(uint16_t *leftDistance, uint16_t *rightDistance)
{
	Servo_SetAngle(AVOID_SERVO_LEFT_ANGLE);       /* 大于90度：左 */
	if(Avoid_Delay(AVOID_SERVO_WAIT_TIME) == 0) return 0;
	if(Avoid_MeasureAverage(leftDistance) == 0)
	{
		if(Serial_IsExitRequested() != 0) return 0;
		/* 侧向无回波按最大量程处理，表示该方向暂未检测到近处障碍。 */
		*leftDistance = AVOID_NO_ECHO_DISTANCE;
	}

	Servo_SetAngle(AVOID_SERVO_RIGHT_ANGLE);      /* 小于90度：右 */
	if(Avoid_Delay(AVOID_SERVO_WAIT_TIME) == 0) return 0;
	if(Avoid_MeasureAverage(rightDistance) == 0)
	{
		if(Serial_IsExitRequested() != 0) return 0;
		/* 侧向无回波按最大量程处理，表示该方向暂未检测到近处障碍。 */
		*rightDistance = AVOID_NO_ECHO_DISTANCE;
	}

	Avoid_ScanDistances[0] = *leftDistance;
	Avoid_ScanDistances[1] = *rightDistance;

	Servo_SetAngle(AVOID_SERVO_ANGLE);
	if(Avoid_Delay(AVOID_SERVO_WAIT_TIME) == 0) return 0;
	return 1;
}

/**
  * @brief  根据左右距离选择方向。
  *         两侧距离差不大时使用左右交替记忆，差距明显时选择更宽的一侧。
  * @param  leftDistance 左侧测量距离，单位：cm。
  * @param  rightDistance 右侧测量距离，单位：cm。
  * @retval AVOID_TURN_LEFT、AVOID_TURN_RIGHT或AVOID_TURN_NONE。
  */
static uint8_t Avoid_SelectTurn(uint16_t leftDistance, uint16_t rightDistance)
{
	uint16_t difference;

	if(leftDistance <= AVOID_SIDE_DISTANCE && rightDistance <= AVOID_SIDE_DISTANCE)
	{
		return AVOID_TURN_NONE;
	}

	if(leftDistance > AVOID_SIDE_DISTANCE && rightDistance <= AVOID_SIDE_DISTANCE)
	{
		return AVOID_TURN_LEFT;
	}

	if(rightDistance > AVOID_SIDE_DISTANCE && leftDistance <= AVOID_SIDE_DISTANCE)
	{
		return AVOID_TURN_RIGHT;
	}

	/* 到这里说明左右两侧都超过了安全距离。 */
	if(leftDistance >= rightDistance)
	{
		difference = leftDistance - rightDistance;
	}
	else
	{
		difference = rightDistance - leftDistance;
	}

	if(difference > AVOID_SIDE_DIFF)
	{
		return (leftDistance > rightDistance) ?
			AVOID_TURN_LEFT : AVOID_TURN_RIGHT;
	}

	/* 两侧接近：按记忆方向选择，并为下一次换边。 */
	if(Avoid_NextBothTurn == AVOID_TURN_LEFT)
	{
		Avoid_NextBothTurn = AVOID_TURN_RIGHT;
		return AVOID_TURN_LEFT;
	}

	Avoid_NextBothTurn = AVOID_INITIAL_BOTH_TURN;
	return AVOID_TURN_RIGHT;
}

/**
  * @brief  按指定方向执行一次可中断的原地转向。
  * @param  direction 转向方向，取AVOID_TURN_LEFT或AVOID_TURN_RIGHT。
  * @param  turnTime 原地转向持续时间，单位：ms。
  * @retval 1表示转向完成，0表示收到退出指令并提前停止。
  */
static uint8_t Avoid_Turn(uint8_t direction, uint16_t turnTime)
{
	if(direction == AVOID_TURN_LEFT)
	{
		Car_SpinLeft(AVOID_SPEED);
	}
	else
	{
		Car_SpinRight(AVOID_SPEED);
	}

	if(Avoid_Delay(turnTime) == 0)
	{
		Car_Stop();
		return 0;
	}
	Car_Stop();
	return 1;
}

/**
  * @brief  初始化自动避障状态、舵机方向和扫描结果。
  * @param  无
  * @retval 无
  */
void Avoid_Init(void)
{
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	Avoid_NextBothTurn = AVOID_INITIAL_BOTH_TURN;
	Avoid_ScanDistances[0] = 0;
	Avoid_ScanDistances[1] = 0;
	Car_Stop();
}

/**
  * @brief  退出自动避障，锁止电机并让舵机回到正前方。
  * @param  无
  * @retval 无
  */
void Avoid_Exit(void)
{
	Car_EmergencyStop();
	Servo_SetAngle(AVOID_SERVO_ANGLE);
}

/**
  * @brief  执行一轮自动避障：正前方检测 -> 后退扫描，直到一侧打开 -> 转约90度。
  * @param  无
  * @retval 无；每轮结束或收到退出指令后返回主状态机。
  */
void Avoid_Run(void)
{
	uint16_t leftDistance;
	uint16_t rightDistance;
	uint8_t turnDirection;
	uint8_t frontValid;

	/* 正常行驶时只测正前方。 */
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	frontValid = Avoid_MeasureAverage(&Avoid_Distance); /* 测量正前方距离。 */
	/* 测距函数的0同时可能表示退出，因此必须先判断全局退出标志。 */
	if(Serial_IsExitRequested() != 0) return;
	if(frontValid == 0)
	{
		Car_Stop();
		return;					//结束Avoid_Run
	}

	if(Avoid_Distance > AVOID_DISTANCE)
	{
		Car_Forward(AVOID_SPEED);
		return;
	}

	/* 发现障碍：停车稳定后开始“后退一步 -> 扫描左右”的循环。 */
	Car_Stop();
	if(Avoid_Delay(AVOID_STOP_TIME) == 0) return;

	/* 两侧都被挡住时采用“后退一步再扫描”，但每一轮都允许X退出。 */
	while(Serial_IsExitRequested() == 0)
	{
		Car_Backward(AVOID_BACK_SPEED);
		if(Avoid_Delay(AVOID_BACK_STEP_TIME) == 0) return;
		Car_Stop();

		/* 扫描结束时舵机已立即回到90度，再进行方向决策。 */
		if(Avoid_ScanSides(&leftDistance, &rightDistance) == 0) return;
		turnDirection = Avoid_SelectTurn(leftDistance, rightDistance);
		if(turnDirection != AVOID_TURN_NONE)
		{
			break;
		}
	}
	if(Serial_IsExitRequested() != 0) return;

	/* 至少一侧打开后，原地转约90度；舵机已经提前回到正前方。 */
	(void)Avoid_Turn(turnDirection, AVOID_TURN_90_TIME);			//执行转向动作
}
