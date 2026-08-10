#ifndef __TRACK_SENSOR_H__
#define __TRACK_SENSOR_H__

#include "stm32f10x.h"

/* 第一个 TCRT5000 的 DO 接 PB10。 */
#define TRACK_SENSOR_PORT GPIOB
#define TRACK_SENSOR_PIN  GPIO_Pin_10

void TrackSensor_Init(void);
uint8_t TrackSensor_Read(void);

#endif
