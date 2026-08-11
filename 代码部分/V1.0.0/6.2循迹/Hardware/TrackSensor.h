#ifndef __TRACK_SENSOR_H__
#define __TRACK_SENSOR_H__

#include "stm32f10x.h"

/*
 * 从车头正面看，四个循迹传感器由左到右依次为：
 * L2（左外侧）、L1（左内侧）、R1（右内侧）、R2（右外侧）。
 * 实际接线：L2=PB15、L1=PB14、R1=PB13、R2=PB10。
 * 经实测，本组 TCRT5000 模块白底输出0，黑线输出1，因此读取后无需取反。
 */
#define TRACK_SENSOR_PORT   GPIOB
#define TRACK_SENSOR_L2_PIN GPIO_Pin_15
#define TRACK_SENSOR_L1_PIN GPIO_Pin_14
#define TRACK_SENSOR_R1_PIN GPIO_Pin_13
#define TRACK_SENSOR_R2_PIN GPIO_Pin_10

void TrackSensor_Init(void);
/* 返回四位循迹状态：bit3~bit0依次表示L2、L1、R1、R2。 */
uint8_t TrackSensor_ReadState(void);

#endif
