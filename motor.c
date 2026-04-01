/******************** (C) COPYRIGHT 2020  *****************************
* File Name          : motor.c
* Author             : 
* Version            : V1.00
* Date               : 2020.12.13
* Description        : Motor module processe
*******************************************************************************
History:
	2020-12	First version.
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "motor.h"
#include "config.h"
#include "user.h"

#define 	MOTOREN		//Ê¹ÄÜ¿ØÖÆ

uint8_t MotorState = MOTOR_STOP;

/*MOTORENÂí´ïÊ¹ÄÜ£¬DEBUG´®¿ÚÊä³ö´òÓ¡Ê¹ÄÜ¡£Âí´ïÊ¹ÄÜ»áÊ¹µ¥Æ¬»ú¸´Î»*/

/* Private variables ---------------------------------------------------------*/
void MotorInit(void)
{
	MotorState = MOTOR_STOP;
	
	// ÉèÖÃÎªÉÏÀ­Êä³ö
//	GPIO_Set_Output(Motor_INA|Motor_INB);
	
	//³õÊ¼»¯Êä³ö
	//gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_4,1);
	GPIO_Pin_Clear(U32BIT(Motor_INA));
	GPIO_Pin_Clear(U32BIT(Motor_INB));
	
}

void MotorStartForward(void)
{
	flg_do_closelockOK = 0;
	flg_do_closelockcmd = 0;

  MotorState = MOTOR_FORWARD;
#ifdef MOTOREN 
	GPIO_Pin_Clear(U32BIT(Motor_INA));
	GPIO_Pin_Set(U32BIT(Motor_INB));
#endif 
	
}

void MotorStartReverse(void)
{
	flg_do_closelockcmd = 1;
	_user.closelock_2s = 1000/2;	//ÔÝ1Ãë
  MotorState = MOTOR_REVERSE;
#ifdef MOTOREN 
	GPIO_Pin_Clear(U32BIT(Motor_INB));
	GPIO_Pin_Set(U32BIT(Motor_INA));
#endif
  
}

void MotorStop(void)
{
	MotorState = MOTOR_STOP;
#ifdef MOTOREN 
	GPIO_Pin_Clear(U32BIT(Motor_INA));
	GPIO_Pin_Clear(U32BIT(Motor_INB));
#endif

}

void MotorEventHandle(void)
{

}


/********************** (C) COPYRIGHT 2020-12-13 ***********************/

