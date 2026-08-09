#include "stm32f10x.h"
#include "Delay.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"

uint16_t Avoid_Distance;
uint16_t Avoid_ScanDistances[AVOID_SCAN_ANGLE_COUNT];

/* 左右都合适且距离接近时，本次优先选择的方向；每次这种情况后交替。 */
static uint8_t Avoid_NextBothTurn = AVOID_INITIAL_BOTH_TURN;

/**
  * @brief  在当前舵机角度连续测量并取有效距离平均值。
  * @return 1 表示有有效结果，0 表示本次没有有效回波。
  */
static uint8_t Avoid_MeasureAverage(uint16_t *result)
{
	uint32_t sum = 0;
	uint8_t validCount = 0;
	uint8_t i;
	uint16_t distance;

	for(i = 0; i < AVOID_MEASURE_COUNT; i++)
	{
		HC_SR04_Start();
		Delay_ms(AVOID_MEASURE_TIME);

		if(HC_SR04_GetDistance(&distance) == 1 && distance > 0 &&
			distance < AVOID_VALID_DISTANCE_MAX)
		{
			sum += distance;
			validCount++;
		}
	}

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
  */
static void Avoid_ScanSides(uint16_t *leftDistance, uint16_t *rightDistance)
{
	Servo_SetAngle(AVOID_SERVO_LEFT_ANGLE);       /* 大于90度：左 */
	Delay_ms(AVOID_SERVO_WAIT_TIME);
	if(Avoid_MeasureAverage(leftDistance) == 0)
	{
		/* 侧向无回波按最大量程处理，表示该方向暂未检测到近处障碍。 */
		*leftDistance = AVOID_NO_ECHO_DISTANCE;
	}

	Servo_SetAngle(AVOID_SERVO_RIGHT_ANGLE);      /* 小于90度：右 */
	Delay_ms(AVOID_SERVO_WAIT_TIME);
	if(Avoid_MeasureAverage(rightDistance) == 0)
	{
		/* 侧向无回波按最大量程处理，表示该方向暂未检测到近处障碍。 */
		*rightDistance = AVOID_NO_ECHO_DISTANCE;
	}

	Avoid_ScanDistances[0] = *leftDistance;
	Avoid_ScanDistances[1] = *rightDistance;

	Servo_SetAngle(AVOID_SERVO_ANGLE);
	Delay_ms(AVOID_SERVO_WAIT_TIME);
}

/**
  * @brief  根据左右距离选择方向。
  *         两侧距离差不大时使用左右交替记忆，差距明显时选择更宽的一侧。
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
  * @brief  执行原地转向并记录本次实际方向。
  */
static void Avoid_Turn(uint8_t direction, uint16_t turnTime)
{
	if(direction == AVOID_TURN_LEFT)
	{
		Car_SpinLeft(AVOID_SPEED);
	}
	else
	{
		Car_SpinRight(AVOID_SPEED);
	}

	Delay_ms(turnTime);
	Car_Stop();
}

void Avoid_Init(void)
{
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	Avoid_NextBothTurn = AVOID_INITIAL_BOTH_TURN;
	Avoid_ScanDistances[0] = 0;
	Avoid_ScanDistances[1] = 0;
	Car_Stop();
}

/**
  * @brief  执行一轮自动避障：正前方检测 -> 后退扫描，直到一侧打开 -> 转约90度。
  */
void Avoid_Run(void)
{
	uint16_t leftDistance;
	uint16_t rightDistance;
	uint8_t turnDirection;
	uint8_t frontValid;

	/* 正常行驶时只测正前方。 */
	Servo_SetAngle(AVOID_SERVO_ANGLE);
	frontValid = Avoid_MeasureAverage(&Avoid_Distance);//测距然后判断是否有障碍物
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
	Delay_ms(AVOID_STOP_TIME);

	while(1)
	{
		Car_Backward(AVOID_BACK_SPEED);
		Delay_ms(AVOID_BACK_STEP_TIME);
		Car_Stop();

		/* 扫描结束时舵机已立即回到90度，再进行方向决策。 */
		Avoid_ScanSides(&leftDistance, &rightDistance);       //舵机左右扫描记录讲数据存到到数组里面
		turnDirection = Avoid_SelectTurn(leftDistance, rightDistance);//判断转向
		if(turnDirection != AVOID_TURN_NONE)
		{
			break;
		}
	}

	/* 至少一侧打开后，原地转约90度；舵机已经提前回到正前方。 */
	Avoid_Turn(turnDirection, AVOID_TURN_90_TIME);					//执行转向动作
}
