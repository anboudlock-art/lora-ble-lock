#ifndef _COMMON_H_
#define _COMMON_H_ 

#include "ARMCM0.h"
#include "lib.h"

typedef struct
{
	uint8_t def_set_flag;
  uint8_t pass[3];        //Á¬½ÓÃÜÂë
  uint8_t lockstate;   		//¹Ø»úÊ±ËøµÄ×´Ì¬(¿ª/¹Ø)
	uint8_t bat_discharge_cnt;//µç³Ø·Åµç¼ÆÊý
}sys_config;

extern sys_config System_Config_Parameter;

#define NONE_WKP		0			//ÎÞ»½ÐÑÔ´
#define INDUCED_KEY	1			//¸ÐÓ¦°´¼ü»½ÐÑ
#define LOCK_STATE  2			//ËøÊÖ¶¯ÉÏËø»½ÐÑ

extern uint8_t WakeupFlag;

#endif

