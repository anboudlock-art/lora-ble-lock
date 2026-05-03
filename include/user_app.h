#ifndef _USER_APP_H
#define _USER_APP_H

#include <stdint.h>

//����Դ
#define  NONE  				0				//�޻���Դ
#define  USER_BUTTON  0x01		//��������
#define  LOCK_CHECK  	0x02		//�Զ���������
#define  RTC_ALARM  	0x04		//RTC����
#define  RTC_4Greport  	0x08		//24Сʱ4G�ϱ�
#define  lock_cut  	0x10		//����
//ϵͳ״̬
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

//ϵͳ״̬��־
extern uint8_t Systerm_States;

extern uint8_t EnterSleepFlag;

//notify�򿪱�־
extern uint8_t start_tx;	//����notify��־
//extern uint8_t gatt_Recive_flag;
extern uint8_t gatt_buff[20];

// ����id
extern uint8_t connect_flag;	//���ӳɹ���־

extern uint8_t WakeupSource;	//����Դ��־

typedef struct
{
	uint8_t default_id;
  uint8_t password[3];    //��������
  uint8_t lockstate;   		//�ػ�ʱ����״̬(��/��)
	uint8_t discharge_flag;//��طŵ����
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

// GET_IMEI (v1.4 protocol 0x60)
extern uint8_t imei_bcd[8];
extern void set_imei_from_string(const char imei[15]);

#endif  // _USER_TASK_H



