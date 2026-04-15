/******************** (C) COPYRIGHT 2020  *****************************
* File Name          : input.c
* Author             : 
* Version            : V1.00
* Date               : 2020.12.13
* Description        : input detect processe
*******************************************************************************
History:
	2020-12	First version.
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "delay.h"
#include "gpio.h"
#include "user.h"
#include "config.h"
#include "lora_e220.h"
#include "os_timer.h"
#include "led.h"
#include "battery.h"
//�˿ڶ���
//#define 	STOP_KEY1		GPIO_0
//#define 	STOP_KEY2		GPIO_0
//#define 	State_KEY		GPIO_0

uint8_t DoorState = 0;

uint8_t StateOld=0;

//******************************************************************************    
//name:             DoorStateInit    
//introduce:        ��״̬����ʼ��  
//parameter:        none   
//return:           none  
//author:           none        
//email:            12345678@qq.com       
//changetime:       2020.04.14             
//******************************************************************************  
void DoorStateInit(void)
{
	GPIO_Set_Input(STOP_KEY1|STOP_KEY2|STATE_KEY, 0);					  //����Ϊ���룬��Ӧ�����벻ȡ��
	PIN_Pullup_Enable(T_QFN_48, STOP_KEY1|STOP_KEY2|STATE_KEY);	//��������
	
}

uint8_t GetInputStopF(void)
{
	uint8_t state=0;
	
  if(GPIO_Pin_Read(U32BIT(STOP_KEY1))) 
	{
		state = 1;
	}		
	else 
	{
		state = 0;
	}
		
  return state;
}

uint8_t GetInputStopR(void)
{
	uint8_t state=0;
    
	if(GPIO_Pin_Read(U32BIT(STOP_KEY2))) 
	{
		state = 1;
	}		
	else 
	{
		state = 0;
	}
	
  return state;
}

//��ȡ����״̬
uint8_t GetDoorState(void)
{
  //��ȡ�����ڲ�����
	uint8_t state=0;
	uint8_t StopF=0;
	uint8_t StopR=0;
	
	StopF = GetInputStopF();
	StopR = GetInputStopR();
	
  if((0==StopF) && (1==StopR))		//����
	{
		state = 1;
		// StateOld = state;
	}	
	if((1==StopF) && (0==StopR))		//����
	{
		state = 0;
		// StateOld = state;
	}
	if((1==StopF) && (1==StopR))		//����
	{
		state = 0;
		// StateOld = state;
	}

	if(state == 0){
		if(GPIO_Pin_Read(U32BIT(lock_hall)) == 0){	
			state = 0;//��ԭ���߼���ֻ��hallΪ�Ͳ�Ϊ����
			
		}
		else{
			state = 1;
		}	
	}

	if(StateOld != state){
		if(flg_4g_EN == 1){
			flg_cmd_gpsdata = 1;	
		}
	}
	StateOld = state;
  	
	return state;
}
//===============================================//

extern u8 lock_state;
extern u8 report_cnt;

//===============================================//
void hall_check(void)
{ //2ms
	static u16 err_cnt=0;
	static u16 err_cnt1 = 0;
	//===================================
	if(flg_do_closelockcmd){//��������

		lock_state = GetDoorState();
		if(_user.closelock_2s){
			_user.closelock_2s--;
			if(_user.closelock_2s==1){
				if(lock_state==0){//0Ϊ�����ɹ�
				}
				else{//1Ϊ����δ�ɹ�	�Զ�����
					if(1==GetInputStopF())
					{
						MotorStop();
						MotorStartForward();
						OS_timer_start(EVT_MOTOR_STOP, 200, false);	//4S ���Զ�ֹͣ
						OS_timer_SetEvt( EVT_STATE_CHECK );					
						OS_timer_start(EVT_ENTER_SLEEP, 200, false);	//4s
						flg_openlock = 1;
					}										
				}
			}
		}

		if(GPIO_Pin_Read(U32BIT(lock_hall)) == 0){
			if(++err_cnt1>1000/2){
				err_cnt1 = 0;
				flg_do_closelockcmd = 0;
				if(lock_state==0){
					flg_do_closelockOK = 1;
				}	
			}
		}
		else{
			err_cnt1 = 0;
		}	
	}//����end	


	//===================================
  if(flg_do_closelockOK == 1){
	 if(GPIO_Pin_Read(U32BIT(lock_hall))) 
	 {
			if(++err_cnt>=2000/2){

				flg_do_closelockOK = 0;
				
				if(flg_err_cutalarm ==0){
					flg_err_cutalarm = 1;
					flg_cmd_gpsdata = 1;
					report_cnt = 0;
						if(flg_4g_EN==0){
							RedLed_Config(RED_LED_OFF);
							lora_e220_init();
							UartEn(true);
							OS_timer_stop(EVT_ENTER_SLEEP);
							OS_timer_start( EVT_START_DEVICE, 2, false );
						}				  
				}
			}		
		}
		else{
			err_cnt = 0;
		}
	}		

}
/********************** (C) COPYRIGHT 2020-12-13 ***********************/

