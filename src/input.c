/******************** (C) COPYRIGHT 2020  *****************************
* File Name          : input.c
* Author             : 
* Version            : V1.00
* Date               : 2020.12.13
* Description        : input detect processe (LoRa Version)
*******************************************************************************
History:
	2020-12	First version.
	2026-03 Modified for LoRa version
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "delay.h"
#include "gpio.h"
#include "user.h"
#include "config.h"
// LoRa版本：移除4G模块头文件
#include "os_timer.h"
#include "led.h"
#include "battery.h"

//端口定义
//#define 	STOP_KEY1		GPIO_0
//#define 	STOP_KEY2		GPIO_0
//#define 	State_KEY		GPIO_0

uint8_t DoorState = 0;

uint8_t StateOld=0;
uint8_t StopkeyOld=0;
uint8_t UserkeyOld=0;

void GetDoorStateInit(void)
{
	DoorState = GetInputStopR();
	StateOld = DoorState;
}
	
uint8_t GetDoorState(void)
{
	return DoorState;
}

uint8_t GetInputStopR(void)
{
	return GPIO_Pin_Read(U32BIT(STOP_KEY1))&&GPIO_Pin_Read(U32BIT(STOP_KEY2));
}

void hall_check(void)
{
	uint8_t state_change = 0;
	
	if(GPIO_Pin_Read(U32BIT(lock_hall)))
	{
		//剪断报警
		OS_timer_start(EVT_CUT_ALARM, 2, 0);
	}
}

void input_init(void)
{
	//配置输入引脚
	GPIO_Pin_Config(U32BIT(STOP_KEY1)|U32BIT(STOP_KEY2)|U32BIT(STATE_KEY)|U32BIT(lock_hall), GPIO_INPUT, GPIO_PULL_UP);
	GPIO_Pin_Config(U32BIT(GPIO_USER_KEY), GPIO_INPUT, GPIO_PULL_UP);
}

void input_scan(void)
{
	uint8_t StopKeyState = 0;
	uint8_t UserKeyState = 0;
	
	StopKeyState = GetInputStopR();
	UserKeyState = GPIO_Pin_Read(U32BIT(GPIO_USER_KEY));
	
	if(StopKeyState != StopkeyOld)
	{
		StopkeyOld = StopKeyState;
		if(StopKeyState)
		{
			DoorState = 1;
		}
		else
		{
			DoorState = 0;
		}
	}
	
	if(UserKeyState != UserkeyOld)
	{
		UserkeyOld = UserKeyState;
	}
}
