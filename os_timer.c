#include "os_timer.h"
#include "DebugLog.h"

typedef struct 
{
	uint8_t enable;
	uint8_t repeat;
	uint32_t delay_cnt;
	uint32_t delay_interval;
	uint32_t event_flag;
} os_timer_str;

static volatile os_timer_str OS_Timer_Str[OS_TIMER_MAX];

volatile uint32_t TIMER_EVENT = 0;

//OS timer²éÑ¯ÊÂ¼þ
void os_timer_query(void)
{
	uint8_t i;
	for(i=0;i<OS_TIMER_MAX;i++)
	{
		if(OS_Timer_Str[i].enable)
		{
			if(OS_Timer_Str[i].delay_cnt>0)
			{
				OS_Timer_Str[i].delay_cnt--;
			}
			else 
			{
				//__disable_irq();
				TIMER_EVENT |= OS_Timer_Str[i].event_flag;
				//__enable_irq();
				
				//dbg_printf("Timer_Event:0x%08x\r\n", OS_Timer_Str[i].event_flag);
				if(OS_Timer_Str[i].repeat)
				{
					OS_Timer_Str[i].delay_cnt = OS_Timer_Str[i].delay_interval;
				}
				else
				{
					OS_Timer_Str[i].enable = 0;
				}
			}
		}
	}
}

//Æô¶¯Ò»¸öÊÂ¼þ¶¨Ê±Æ÷
void OS_timer_start(uint32_t event, uint32_t interval, uint8_t repeat)
{
	uint8_t index=0;
	
//	if(TIMER_EVENT & event)
//		TIMER_EVENT ^= event;
	
	for(index=0; index<OS_TIMER_MAX; index++)
	{
		if(event & (1<<index))
		{
			break;
		}
	}
	
	if(OS_Timer_Str[index].enable==0)
	{
		OS_Timer_Str[index].event_flag = event;
		OS_Timer_Str[index].delay_cnt = interval;
		OS_Timer_Str[index].delay_interval = interval;
		OS_Timer_Str[index].repeat = repeat;
		OS_Timer_Str[index].enable = 1;
	}
}

//Í£Ö¹Ò»¸öÊÂ¼þ¶¨Ê±Æ÷
void OS_timer_stop(uint32_t event)
{
	uint8_t index=0;
	
//	if(TIMER_EVENT & event)
//		TIMER_EVENT ^= event;
	
	for(index=0; index<OS_TIMER_MAX; index++)
	{
		if(event & (1<<index))
		{
			break;
		}
	}
	
	if(OS_Timer_Str[index].enable)
	{
		OS_Timer_Str[index].event_flag = TIMER_NO_EVT;
		OS_Timer_Str[index].delay_cnt = 0;
		OS_Timer_Str[index].delay_interval = 0;
		OS_Timer_Str[index].repeat = 0;
		OS_Timer_Str[index].enable = 0;
	}
}

void OS_timer_SetEvt(uint32_t event)
{
	//__disable_irq();
	TIMER_EVENT |= event;
	//__enable_irq();

}

void Stop_Timer_All(void)
{
	OS_timer_stop(EVT_START_DEVICE);
	OS_timer_stop(EVT_ENTER_SLEEP);
	OS_timer_stop(EVT_AUTO_LOCK);
	OS_timer_stop(EVT_MOTOR_STOP);
	OS_timer_stop(EVT_BAT_DISCHARGE);
	OS_timer_stop(EVT_REDLED_CONTROL);
	OS_timer_stop(EVT_STATE_CHECK);
	OS_timer_stop(EVT_STOP_BLE);
	OS_timer_stop(EVT_BLUELED_CONTROL);
	
}



