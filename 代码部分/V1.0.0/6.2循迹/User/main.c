#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "TrackSensor.h"
#include "Track.h"
#include "stdio.h"

int main(void)
{
	uint8_t state;
	uint8_t printCounter = 0;

	/* 初始化电机、蓝牙串口、四路循迹输入和循迹控制状态。 */
	Car_Init();
	Serial_Init();
	TrackSensor_Init();
	Track_Init();

	printf("4-channel track test\r\n");
	printf("L2 L1 R1 R2 = PB15 PB14 PB13 PB10\r\n");
	printf("Start after 2 seconds\r\n");
	/* 上电后预留2秒，方便将小车放稳；延时结束后自动开始循迹。 */
	Delay_ms(2000);

	while(1)
	{
		/* 读取顺序固定为L2、L1、R1、R2，并立即执行一次状态表控制。 */
		state = TrackSensor_ReadState();
		Track_Run(state);

		/*
		 * 控制周期为10ms，但串口只每20次循环打印一次（约200ms）。
		 * 这样既能在手机上观察状态，又能减少阻塞式printf对循迹实时性的影响。
		 */
		printCounter++;
		if(printCounter >= 20)
		{
			printCounter = 0;
			/* 右移并与1相与，依次取出state的bit3、bit2、bit1、bit0。 */
			printf("Track=%d%d%d%d\r\n",
				(state >> 3) & 0x01,
				(state >> 2) & 0x01,
				(state >> 1) & 0x01,
				state & 0x01);
		}

		Delay_ms(10);
	}
}
