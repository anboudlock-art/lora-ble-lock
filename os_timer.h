#ifndef _os_timer_H_
#define _os_timer_H_

#include "stdint.h"

#define OS_TIMER_MAX		9

#define	TIMER_NO_EVT 0x00000000
#define	TIMER_EVT_1  0x00000001
#define	TIMER_EVT_2  0x00000002
#define	TIMER_EVT_3  0x00000004
#define	TIMER_EVT_4  0x00000008
#define	TIMER_EVT_5  0x00000010
#define	TIMER_EVT_6  0x00000020
#define	TIMER_EVT_7  0x00000040
#define	TIMER_EVT_8  0x00000080
#define	TIMER_EVT_9  0x00000100
#define	TIMER_EVT_10  0x00000200
#define	TIMER_EVT_11  0x00000400
#define	TIMER_EVT_12  0x00000800
#define	TIMER_EVT_13  0x00001000
#define	TIMER_EVT_14  0x00002000
#define	TIMER_EVT_15  0x00004000
#define	TIMER_EVT_16  0x00008000
#define	TIMER_EVT_17  0x00010000
#define	TIMER_EVT_18  0x00020000


//ÏÈ¶¨ÒåÊÂ¼þ±êÖ¾Î»£¬ÔÙstart¶¨Ê±Æ÷
#define	 EVT_START_DEVICE			TIMER_EVT_1
#define	 EVT_ENTER_SLEEP			TIMER_EVT_2
#define	 EVT_AUTO_LOCK				TIMER_EVT_3
#define	 EVT_MOTOR_STOP				TIMER_EVT_4
#define	 EVT_BAT_DISCHARGE		TIMER_EVT_5
#define	 EVT_REDLED_CONTROL		TIMER_EVT_6
#define	 EVT_STATE_CHECK			TIMER_EVT_7
#define	 EVT_STOP_BLE					TIMER_EVT_8
#define	 EVT_BLUELED_CONTROL 	TIMER_EVT_9

extern volatile  uint32_t TIMER_EVENT;

extern void os_timer_query(void);
extern void OS_timer_start(uint32_t event, uint32_t interval, uint8_t repeat);
extern void OS_timer_stop(uint32_t event);
extern void OS_timer_SetEvt(uint32_t event);

extern void Stop_Timer_All(void);


#endif
