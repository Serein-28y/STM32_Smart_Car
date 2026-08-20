#ifndef __TRACK_H__
#define __TRACK_H__

#include "stm32f10x.h"

void Track_Init(void);
/* 根据四位传感器状态执行一次低速状态表循迹控制。 */
void Track_Run(uint8_t state);

#endif
