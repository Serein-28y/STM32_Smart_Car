#ifndef __AVOID_H__
#define __AVOID_H__

#include "stm32f10x.h"

#define AVOID_DISTANCE			20		//避障距离，单位cm
#define AVOID_MEASURE_TIME		100		//测距等待时间，单位ms
#define AVOID_STOP_TIME			500		//停车时间，单位ms
#define AVOID_BACK_TIME			200		//后退时间，单位ms
#define AVOID_TURN_TIME			300		//左转时间，单位ms
#define AVOID_SERVO_ANGLE		90		//舵机正前方角度

void Avoid_Init(void);
void Avoid_Run(void);

#endif
