#include "stm32f10x.h"
#include "TrackSensor.h"

void TrackSensor_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 四路传感器全部接在GPIOB，只需开启一次GPIOB时钟。 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = TRACK_SENSOR_L2_PIN |
		TRACK_SENSOR_L1_PIN | TRACK_SENSOR_R1_PIN | TRACK_SENSOR_R2_PIN;
	/*
	 * 配置为上拉输入：传感器正常连接时由DO输出决定电平；
	 * 如果某根DO线松动或断开，上拉可避免GPIO处于随机跳变的浮空状态。
	 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TRACK_SENSOR_PORT, &GPIO_InitStructure);
}

uint8_t TrackSensor_ReadState(void)
{
	uint8_t state = 0;

	/*
	 * 把四个独立GPIO压缩到一个变量中，便于switch状态表判断：
	 * bit3 bit2 bit1 bit0
	 *  L2   L1   R1   R2
	 * 例如0110表示两个内侧传感器检测到黑线，小车基本居中。
	 */
	if(GPIO_ReadInputDataBit(TRACK_SENSOR_PORT, TRACK_SENSOR_L2_PIN)) state |= 0x08;
	if(GPIO_ReadInputDataBit(TRACK_SENSOR_PORT, TRACK_SENSOR_L1_PIN)) state |= 0x04;
	if(GPIO_ReadInputDataBit(TRACK_SENSOR_PORT, TRACK_SENSOR_R1_PIN)) state |= 0x02;
	if(GPIO_ReadInputDataBit(TRACK_SENSOR_PORT, TRACK_SENSOR_R2_PIN)) state |= 0x01;

	return state;
}
