/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "os_timer.h"
#include "led.h"

#include "config.h"
#include "delay.h"

#define 	LED_EN		//Ê¹ÄÜ¿ØÖÆ

// LED Red ¡ª¡ª> LED2¹Ü½Å

//led ºìµÆ	PD5
//#define		LED_RED				GPIO_0
//#define		LED_BLUE			GPIO_0

//·ÅµçÊ¹ÄÜ¹Ü½Å PC7
//#define		DISCHRG_PIN		GPIO_0
	


//LED_STATUS ledstatus = ALL_LED_OFF;

LED_STATUS redstatus = RED_LED_OFF;
LED_STATUS bluestatus = BLUE_LED_OFF;

uint8_t dischrg_flag = 0;

//void led_set_status(LED_STATUS newStatus)
//{
//	ledstatus = newStatus;	
//}

void RedLed_Config(LED_STATUS newStatus)
{
	redstatus = newStatus;	
}

void BlueLed_Config(LED_STATUS newStatus)
{
	bluestatus = newStatus;	
}

/******************************************************************************      
*name:             LedInit      
*introduce:        led³õÊ¼»¯    
*parameter:        none     
*return:           none    
*author:              
*changetime:       2020.12.13    
******************************************************************************/    
void LedInit(void)    
{ 
	//µç³Ø·Åµç¿ØÖÆ¿Ú
//	GPIO_Set_Output(DISCHRG_PIN|LED_RED|LED_BLUE);
//	PIN_Pullup_Enable(T_QFN_48, DISCHRG_PIN|LED_RED|LED_BLUE);
	DischargeOff();	
	dischrg_flag = 0;	
	
	//¹Ø±ÕËùÓÐµÆ
	RedLedOff();
	BlueLedOff();

  //ºìÀ¶µÆÍ¬Ê±ÁÁ400ms(ÉÏµç×Ô¼ì)
  //led_set_status(ALL_LED_OFF);
} 

void LedBlink(void)
{
	RedLedOn();
	BlueLedOn();
	
	DelayMS(300);
	
	RedLedOff();
	BlueLedOff();
}




void redledblink3(void){
	
	RedLedOff();
	DelayMS(300);
  RedLedOn();
	
	DelayMS(300);
	RedLedOff();

	DelayMS(300);
  RedLedOn();
	
	DelayMS(300);
	RedLedOff();

 	DelayMS(300);
  RedLedOn();
	
	DelayMS(300);
	RedLedOff(); 

}

void LedBlink_Fast(void)
{
	OS_timer_stop(EVT_REDLED_CONTROL);
	//OS_timer_stop(EVT_BLUELED_CONTROL);
	RedLedOn();
	delay_ms(300);
	RedLedOff();
}

void RedLedEventHandle(void)
{ 
  switch(redstatus)
  {
    //ºìÀ¶µÆÍ¬Ê±ÁÁ400ms(ÉÏµç×Ô¼ì)
//    case ALL_ON_400MS_1:
//      RedLedOn();
//      BlueLedOn();
//      ledstatus = ALL_ON_400MS_2;
//			OS_timer_start(EVT_LED_CONTROL, 40, false);
//      break;
//    case ALL_ON_400MS_2:
//      RedLedOff();
//      BlueLedOff();
//      break;  
    
    //ºìµÆÁÁ200ms(¶Ì°´¼üµÆ)
    case RED_ON_200MS_1:
      RedLedOn();
      redstatus = RED_ON_200MS_2;
			OS_timer_start(EVT_REDLED_CONTROL, 20, false);
      break;
    case RED_ON_200MS_2:
      RedLedOff();
			OS_timer_stop(EVT_REDLED_CONTROL);
      break;  
        
    //À¶µÆÁÁ200ms(³¤°´¼üµÆ)   
//    case BLUE_ON_200MS_1:
//      RedLedOff();
//      BlueLedOn();
//      ledstatus = BLUE_ON_200MS_2;
//			OS_timer_start(EVT_LED_CONTROL, 20, false);
//      break;
//    case BLUE_ON_200MS_2:
//      BlueLedOff();
//      break;    
      
    //À¶µÆÁÁ2s(°´¼ü¿ªËø³É¹¦Ö¸Ê¾µÆ)   
//    case BLUE_ON_2S_1:
//      RedLedOff();
//      BlueLedOn();
//      ledstatus = BLUE_ON_2S_2;
//			OS_timer_start(EVT_LED_CONTROL, 200, false);
//      break;
//    case BLUE_ON_2S_2:
//      BlueLedOff();
//      break;  
    
    //ºìµÆ³ÖÐø½»ÌæÉÁË¸(¹ã²¥Ö¸Ê¾µÆ)     
    case RED_SLOW_FLASH_1:
      RedLedOn();
      redstatus = RED_SLOW_FLASH_2;
			OS_timer_start(EVT_REDLED_CONTROL, 100, false);
      break;  
    case RED_SLOW_FLASH_2:
      RedLedOff();
      redstatus = RED_SLOW_FLASH_1;
			OS_timer_start(EVT_REDLED_CONTROL, 100, false);
      break;  
     
    //À¶µÆ³ÖÐøÂýÉÁ(À¶ÑÀÒÑÁ¬½ÓÖ¸Ê¾µÆ) 
//    case BLUE_SLOW_FLASH_1:
//      //RedLedOff();
//      BlueLedOn();
//      ledstatus = BLUE_SLOW_FLASH_2;
//			OS_timer_start(EVT_LED_CONTROL, 100, false); 
//      break;  
//    case BLUE_SLOW_FLASH_2:
//      BlueLedOff();
//      ledstatus = BLUE_SLOW_FLASH_1;
//			OS_timer_start(EVT_LED_CONTROL, 100, false);
//      break; 
   
    //À¶µÆ¿ìÉÁÈýÏÂ(½øÈë°´¼ü¿ªËøÄ£Ê½Ö¸Ê¾µÆ)
//    case  BLUE_THREE_FLASH_1:
//      //RedLedOff();
//      BlueLedOn();
//      ledstatus = BLUE_THREE_FLASH_2;
//			OS_timer_start(EVT_LED_CONTROL, 50, false);
//      break;  
//    case  BLUE_THREE_FLASH_2:
//      BlueLedOff();
//      ledstatus = BLUE_THREE_FLASH_3;
//			OS_timer_start(EVT_LED_CONTROL, 50, false);
//      break;       
//    case  BLUE_THREE_FLASH_3:
//      BlueLedOn();
//      ledstatus = BLUE_THREE_FLASH_4;
//			OS_timer_start(EVT_LED_CONTROL, 50, false);
//      break;  
//    case  BLUE_THREE_FLASH_4:
//      BlueLedOff();
//      ledstatus = BLUE_THREE_FLASH_5;
//			OS_timer_start(EVT_LED_CONTROL, 50, false);
//      break;          
//    case  BLUE_THREE_FLASH_5:
//      BlueLedOn();
//      ledstatus = BLUE_THREE_FLASH_6;
//			OS_timer_start(EVT_LED_CONTROL, 50, false);
//      break;  
//    case  BLUE_THREE_FLASH_6:
//      BlueLedOff();
//      ledstatus = ALL_LED_OFF;
//			OS_timer_stop(EVT_LED_CONTROL);
//      break;       
    
    //ºìµÆ¿ìÉÁÈýÏÂ(°´¼ü¿ªËøÊ§°ÜÖ¸Ê¾µÆ)
    case  RED_THREE_FLASH_1:
      RedLedOn();
      redstatus = RED_THREE_FLASH_2;
			OS_timer_start(EVT_REDLED_CONTROL, 10, false);
      break;  
    case  RED_THREE_FLASH_2:
      RedLedOff();
      redstatus = RED_THREE_FLASH_3;
			OS_timer_start(EVT_REDLED_CONTROL, 20, false);
      break;       
    case  RED_THREE_FLASH_3:
      RedLedOn();
      redstatus = RED_THREE_FLASH_4;
			OS_timer_start(EVT_REDLED_CONTROL, 10, false);
      break;  
    case  RED_THREE_FLASH_4:
      RedLedOff();
      redstatus = RED_THREE_FLASH_5;
			OS_timer_start(EVT_REDLED_CONTROL, 20, false);
      break;          
    case  RED_THREE_FLASH_5:
      RedLedOn();
      redstatus = RED_THREE_FLASH_6;
			OS_timer_start(EVT_REDLED_CONTROL, 10, false);
      break;  
    case  RED_THREE_FLASH_6:
      RedLedOff();
      redstatus = RED_LED_OFF;
			OS_timer_stop(EVT_REDLED_CONTROL);
      break;       
     
    //ºìµÆ³ÖÐøÂýÉÁ(ÏµÍ³µçÁ¿µÍÖ¸Ê¾µÆ) 
    case RED_FAST_FLASH_1:
      RedLedOn();
      redstatus = RED_FAST_FLASH_2;
			OS_timer_start(EVT_REDLED_CONTROL, 20, false);
      break;  
    case RED_FAST_FLASH_2:
      RedLedOff();
      redstatus = RED_FAST_FLASH_1;
			OS_timer_start(EVT_REDLED_CONTROL, 40, false);
      break;  
		
		//ºìµÆµãÁÁ1S
		case RED_SLOW_FLASH_ONE_1:
			RedLedOn();
      redstatus = RED_SLOW_FLASH_ONE_2;
			OS_timer_start(EVT_REDLED_CONTROL, 100, false);
			break;		
		case RED_SLOW_FLASH_ONE_2:
			RedLedOff();
      redstatus = RED_LED_OFF;
			OS_timer_stop(EVT_REDLED_CONTROL);
			break;
		
//		case BLUE_SLOW_FLASH_ONE_1:
//			RedLedOff();
//      BlueLedOn();
//      ledstatus = BLUE_SLOW_FLASH_ONE_2;
//			OS_timer_start(EVT_LED_CONTROL, 100, false);
//			break;	
//		case BLUE_SLOW_FLASH_ONE_2:
//			RedLedOff();
//      BlueLedOff();
//      ledstatus = ALL_LED_OFF;
//			OS_timer_stop(EVT_LED_CONTROL);
//			break;
      
    //ËùÓÐµÆÃð 
    case RED_LED_OFF:
      RedLedOff();
			OS_timer_stop(EVT_REDLED_CONTROL);
      break;    
        
    default:
      break;
	} 
}

void BlueLedEventHandle(void)
{
		switch(bluestatus)
  {       
    //À¶µÆÁÁ200ms(³¤°´¼üµÆ)   
    case BLUE_ON_200MS_1:
      BlueLedOn();
      bluestatus = BLUE_ON_200MS_2;
			OS_timer_start(EVT_BLUELED_CONTROL, 20, false);
      break;
    case BLUE_ON_200MS_2:
      BlueLedOff();
			bluestatus = BLUE_LED_OFF;
			OS_timer_stop(EVT_BLUELED_CONTROL);
      break;    
      
    //À¶µÆÁÁ2s(°´¼ü¿ªËø³É¹¦Ö¸Ê¾µÆ)   
    case BLUE_ON_2S_1:
      BlueLedOn();
      bluestatus = BLUE_ON_2S_2;
			OS_timer_start(EVT_BLUELED_CONTROL, 200, false);
      break;
    case BLUE_ON_2S_2:
      BlueLedOff();
			bluestatus = BLUE_ON_2S_2;
      break;   
     
    //À¶µÆ³ÖÐøÂýÉÁ(À¶ÑÀÒÑÁ¬½ÓÖ¸Ê¾µÆ) 
    case BLUE_SLOW_FLASH_1:
      BlueLedOn();
      bluestatus = BLUE_SLOW_FLASH_2;
			OS_timer_start(EVT_BLUELED_CONTROL, 100, false); 
      break;  
    case BLUE_SLOW_FLASH_2:
      BlueLedOff();
      bluestatus = BLUE_SLOW_FLASH_1;
			OS_timer_start(EVT_BLUELED_CONTROL, 100, false);
      break; 
   
    //À¶µÆ¿ìÉÁÈýÏÂ(½øÈë°´¼ü¿ªËøÄ£Ê½Ö¸Ê¾µÆ)
    case  BLUE_THREE_FLASH_1:
      BlueLedOn();
      bluestatus = BLUE_THREE_FLASH_2;
			OS_timer_start(EVT_BLUELED_CONTROL, 10, false);
      break;  
    case  BLUE_THREE_FLASH_2:
      BlueLedOff();
      bluestatus = BLUE_THREE_FLASH_3;
			OS_timer_start(EVT_BLUELED_CONTROL, 20, false);
      break;       
    case  BLUE_THREE_FLASH_3:
      BlueLedOn();
      bluestatus = BLUE_THREE_FLASH_4;
			OS_timer_start(EVT_BLUELED_CONTROL, 10, false);
      break;  
    case  BLUE_THREE_FLASH_4:
      BlueLedOff();
      bluestatus = BLUE_THREE_FLASH_5;
			OS_timer_start(EVT_BLUELED_CONTROL, 20, false);
      break;          
    case  BLUE_THREE_FLASH_5:
      BlueLedOn();
      bluestatus = BLUE_THREE_FLASH_6;
			OS_timer_start(EVT_BLUELED_CONTROL, 10, false);
      break;  
    case  BLUE_THREE_FLASH_6:
      BlueLedOff();
      bluestatus = BLUE_LED_OFF;
			OS_timer_stop(EVT_BLUELED_CONTROL);
      break;             
		
		case BLUE_SLOW_FLASH_ONE_1:
      BlueLedOn();
      bluestatus = BLUE_SLOW_FLASH_ONE_2;
			OS_timer_start(EVT_BLUELED_CONTROL, 30, false);
			break;	
		case BLUE_SLOW_FLASH_ONE_2:
      BlueLedOff();
      bluestatus = BLUE_LED_OFF;
			OS_timer_stop(EVT_BLUELED_CONTROL);
			break;
      
    //ËùÓÐµÆÃð 
    case BLUE_LED_OFF:
      BlueLedOff();
			OS_timer_stop(EVT_BLUELED_CONTROL);
      break;    
        
    default:
      break;
	} 
}

void AllLedOff(void)
{
	redstatus = RED_LED_OFF;
	bluestatus = BLUE_LED_OFF;
	RedLedOff();
  BlueLedOff();
  greenLedOff();  
	OS_timer_stop(EVT_REDLED_CONTROL);
	OS_timer_stop(EVT_BLUELED_CONTROL);
}

void BatDischargeOn(void)
{
	DischargeOn();
}

void BatDischargeOff(void)
{
	DischargeOff();
}

/********************** (C) COPYRIGHT 2020-12-13 ***********************/
