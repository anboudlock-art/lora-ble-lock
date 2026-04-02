/********************************************************************************
** ÎÄ¼þÃû³Æ: Common.h
** Ãè    Êö: Í¨ÓÃº¯Êý³ÌÐòÍ·ÎÄ¼þ
** ´´½¨×÷Õß: 
** ½¨Á¢ÈÕÆÚ: 
** °æ±¾±ä¸ü: 

********************************************************************************/
#include "config.h"

#ifndef __USER1_H
#define __USER1_H

/**********************************Í·ÎÄ¼þ°üº¬***********************************/
#include "Typedef.h"
#include "gpio.h"
#include "config.h"
#include "GPS.h"
#define flg_200ms		 flag_00.bitn.bit0    //
#define flg_poweron flag_00.bitn.bit1    
#define flg_ledflash flag_00.bitn.bit2
#define flg_clr_bc95        flag_00.bitn.bit5//ÇåBC95 bufÇø
#define flg_10ms            flag_00.bitn.bit6
#define flg_flashwrite      flag_00.bitn.bit7

	


//_user.flag  ÒÔÏÂ
//_user.flag00
#define	flg_do_closelockcmd			_user.flag00.bitn.bit0	//·¢¹ØËøÖ¸Áî
#define	flg_do_closelockOK			_user.flag00.bitn.bit1	//¹ØËøÖ¸ÁîOK
#define flg_err_cutalarm			_user.flag00.bitn.bit2
#define flg_needred3				_user.flag00.bitn.bit3	//ÐèÉÁ3´Î
#define flg_cutup					_user.flag00.bitn.bit4

#define	flg_eepOK					_user.flag00.bitn.bit11
#define	flg_havecard				_user.flag00.bitn.bit12
#define flg_openlock				_user.flag00.bitn.bit13





//_user.flag01
// #define flg_cmd_4glocation		_user.flag01.bitn.bit0		//4G»ùÕ¾¶¨Î»	
#define flg_resetsystem			_user.flag01.bitn.bit1
#define	flg_first_logon 		_user.flag01.bitn.bit2
#define flg_CIPSEND			_user.flag01.bitn.bit3	
#define flg_4g_EN			_user.flag01.bitn.bit4	


#define flg_reset			_user.flag01.bitn.bit5	
#define flg_setipOK			_user.flag01.bitn.bit6	
#define flg_onlysendonce	_user.flag01.bitn.bit7

//ÒòÄ£Ê½ÓÐÇ°µ¼Âë£¬¹Ê½«4GÏà¹ØÃüÁîµ¥¶À¶¨Òå±äÁ¿Ìå
//_user.flag_cmd
#define	flg_cmd_logon				_user.flag_cmd.bitn.bit0
#define flg_cmd_reportsleepset 		_user.flag_cmd.bitn.bit1
#define flg_cmd_lockmessage			_user.flag_cmd.bitn.bit2
#define flg_cmd_sealmessage			_user.flag_cmd.bitn.bit3
#define flg_cmd_unsealmessage		_user.flag_cmd.bitn.bit4
// #define flg_cmd_heat				_user.flag_cmd.bitn.bit5
#define flg_cmd_gpsdata				_user.flag_cmd.bitn.bit6
#define flg_cmd_reportlockstatus	_user.flag_cmd.bitn.bit7
#define flg_cmd_reportseal			_user.flag_cmd.bitn.bit8//Ê©·â
#define flg_cmd_reportsenddelay 	_user.flag_cmd.bitn.bit9
#define flg_cmd_sendseal			_user.flag_cmd.bitn.bit10




#define D_wakeuptime 	net4G.sleep_delayset * 60		//5*60		//ÐÝÃß15·ÖÖÓ
/*********************************Êý¾ÝÀàÐÍ¶¨Òå**********************************/
typedef union
{
	u8		data8[8];	
	u16		data16[4];	
}lock_id_t;


typedef struct{
	u8 head[2];
	lock_id_t lock_id;
	u8 key_code[6];
	u8 IP[4];
	u16 port;
}flash_save_t;


typedef union{
	flash_save_t savedata;
	// u16 eepdata_16[9+3];
	u8 eepdata_8[12*2];
}eep_save_t;

typedef union {
	uint8_t array[16+12];
	
	struct{
		char CIMI[16];
		char otg_code[4];
		uint8_t LVD_bat;
		uint8_t weekuptime_set;
		uint8_t IP[4];
		uint16_t port;
		uint8_t factory_mode;		
	}member;
}param_t;

typedef enum
{
	status_close =0,		//ÂäËø
	status_open,			//¿ªËø
}lock_status_t;

typedef enum{
  led_mode_alloff = 0,
  led_mode_buleon,
  led_mode_buleflash,
  led_mode_buleflashfast,  
  led_mode_buleflash2buleon,
  led_mode_redon,
  led_mode_redflash,
  led_mode_redflash2buleon,
}led_mode_t;


typedef struct{
  u8 led_flashcnt;
  u8 led_errflashcnt;
  led_mode_t led_mode;
	
	
  struct gap_ble_addr ble_ID;
  char mac_ascii[17];//¸ñÊ½Îª00:27:15:64:63:ed

 uint8_t bc95_reset_time;

 u16 powerondelay1s;//ÉÏµçÑÓÊ±1Ãë

 param_t param;

 u32 timestamp;
 type32_t lock_SN;//8Î»ËøºÅÓÃu32
 u16 lock_ID[2];
 lock_id_t lock_sn8array;//Ëøsn
 lock_status_t lock_status;//Ëø×´Ì¬
 u8 bat_volt;
 u16 wakeuptime;
 uint16_bit_t flag00;
 uint16_bit_t flag01;
 uint16_bit_t flag_cmd;

 u8 key_7cnt;
 u8 key_7delay;
 u16 closelock_2s;//ÉÏËøºó2sÄÚ¼ì²âµ½Ëø³É¹¦²ÅÄÜ±¨¾¯
}user_object_t;


extern user_object_t _user;
extern byte_bit_t flag_00;
extern byte_bit_t flag_01;
extern u8 _flash_write;
extern eep_save_t eep_save;


extern const char hex2ascii_tab[];
extern unsigned char SignatureBuf[33];
/***********************************º¯ÊýÉùÃ÷************************************/

#endif


