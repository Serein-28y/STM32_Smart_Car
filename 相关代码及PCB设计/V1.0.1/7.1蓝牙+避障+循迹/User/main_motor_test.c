/*
 * main_motor_test.c —— 电机通道自检（临时诊断用）
 *
 * 用途：R指令"直行"、循迹"不按路线走"这类差速转向失效问题，先用本程序
 *       逐通道驱动电机，判断是软件/硬件/接线问题。
 *
 * 用法（二选一）：
 *   1) 先把 User\main.c 备份成 main.c.bak，再把本文件内容复制为 main.c；
 *      测试完恢复 main.c 即可。
 *   2) 或者把 main.c 里的 main 函数改名（如 main_app），把本文件加入工程编译。
 *
 * 串口 9600 会打印每一步，用手机蓝牙/USB转TTL观察即可。
 */
#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "stdio.h"

/* 执行一步：设定左右轮速度(左,右)，运行 ms 毫秒后停车 */
static void RunPhase(const char* name, int16_t left, int16_t right, uint16_t ms)
{
	printf("\r\n=== %s ===\r\n", name);
	Car_SetSpeed(left, right);
	Delay_ms(ms);
	Car_Stop();
	Delay_ms(600);
}

int main(void)
{
	Car_Init();
	Serial_Init();
	Car_DriveEnable();          /* 解除驱动锁止，允许电机动作 */

	Delay_ms(2000);             /* 上电预留2秒，把小车架起来/放稳 */

	printf("\r\nMOTOR SELF TEST START\r\n");

	/* 1~2：单轮测试。只应有一个轮子转。 */
	RunPhase("1. LEFT only fwd 40   (40,0)",   40,  0, 1500);
	RunPhase("2. RIGHT only fwd 40  (0,40)",    0, 40, 1500);

	/* 3：双轮同速直行。 */
	RunPhase("3. BOTH fwd 40        (40,40)",  40, 40, 1500);

	/* 4~5：差速转弯，应走出弧线而不是直行。 */
	RunPhase("4. TURN RIGHT         (40,20)",  40, 20, 1500);
	RunPhase("5. TURN LEFT          (20,40)",  20, 40, 1500);

	/* 6~7：原地旋转，应明显原地打转。 */
	RunPhase("6. SPIN LEFT          (-40,40)", -40, 40, 1500);
	RunPhase("7. SPIN RIGHT         (40,-40)",  40,-40, 1500);

	printf("\r\nMOTOR SELF TEST DONE\r\n");
	while(1)
	{
		Delay_ms(1000);
	}
}