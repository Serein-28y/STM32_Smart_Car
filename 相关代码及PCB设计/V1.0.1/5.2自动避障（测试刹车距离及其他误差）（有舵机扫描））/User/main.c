#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"

/*
 * 刹车距离测试参数：
 *   TEST_STOP_DISTANCE_CM：发出停车命令的距离阈值，默认沿用自动避障阈值；
 *   TEST_REPORT_INTERVAL_MS：停车后两次蓝牙数据之间的等待时间。
 * 超声波回波最长等待时间沿用 AVOID_MEASURE_TIME（当前为 150 ms）。
 */
#define TEST_STOP_DISTANCE_CM    AVOID_DISTANCE
#define TEST_DRIVE_SPEED         AVOID_SPEED
#define TEST_MEASURE_WAIT_MS     AVOID_MEASURE_TIME
#define TEST_REPORT_INTERVAL_MS  500

typedef enum
{
	TEST_STATE_FORWARD = 0,
	TEST_STATE_STOPPED
} TestState_t;

/* 触发一次 HC-SR04 并读取有效的厘米距离。 */
static uint8_t Test_MeasureFront(uint16_t *distance)
{
	HC_SR04_Start();
	Delay_ms(TEST_MEASURE_WAIT_MS);

	if(HC_SR04_GetDistance(distance) == 1 && *distance > 0 && *distance < 400)
	{
		return 1;
	}

	*distance = 0;
	return 0;
}

/* 通过蓝牙发送一行带标签的距离数据，便于手机端直接记录。 */
static void Test_SendDistance(char *label, uint16_t distance)
{
	Serial_SendString(label);
	Serial_SendNum(distance, 3);
	Serial_SendString(" cm\r\n");
}

int main(void)
{
	uint16_t distance;
	TestState_t state = TEST_STATE_FORWARD;

	Car_Init();
	Servo_Init();
	Servo_SetAngle(AVOID_SERVO_ANGLE);   /* 确保超声波朝向正前方 */
	HC_SR04_Init();
	Serial_Init();                       /* USART1 连接 HC-06/HC-05 */

	Serial_SendString("BRAKE_TEST START, threshold=");
	Serial_SendNum(TEST_STOP_DISTANCE_CM, 3);
	Serial_SendString(" cm\r\n");
	Car_Forward(TEST_DRIVE_SPEED);

	while(1)
	{
		if(state == TEST_STATE_FORWARD)
		{
			/* 一直前进，直到测得距离小于等于停车阈值。 */
			if(Test_MeasureFront(&distance) == 0)
			{
				/* 测距无效时停车，避免传感器异常导致继续撞击。 */
				Car_Stop();
				state = TEST_STATE_STOPPED;
				Serial_SendString("BRAKE_TEST SENSOR_INVALID\r\n");
				continue;
			}

			if(distance <= TEST_STOP_DISTANCE_CM)
			{
				/* 记录发出停车命令瞬间的距离，随后只测量不再驱动电机。 */
				Car_Stop();
				state = TEST_STATE_STOPPED;
				Test_SendDistance("STOP_CMD distance=", distance);
			}
			else
			{
				/* 防止串口中断改变电机状态，主循环每次有效测量都重新前进。 */
				Car_Forward(TEST_DRIVE_SPEED);
			}
		}
		else
		{
			/* 停车后持续测量，回传的值就是包含刹车滑行后的实际距离。 */
			if(Test_MeasureFront(&distance) == 1)
			{
				Test_SendDistance("STOPPED actual_distance=", distance);
			}
			else
			{
				Serial_SendString("STOPPED actual_distance=INVALID\r\n");
			}

			/* 让手机端按固定节奏接收数据；电机保持停止。 */
			Car_Stop();
			Delay_ms(TEST_REPORT_INTERVAL_MS);
		}
	}
}
