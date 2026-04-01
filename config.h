#ifndef _CONFIG_H_
#define _CONFIG_H_ 

#include "ARMCM0.h"
#include "lib.h"

#define _SYD_RTT_DEBUG_

#define USER_32K_CLOCK_RCOSC


#define Device_Name 'L','o','c','k','_','0','0','0','0','0','0','0','0','0','0','0','0',
#define Device_Name_Len 17

#define	MAX_RX_LENGTH	1024
#define	MAX_TX_LENGTH	50


#define WAKEUP_PIN		BIT20	 //P011唤醒按键
#define WAKEUP_hall		BIT17	 //P011唤醒按键


// #define STATE_PIN			GPIO_13	 //锁粱上锁唤醒


//用户添加
#define	DISCHRG_PIN			GPIO_10     //电池放电控制脚

#define	PWR_4G			GPIO_18     //

#define	STATE_KEY				GPIO_13     //状态开关

#define	STOP_KEY2				GPIO_14     //停转开关2
#define	STOP_KEY1				GPIO_12    //停转开关1

#define	lock_hall				GPIO_17    //

#define	GPIO_USER_KEY	  GPIO_20     //用户按键


#define	Motor_INB				GPIO_11		//GPIO_16     //电机控制输入B
#define	Motor_INA				GPIO_15     //电机控制输入A


#define	LED_BLUE				GPIO_7    //绿色指示灯
#define	LED_RED					GPIO_9    //红色指示灯
#define	LED_green				GPIO_8    //指示灯	

//260101+
#define	WDG_OUT				GPIO_31    // FEED DOG 
#define WDG_OUT_H()		GPIO_Pin_Set(U32BIT(WDG_OUT))
#define WDG_OUT_L()		GPIO_Pin_Clear(U32BIT(WDG_OUT))

#define	SPCIO				GPIO_27    // 
#define SPCIO_H()		GPIO_Pin_Set(U32BIT(SPCIO))
#define SPCIO_L()		GPIO_Pin_Clear(U32BIT(SPCIO))

typedef struct{
	uint8_t data[MAX_RX_LENGTH];
	uint32_t header,tail;
} Uart_Rx_Buf_t;

typedef struct{
	uint8_t  data[MAX_TX_LENGTH][MAX_ATT_DATA_SZ];
	uint32_t header,tail;
} Uart_Tx_Buf_t;

extern Uart_Rx_Buf_t uart_rx_buf;
extern Uart_Tx_Buf_t uart_tx_buf;

#define  EVT_NUM  ((uint8_t) 0x02)//当前定时器事件数

#define  EVT_2S   ((uint32_t)    0x00000100)//定时器2s事件
#define  EVT_1S_OTA   ((uint32_t)    0x00000200)//定时器2s事件

//2021.04.20添加
//#define  EVT_TIMBASE   ((uint32_t)    0x00000400)//定时器10ms事件

#define  RTCEVT_NUM  ((uint8_t) 0x02)				//RTC定时器事件数

//#define  RTCEVT_10S  ((uint32_t)  0x00000001)//定时器1s事件
/*整分钟定时器事件，因为其余的事件追求的是时间的等长，但是在手环中还需要一个分钟的定时器，每次60秒的时候需要一个定时器事件，
比如60秒,120秒，180秒等，概况起来就是整（正）分钟*/
//#define  RTCEVT_whole_minute  ((uint32_t)  0x00000002)
#define  RTCEVT_185S  ((uint32_t) 0x0000004)//定时器185s事件
#define  RTCEVT_10S  ((uint32_t) 0x0000002)//定时器185s事件

// #define  RTCEVT_72h  ((uint32_t) 0x0000008)//定时器72h事件  //取消

// #define	RTCEVT_24h 	 ((uint32_t) 0x0000002)//定时器24h事件//取消

extern void BLSetConnectionUpdate(uint8_t target);

#endif

