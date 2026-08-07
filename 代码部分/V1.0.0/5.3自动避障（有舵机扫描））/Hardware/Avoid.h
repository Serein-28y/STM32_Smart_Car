#ifndef __AVOID_H__
#define __AVOID_H__

#include "stm32f10x.h"
#include "Car.h"

#define AVOID_DISTANCE           26      // 正前方触发避障的距离，单位：cm
#define AVOID_SIDE_DISTANCE      50      // 左右方向最低可通行距离，单位：cm
#define AVOID_MEASURE_TIME       150     // 每次超声波测距等待时间，单位：ms
#define AVOID_SERVO_WAIT_TIME    250     // 舵机转到目标角度后的稳定等待时间，单位：ms
#define AVOID_MEASURE_COUNT      2       // 每个方向的测量次数
#define AVOID_STOP_TIME          300     // 发现障碍后的停车稳定时间，单位：ms
#define AVOID_BACK_TIME          200     // 正常避障后退时间，单位：ms
#define AVOID_DEAD_BACK_TIME     300     // 死路复测前的额外后退时间，单位：ms
#define AVOID_TURN_TIME          200     // 原地转向时间，单位：ms，需要实车调试
#define AVOID_U_TURN_TIME        (AVOID_TURN_TIME * 2) // 死路掉头时间，初始为普通转向的2倍
#define AVOID_SERVO_ANGLE        90      // 舵机正前方角度
#define AVOID_SERVO_LEFT_ANGLE   165     // 舵机左侧（物理）扫描角度
#define AVOID_SERVO_RIGHT_ANGLE  15      // 舵机右侧（物理）扫描角度
#define AVOID_SPEED              SPEED_MEDIUM // 避障行驶速度

#define AVOID_TURN_NONE          0       // 无转向决策，左右均不可通行
#define AVOID_TURN_LEFT          1       // 左转
#define AVOID_TURN_RIGHT         2       // 右转

#define AVOID_SCAN_ANGLE_COUNT   5       // 多角度扫描数量
#define AVOID_NO_ECHO_DISTANCE   400     // 无回波时视为开阔空间，单位：cm（HC-SR04最大量程）

extern uint16_t Avoid_ScanDistances[AVOID_SCAN_ANGLE_COUNT]; // 所有扫描角度的距离结果

void Avoid_Init(void);
void Avoid_Run(void);

#endif
