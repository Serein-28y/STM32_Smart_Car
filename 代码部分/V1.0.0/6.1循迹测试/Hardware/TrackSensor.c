#include "stm32f10x.h"
#include "TrackSensor.h"

void TrackSensor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = TRACK_SENSOR_PIN;
	/* TCRT5000 模块的 DO 常见为开集电极输出；上拉可保证未拉低时电平确定。 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TRACK_SENSOR_PORT, &GPIO_InitStructure);
}

uint8_t TrackSensor_Read(void)
{
	return (uint8_t)GPIO_ReadInputDataBit(TRACK_SENSOR_PORT, TRACK_SENSOR_PIN);
}
