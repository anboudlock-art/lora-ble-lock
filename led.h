/**
 * Copyright (c) 2020, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */

#ifndef LED_H
#define LED_H

#include <stdint.h>

typedef enum
{
  //ºìÀ¶µÆÍ¬Ê±ÁÁ400ms(ÉÏµç×Ô¼ì)
  ALL_ON_400MS_1 = 0,
  ALL_ON_400MS_2,
  //ºìµÆ³ÖÐø½»ÌæÉÁË¸(¹ã²¥Ö¸Ê¾µÆ)
  RED_SLOW_FLASH_1,
  RED_SLOW_FLASH_2,
  //À¶µÆ³ÖÐøÂýÉÁ(À¶ÑÀÒÑÁ¬½ÓÖ¸Ê¾µÆ) 
  BLUE_SLOW_FLASH_1,
  BLUE_SLOW_FLASH_2,
  //ºìµÆÁÁ200ms(¶Ì°´¼üµÆ)
  RED_ON_200MS_1,
  RED_ON_200MS_2,
  //À¶µÆÁÁ200ms(³¤°´¼üµÆ) 
  BLUE_ON_200MS_1,
  BLUE_ON_200MS_2,
  //À¶µÆÁÁ2s(°´¼ü¿ªËø³É¹¦Ö¸Ê¾µÆ)
  BLUE_ON_2S_1,
  BLUE_ON_2S_2,
  //À¶µÆ¿ìÉÁÈýÏÂ(½øÈë°´¼ü¿ªËøÄ£Ê½Ö¸Ê¾µÆ)
  BLUE_THREE_FLASH_1,
  BLUE_THREE_FLASH_2,
  BLUE_THREE_FLASH_3,
  BLUE_THREE_FLASH_4,  
  BLUE_THREE_FLASH_5,
  BLUE_THREE_FLASH_6,  
  //ºìµÆ¿ìÉÁÈýÏÂ(°´¼ü¿ªËøÊ§°ÜÖ¸Ê¾µÆ)
  RED_THREE_FLASH_1,
  RED_THREE_FLASH_2,
  RED_THREE_FLASH_3,
  RED_THREE_FLASH_4,  
  RED_THREE_FLASH_5,
  RED_THREE_FLASH_6,
  //ºìµÆ³ÖÐø¿ìÉÁ(ÏµÍ³µçÁ¿µÍÖ¸Ê¾µÆ) 
  RED_FAST_FLASH_1,
  RED_FAST_FLASH_2,
	
	RED_SLOW_FLASH_ONE_1,
	RED_SLOW_FLASH_ONE_2,
	
	BLUE_SLOW_FLASH_ONE_1,
	BLUE_SLOW_FLASH_ONE_2,
	
	RED_LED_OFF,
	BLUE_LED_OFF,
	
  //ËùÓÐµÆÏ¨Ãð
  ALL_LED_OFF,
}LED_STATUS;

//extern LED_STATUS redstatus;
//extern LED_STATUS bluestatus;

extern uint8_t dischrg_flag;
#define 	greenLedOn()			GPIO_Pin_Set(U32BIT(LED_green))
#define 	greenLedOff()			GPIO_Pin_Clear(U32BIT(LED_green))

#define 	BlueLedOn()			GPIO_Pin_Set(U32BIT(LED_BLUE))
#define 	BlueLedOff()		GPIO_Pin_Clear(U32BIT(LED_BLUE))

#define 	RedLedOn()			GPIO_Pin_Set(U32BIT(LED_RED))
#define 	RedLedOff()			GPIO_Pin_Clear(U32BIT(LED_RED))

#define 	DischargeOn()		GPIO_Pin_Set(U32BIT(DISCHRG_PIN))
#define 	DischargeOff()	GPIO_Pin_Clear(U32BIT(DISCHRG_PIN))
 
/* Exported functions ------------------------------------------------------- */
void LedBlink(void);
void LedBlink_Fast(void);
void LedInit(void);
//void led_set_status(LED_STATUS newStatus);
void RedLed_Config(LED_STATUS newStatus);
void BlueLed_Config(LED_STATUS newStatus);
void AllLedOff(void);

void RedLedEventHandle(void);
void BlueLedEventHandle(void);

void BatDischargeOn(void);
void BatDischargeOff(void);

void redledblink3(void);

#endif
