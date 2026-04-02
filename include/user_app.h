#ifndef _USER_APP_H
#define _USER_APP_H

#include <stdint.h>

//唤醒源
#define  NONE  				0				//无唤醒源
#define  USER_BUTTON  0x01		//按键唤醒
#define  LOCK_CHECK  	0x02		//自动上锁唤醒
#define  RTC_ALARM  	0x04		//RTC唤醒
#define  RTC_4Greport  	0x08		//24小时4G上报
#define  lock_cut  	0x10		//剪断
//系统状态
#define	 SLEEP			 	0
//#define  ENTERSLEEP   0x80
#define  STARTUP   		0x40
#define  OTAMODE			0x80
#define	 POWERON			0x01
#define	 AUTOLOCK			0x02
#define	 BATDISCHG		0x04

#define DEVICE_NAME_LEN		10
#define DEVICE_NAME_REQ		("Smart Lock")	

#define APPEARANCE_LEN		(2)
#define APPEARANCE_REQ		("\x03\xC2")

#define MANUFACTURER_NAME_LEN		(17)
#define MANUFACTURER_NAME_REQ		("Manufacturer Name")

#define MODEL_NUMBER_LEN		(12)
#define MODEL_NUMBER_REQ		("Model Number")

#define SERIAL_NUMBER_LEN		(13)
#define SERIAL_NUMBER_REQ		("Serial Number")

#define HARDWARE_REVISION_LEN		(17)	
#define HARDWARE_REVISION_REQ		("Hardware Revision")		

#define FIRMWARE_REVISION_LEN		(17)		
#define FIRMWARE_REVISION_REQ		("Firmware Revision")			

#define SOFTWARE_REVISION_LEN		(9)		
#define SOFTWARE_REVISION_REQ		("CLOCKV500")	

#define PNP_ID_LEN		(7)		
#define PNP_ID_REQ		("\x01\x17\x27\xb0\x32\x10\x24")

#define SYSTEM_ID_LEN	(8)
#define SYSTEM_ID_REQ	("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")

#define BLE_VendorV1_Notify_User_Description  0x0026
#define VENDORV1_NOTIFY_DESCRIPTION_LEN	(16)
#define VENDORV1_NOTIFY_DESCRIPTION_REQ	("Vendor V1 Notify")

#define BLE_VendorV1_Write_User_Description  0x002A
#define VENDORV1_WRITE_DESCRIPTION_LEN	(15)
#define VENDORV1_WRITE_DESCRIPTION_REQ	("Vendor V1 Write")

#define BLE_VendorV2_Notify_User_Description  0x002E
#define VENDORV2_NOTIFY_DESCRIPTION_LEN	(16)
#define VENDORV2_NOTIFY_DESCRIPTION_REQ	("Vendor V2 Notify")

#define BLE_VendorV2_Write_User_Description  0x0032
#define VENDORV2_WRITE_DESCRIPTION_LEN	(15)
#define VENDORV2_WRITE_DESCRIPTION_REQ	("Vendor V2 Write")

//============================================================================
// 密码存储相关定义 (新增)
//============================================================================
#define PASSWORD_FLASH_ADDR    0x0F0000    // 密码存储在SPI Flash的地址 (1MB位置)
#define PASSWORD_VALID_FLAG    0xAA        // 密码有效标志

// 密码存储结构
typedef struct {
    uint8_t valid_flag;        // 有效标志 (0xAA表示有效)
    uint8_t password[6];       // 6位密码
    uint8_t checksum;          // 校验和
} Password_Storage_t;

//============================================================================

//系统状态标志
extern uint8_t Systerm_States;

extern uint8_t EnterSleepFlag;

//notify打开标志
extern uint8_t start_tx;	//允许notify标志
//extern uint8_t gatt_Recive_flag;
extern uint8_t gatt_buff[20];

// 连接id
extern uint8_t connect_flag;	//连接成功标志

extern uint8_t WakeupSource;	//唤醒源标志

typedef struct
{
	uint8_t default_id;
  uint8_t password[3];    //连接密码
  uint8_t lockstate;   		//关机时锁的状态(开/关)
	uint8_t discharge_flag;//电池放电计数
} SystemParameter_Def;

//extern SystemParameter_Def SystemParameter;

extern void aes_test(void);

extern uint8_t gatt_read_AppCB(uint8_t *p_value, uint8_t len);
extern void GATT_ReadWrite_Process(void);

extern void SetAllParaDefault(void);
extern void ConfigParameterDefault(void);

extern void user_wakeup(void);
//extern void user_init(void);
extern void user_task(void);

extern void user_encrypt_notify(const unsigned char* Key, uint8_t *rawDataBuf, uint16_t rawDatalength);

extern void WakupConfig_BeforeSleep(void);

extern void test_wakeup(void);

extern void hardware_init(void);

extern void ota_connect_evnet(void);

extern void gap_connect_evnet(void);
extern void gap_disconnect_evnet(void);

extern void Enable_Timer2_2ms(void);

extern void FeedDog(void);//260101

//============================================================================
// 密码管理函数声明 (新增)
//============================================================================
extern void password_init(void);                              // 密码初始化
extern int password_verify(const uint8_t *pwd);               // 密码验证
extern int password_set(const uint8_t *pwd);                  // 设置新密码
extern int password_set_single_byte(uint8_t pwd_byte);        // 单字节设置密码
extern void password_reset(void);                             // 重置为默认密码
//============================================================================

//============================================================================
// BLE 指令处理函数声明 (从 4G-BLE 版本整合)
//============================================================================
extern uint8_t gatt_read_AppCB(uint8_t *p_data, uint8_t pdata_len);  // BLE 指令处理
extern void GATT_ReadWrite_Process(void);                             // GATT 读写处理
//============================================================================

#endif  // _USER_TASK_H

