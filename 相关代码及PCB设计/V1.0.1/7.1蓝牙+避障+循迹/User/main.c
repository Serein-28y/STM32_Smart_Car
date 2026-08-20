#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"
#include "TrackSensor.h"
#include "Track.h"
#include "stdio.h"

typedef enum
{
	MODE_MENU = 0,
	MODE_MANUAL,
	MODE_TRACK,
	MODE_AVOID
} CarMode;

/* 当前运行模式只允许由菜单进入、由X指令退出，防止模式间直接抢占电机。 */
static CarMode CurrentMode = MODE_MENU;

/**
  * @brief  获取当前运行模式的英文名称，用于蓝牙状态反馈。
  * @param  无
  * @retval 当前模式对应的只读字符串。
  */
static const char *Menu_GetModeName(void)
{
	switch(CurrentMode)
	{
		case MODE_MANUAL: return "MANUAL";
		case MODE_TRACK:  return "TRACK";
		case MODE_AVOID:  return "AVOID";
		default:          return "MENU";
	}
}

/**
  * @brief  通过蓝牙串口发送菜单、当前模式和可用指令。
  * @param  无
  * @retval 无
  */
static void Menu_Print(void)
{
	printf("\r\n=== SMART CAR MENU ===\r\n");
	printf("Current: %s\r\n", Menu_GetModeName());
	printf("1: Manual control\r\n");
	printf("2: Auto track\r\n");
	printf("3: Auto avoid\r\n");
	printf("X: Stop and exit mode\r\n");
	printf("?: Show menu/status\r\n");
	printf("Manual: F B L R A D S\r\n");
}

/**
  * @brief  从菜单进入指定模式，并初始化该模式的控制状态。
  * @param  newMode 目标模式，只允许MANUAL、TRACK或AVOID。
  * @retval 无
  */
static void Menu_EnterMode(CarMode newMode)
{
	/* 清除上一个模式残留的退出请求和普通指令，再允许电机接受新命令。 */
	Serial_ClearExitRequest();
	Serial_ClearRxBuffer();
	Car_DriveEnable();
	CurrentMode = newMode;

	switch(CurrentMode)
	{
		case MODE_MANUAL:
			Car_Stop();
			printf("OK: MANUAL mode\r\n");
			break;

		case MODE_TRACK:
			Track_Init();
			printf("OK: TRACK mode\r\n");
			break;

		case MODE_AVOID:
			Avoid_Init();
			printf("OK: AVOID mode\r\n");
			break;

		default:
			Car_EmergencyStop();
			CurrentMode = MODE_MENU;
			break;
	}
}

/**
  * @brief  停止当前模式、清理模式状态并返回主菜单。
  * @param  无
  * @retval 无
  */
static void Menu_ExitMode(void)
{
	/* 避障退出时还需要让舵机回正，其他模式只需锁止电机。 */
	if(CurrentMode == MODE_AVOID)
	{
		Avoid_Exit();
	}
	else
	{
		Car_EmergencyStop();
	}

	CurrentMode = MODE_MENU;
	Serial_ClearRxBuffer();
	Serial_ClearExitRequest();
	printf("OK: stopped, back to MENU\r\n");
	Menu_Print();
}

/**
  * @brief  处理MENU状态下收到的单字符指令。
  * @param  command 蓝牙接收到的菜单指令。
  * @retval 无
  */
static void Menu_HandleMenuCommand(uint8_t command)
{
	switch(command)
	{
		case '1': Menu_EnterMode(MODE_MANUAL); break;
		case '2': Menu_EnterMode(MODE_TRACK);  break;
		case '3': Menu_EnterMode(MODE_AVOID);  break;
		case '?': Menu_Print();                 break;
		case '\r':
		case '\n': break;
		default:
			Car_EmergencyStop();
			printf("ERR: choose 1, 2, 3 or ?\r\n");
			break;
	}
}

/**
  * @brief  处理MANUAL状态下的方向、停车和查询指令。
  * @param  command 蓝牙接收到的手动控制指令。
  * @retval 无
  */
static void Menu_HandleManualCommand(uint8_t command)
{
	switch(command)
	{
		case 'F': Car_Forward(SPEED_MEDIUM);   printf("MANUAL: F\r\n"); break;
		case 'B': Car_Backward(SPEED_MEDIUM);  printf("MANUAL: B\r\n"); break;
		case 'L': Car_TurnLeft(SPEED_MEDIUM);  printf("MANUAL: L\r\n"); break;
		case 'R': Car_TurnRight(SPEED_MEDIUM); printf("MANUAL: R\r\n"); break;
		case 'A': Car_SpinLeft(SPEED_MEDIUM);  printf("MANUAL: A\r\n"); break;
		case 'D': Car_SpinRight(SPEED_MEDIUM); printf("MANUAL: D\r\n"); break;
		case 'S': Car_Stop();                  printf("MANUAL: STOP\r\n"); break;
		case '?': Menu_Print(); break;
		case '1':
		case '2':
		case '3':
			printf("ERR: send X before changing mode\r\n");
			break;
		case '\r':
		case '\n': break;
		default:
			/* 未知手动指令优先停车，避免沿用上一次运动状态。 */
			Car_Stop();
			printf("ERR: invalid MANUAL command, stopped\r\n");
			break;
	}
}

/**
  * @brief  处理TRACK或AVOID自动模式下收到的普通指令。
  * @param  command 蓝牙接收到的指令；自动模式只接受查询，X由中断处理。
  * @retval 无
  */
static void Menu_HandleAutoCommand(uint8_t command)
{
	if(command == '?')
	{
		Menu_Print();
	}
	else if(command == '1' || command == '2' || command == '3')
	{
		printf("ERR: send X before changing mode\r\n");
	}
	else if(command != '\r' && command != '\n')
	{
		printf("ERR: only X or ? is valid in AUTO mode\r\n");
	}
}

/**
  * @brief  初始化智能小车，并循环调度菜单、手动、循迹和避障模式。
  * @param  无
  * @retval 程序持续运行，不返回。
  */
int main(void)
{
	uint8_t command;

	Car_Init();
	Serial_Init();
	TrackSensor_Init();
	Servo_Init();
	HC_SR04_Init();
	Track_Init();
	Avoid_Init();
	Car_EmergencyStop();

	printf("Smart car ready. Send ? for menu.\r\n");

	while(1)
	{
		/* X具有最高优先级：先完成模式退出，再处理任何普通接收数据。 */
		if(Serial_IsExitRequested() != 0)
		{
			Menu_ExitMode();
			continue;
		}

		/* 一次取完缓冲区中的指令，避免快速连续发送时只处理最后一个字节。 */
		while(Serial_ReadByte(&command) != 0)
		{
			if(CurrentMode == MODE_MENU)
			{
				Menu_HandleMenuCommand(command);
			}
			else if(CurrentMode == MODE_MANUAL)
			{
				Menu_HandleManualCommand(command);
			}
			else
			{
				Menu_HandleAutoCommand(command);
			}
		}

		/* 自动模式每次只运行一轮，随后重新检查串口退出请求。 */
		if(CurrentMode == MODE_TRACK)
		{
			Track_Run(TrackSensor_ReadState());
			Delay_ms(10);
		}
		else if(CurrentMode == MODE_AVOID)
		{
			Avoid_Run();
		}
		else
		{
			Delay_ms(1);
		}
	}
}
