#ifndef __AVOID_H__
#define __AVOID_H__

#include "stm32f10x.h"
#include "Car.h"

#define AVOID_DISTANCE           26      /* 前方触发避障的距离，单位：cm */
#define AVOID_SIDE_DISTANCE      40      /* 左右方向可通行的最小距离，单位：cm */
#define AVOID_SIDE_DIFF          20      /* 左右距离差超过此值时优先走更宽的一侧，单位：cm */

#define AVOID_MEASURE_TIME       150     /* 两次超声波测量之间的等待时间，单位：ms */
#define AVOID_SERVO_WAIT_TIME    250     /* 舵机转到扫描角度后的稳定等待时间，单位：ms */
#define AVOID_MEASURE_COUNT      2       /* 每个方向的测量次数，最后取有效值平均值 */
#define AVOID_STOP_TIME          300     /* 发现障碍后停车稳定时间，单位：ms */
#define AVOID_BACK_STEP_TIME     200     /* 每轮左右扫描前的后退时间，单位：ms */
#define AVOID_BACK_SPEED         SPEED_BACKWARD /* 自动避障后退速度，实际数值在Car.h中设置 */
#define AVOID_TURN_90_TIME       600     /* 选择方向后原地转约90度的时间，单位：ms，需实车调整 */

#define AVOID_SERVO_ANGLE        90      /* 舵机正前方角度，单位：度 */
#define AVOID_SERVO_LEFT_ANGLE   150     /* 左侧扫描角度：大于90度为物理左侧，单位：度 */
#define AVOID_SERVO_RIGHT_ANGLE  30      /* 右侧扫描角度：小于90度为物理右侧，单位：度 */
#define AVOID_SPEED              SPEED_MEDIUM /* 避障时小车行驶和转向速度 */

#define AVOID_TURN_NONE          0       /* 无可通行方向 */
#define AVOID_TURN_LEFT          1       /* 选择左转 */
#define AVOID_TURN_RIGHT         2       /* 选择右转 */
#define AVOID_INITIAL_BOTH_TURN  AVOID_TURN_LEFT /* 左右都合适时第一次优先方向 */

#define AVOID_SCAN_ANGLE_COUNT   2       /* 扫描结果数量：0为左侧，1为右侧 */
#define AVOID_VALID_DISTANCE_MAX 400     /* 超声波测量的有效距离上限，单位：cm */
#define AVOID_NO_ECHO_DISTANCE   AVOID_VALID_DISTANCE_MAX /* 侧向无回波时采用的开阔距离，单位：cm */

extern uint16_t Avoid_ScanDistances[AVOID_SCAN_ANGLE_COUNT];

void Avoid_Init(void);
void Avoid_Run(void);

#endif
