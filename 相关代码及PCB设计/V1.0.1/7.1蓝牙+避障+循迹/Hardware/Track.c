#include "stm32f10x.h"
#include "Car.h"
#include "Track.h"

/*
 * 第二阶段使用固定速度状态表，所有速度范围均为0~100。
 * 转弯时降低内侧轮速度、提高外侧轮速度，从而利用左右轮差速修正方向。
 */
#define TRACK_STRAIGHT_SPEED       32
#define TRACK_SLIGHT_INNER_SPEED   20
#define TRACK_SLIGHT_OUTER_SPEED   42
#define TRACK_MEDIUM_INNER_SPEED   10
#define TRACK_MEDIUM_OUTER_SPEED   46
#define TRACK_SHARP_INNER_SPEED     0
#define TRACK_SHARP_OUTER_SPEED    50
#define TRACK_SEARCH_SPEED         35

#define TRACK_DIRECTION_LEFT       (-1)
#define TRACK_DIRECTION_NONE         0
#define TRACK_DIRECTION_RIGHT        1

/*
 * 记录最后一次发现黑线的方向。
 * 当四路全部为0（丢线）时，小车继续向最后发现黑线的一侧搜索。
 */
static int8_t Track_LastDirection = TRACK_DIRECTION_NONE;

void Track_Init(void)
{
	Track_LastDirection = TRACK_DIRECTION_NONE;
	Car_Stop();
}

void Track_Run(uint8_t state)
{
	/*
	 * state的bit3~bit0依次为L2、L1、R1、R2；1表示检测到黑线。
	 * 黑线位于左侧时左轮减速、右轮加速；位于右侧时动作相反。
	 */
	switch(state)
	{
		case 0x06: /* 0110：黑线位于两个内侧传感器下方，保持直行。 */
			Car_SetSpeed(TRACK_STRAIGHT_SPEED, TRACK_STRAIGHT_SPEED);
			break;

		case 0x04: /* 0100：黑线略偏左，小幅向左修正。 */
			Track_LastDirection = TRACK_DIRECTION_LEFT;
			Car_SetSpeed(TRACK_SLIGHT_INNER_SPEED, TRACK_SLIGHT_OUTER_SPEED);
			break;

		case 0x0C: /* 1100：黑线明显偏左。 */
		case 0x0E: /* 1110：左侧较宽区域检测到黑线，同样向左修正。 */
			Track_LastDirection = TRACK_DIRECTION_LEFT;
			Car_SetSpeed(TRACK_MEDIUM_INNER_SPEED, TRACK_MEDIUM_OUTER_SPEED);
			break;

		case 0x08: /* 1000：只有左外侧检测到黑线，需要大幅左转。 */
			Track_LastDirection = TRACK_DIRECTION_LEFT;
			Car_SetSpeed(TRACK_SHARP_INNER_SPEED, TRACK_SHARP_OUTER_SPEED);
			break;

		case 0x02: /* 0010：黑线略偏右，小幅向右修正。 */
			Track_LastDirection = TRACK_DIRECTION_RIGHT;
			Car_SetSpeed(TRACK_SLIGHT_OUTER_SPEED, TRACK_SLIGHT_INNER_SPEED);
			break;

		case 0x03: /* 0011：黑线明显偏右。 */
		case 0x07: /* 0111：右侧较宽区域检测到黑线，同样向右修正。 */
			Track_LastDirection = TRACK_DIRECTION_RIGHT;
			Car_SetSpeed(TRACK_MEDIUM_OUTER_SPEED, TRACK_MEDIUM_INNER_SPEED);
			break;

		case 0x01: /* 0001：只有右外侧检测到黑线，需要大幅右转。 */
			Track_LastDirection = TRACK_DIRECTION_RIGHT;
			Car_SetSpeed(TRACK_SHARP_OUTER_SPEED, TRACK_SHARP_INNER_SPEED);
			break;

		case 0x00: /* 0000：四路均为白底，按上次偏移方向搜索黑线。 */
			if(Track_LastDirection == TRACK_DIRECTION_LEFT)
			{
				Car_SetSpeed(0, TRACK_SEARCH_SPEED);
			}
			else if(Track_LastDirection == TRACK_DIRECTION_RIGHT)
			{
				Car_SetSpeed(TRACK_SEARCH_SPEED, 0);
			}
			else
			{
				Car_Stop();
			}
			break;

		case 0x0F: /* 1111：横线或大面积黑色区域，按照要求立即停车。 */
			Car_Stop();
			break;

		default: /* 其余不连续或含义不明确的组合，为安全起见停车。 */
			Car_Stop();
			break;
	}
}
