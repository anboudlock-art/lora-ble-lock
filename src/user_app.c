/**
 * 蓝牙锁用户应用 - LoRa版本（纯净版）
 * 
 * 功能说明:
 * 1. E220-900T22S LoRa模块通信
 * 2. 工作模式切换: 模式3(配置) <-> 模式0(通信) 每3秒切换
 * 3. 指令协议: 开锁/上锁/剪断报警
 * 4. 定时上报: 上报锁状态和电量
 * 5. 密码管理: 支持6位密码设置和验证
 */

#include "DebugLog.h"
#include <string.h>
#include "lib.h"
#include "gpio.h"
#include "spi.h"
#include "flash.h"
#include "ABD_ble_lock_service.h"
#include "softtimer.h"
#include "timer.h"
// #include "wdt.h"  // LoRa版本不使用看门狗
#include "delay.h"

#include "uart.h"
#include "user_app.h"
#include "os_timer.h"

// LoRa版本不使用看门狗，定义空宏
#define wdt_disable()  ((void)0)
#define wdt_enable(x)  ((void)0)
#define wdt_clear()    ((void)0)

#include "led.h"
#include "battery.h"
#include "input.h"
#include "motor.h"
#include "my_aes.h"
#include "user.h"

// LoRa模块驱动
#include "e220_driver.h"


#define USER_FLASH_BASE_ADDR 0	//flash data 8K
#define USER_FLASH_LENGHT 8

#define BATTERY_LOW_LEVEL		20	//低电量 30%  2-3V：0-100% 即2.3V

//notify打开标志
uint8_t start_tx=0;	//允许notify标志

uint8_t gatt_buff[20]={0};		//接收的数据

// 连接id
uint8_t connect_flag=0;	//连接成功标志

uint8_t WakeupSource=NONE;	//唤醒源标志

uint8_t Systerm_States=SLEEP;

uint8_t EnterSleepFlag=0;

//============================================================================
// LoRa工作状态
//============================================================================
uint8_t g_lora_enabled = 0;         // LoRa使能标志
uint8_t g_lora_unlock_pending = 0;  // 待开锁标志
uint8_t g_lora_lock_pending = 0;    // 待上锁标志
//============================================================================

/***********************
 * TYPEDEFS (类型定义)
 **********************/
#define	 DEFAULT_CODE	0xA5

#define  NONE_KEY		0x00
#define  SHORT_KEY	0x01
#define  LONG_KEY		0x02

#define  KEY_DOWN		0
#define  KEY_UP     1

/*******************************
 * GLOBAL VARIABLES (全局变量)
 ******************************/
 
uint8_t lock_state = 0;

static uint8_t   cmd_id=0;

uint8_t GetConnectTimeSuccess = 0;

uint8_t ConnectPasswordCorrect = 0;

static uint8_t ADV_Flag = 0;

//============================================================================
// 密码存储全局变量
//============================================================================
Password_Storage_t g_password_storage;  // 密码存储结构
//============================================================================

/******************************
 * LOCAL VARIABLES (本地变量)
 *****************************/
static uint8_t   BatValue;         		//电量

//AES加解密
uint8_t aes_key1[16];
uint8_t aes_key2[16];

unsigned char key_state=KEY_UP;
unsigned char key_lock = 0;

unsigned char key_enable=0;				//按键使能标志
unsigned int key_delay_cnt = 0;
unsigned char key_code = NONE_KEY;

unsigned char StateKeyState=KEY_UP;

uint8_t key_state_lock=0;
uint8_t LockOpen_Report = 0;

u8 timerSendFlag=0;

extern void rf_stop(void);
extern void rf_restart(void);

void  GPIO_callback(void);

//============================================================================
// LoRa处理函数
//============================================================================

/**
 * LoRa版本 - 1秒定时处理
 * 替换原来的4G处理
 */
void lora_1000ms_prc(void)
{
    // LoRa模式切换已在Lora_ModeSwitchHandler中处理
    // 这里可以添加其他定时任务，如心跳上报等
}

/**
 * LoRa版本 - 定时上报锁状态
 */
void lora_periodic_report(void)
{
    uint8_t status;
    
    // 获取当前锁状态
    lock_state = GetDoorState();
    BatValue = GetBatCapacity();
    
    // 转换为LoRa协议状态码
    if(lock_state == 0) {
        status = LOCK_STATUS_LOCKED;   // 0x10 上锁
    } else {
        status = LOCK_STATUS_UNLOCKED; // 0x01 开锁
    }
    
    // 上报到网关(带电量)
    Lora_ReportStatusWithBattery(status, BatValue);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa Periodic Report: Status=0x%02X, BAT=%d%%\r\n", status, BatValue);
    #endif
}

/**
 * LoRa版本 - 开锁完成回调
 * 开锁后立即上报锁状态
 */
void lora_on_unlock_complete(void)
{
    // 上报开锁状态
    Lora_ReportStatus(LOCK_STATUS_UNLOCKED);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa: Unlock complete, report status\r\n");
    #endif
}

/**
 * LoRa版本 - 上锁完成回调
 * 上锁后立即上报锁状态
 */
void lora_on_lock_complete(void)
{
    // 上报上锁状态
    Lora_ReportStatus(LOCK_STATUS_LOCKED);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa: Lock complete, report status\r\n");
    #endif
}

/**
 * LoRa版本 - 剪断报警回调
 */
void lora_on_cut_alarm(void)
{
    // 上报剪断报警
    Lora_ReportStatus(LOCK_STATUS_CUT);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa: Cut alarm, report status\r\n");
    #endif
}

//============================================================================

/**********************************************************
函数名: time1000ms_prc
描  述: 1s处理一次 (LoRa版本)
**********************************************************/
void time1000ms_prc(void)
{
// LoRa版本处理
    lora_1000ms_prc();
}

void auto_close(void)
{
	_user.bat_volt = BatValue = GetBatCapacity();
	
	//电池电量充足才会启动马达
	if(BatValue >= BATTERY_LOW_LEVEL)
	{
		if(1==GetInputStopR())		//如果是开锁的状态
		{
			Systerm_States |= AUTOLOCK;
			OS_timer_start(EVT_AUTO_LOCK, 0, false);
			key_state_lock = 1;//关锁状态
		}
	}
}

/**********************************************************
函数名: bt_slot10ms
描  述: 10ms定时处理
**********************************************************/
void bt_slot10ms(void)
{
	static u8 R_500mscnt,R_200mscnt;

	if(_user.key_7delay) _user.key_7delay--;

	if(++R_200mscnt>=20){
		R_200mscnt = 0;
		flg_200ms ^= 1;
	}

	R_500mscnt++;
	
	if(R_500mscnt>=50){
		R_500mscnt = 0;
		flg_ledflash ^= 1;
		if(flg_ledflash==0){
			time1000ms_prc();
		}
	}
    
// LoRa模式切换处理 (每10ms调用一次)
    Lora_ModeSwitchHandler();
    
    // LoRa状态机处理
    Lora_StateHandler();
#endif
}

/**********************************************************
函数名: updatedisp
描  述: LED显示更新 (LoRa版本)
**********************************************************/
void updatedisp(void)
{
// LoRa版本: 绿灯指示LoRa工作状态
    if(g_lora_enabled)
    {
        // 根据当前模式显示
        if(g_lora_state.current_mode == E220_MODE_TRANSPARENT)
        {
            // 模式0: 通信模式 - 绿灯亮
            if(flg_200ms){
                greenLedOn();
            }
            else{
                greenLedOff();
            }
        }
        else if(g_lora_state.current_mode == E220_MODE_CONFIG)
        {
            // 模式3: 配置模式 - 绿灯快闪
            greenLedOn();
        }
    }
    else
    {
        greenLedOff();
    }
}

/**********************************************************
函数名: Timer_2_callback
描  述: 2ms定时器回调 (LoRa版本)
**********************************************************/
static void Timer_2_callback(void)
{
	static u8 bt_slotcnt;
	
	hall_check();//剪断报警方检测
    
// LoRa版本: 不调用net4G_prc()

	switch(++bt_slotcnt){
		case 1:
			os_timer_query();	
		break;
		case 2:
			if(_user.lock_status != lock_state){
// LoRa版本: 锁状态变化时上报
                if(g_lora_enabled)
                {
                    uint8_t status = lock_state ? LOCK_STATUS_UNLOCKED : LOCK_STATUS_LOCKED;
                    Lora_ReportStatus(status);
                }
				_user.lock_status = lock_state;
			}
		break;
		case 3:
			// key_scan();
			// dealkey();			
		break;
		case 4:
			updatedisp();
		break;
		case 5:
			bt_slot10ms();
			bt_slotcnt = 0;
			flg_10ms = 1;
			if(key_state==KEY_DOWN)
				key_delay_cnt++;
		break;
	}	
}

void Enable_Timer2_2ms(void)
{
	timer_2_enable(32768*2/1000, Timer_2_callback);
}

void Disable_Timer2_2ms(void)
{
	timer_2_disable();
}

/*********************************************************************************
*函数名称:computer_sum
*功能描述:计算校验和
**********************************************************************************/
static uint8_t computer_sum(uint8_t *data,uint16_t lenth)
{
  uint8_t bcc = 0;
  uint16_t i = 0;
  for(i = 0;i < lenth;i ++)
  {
    bcc += data[i];
  }
  return bcc;
}

/* 发送给手机端 */
void user_encrypt_notify(const unsigned char* Key, uint8_t *rawDataBuf, uint16_t rawDatalength)
{
	struct gap_att_report report;
	uint8_t send_buff[16];
	uint8_t rawdata[16];
	
	memset(send_buff, 0, 16);
		
	if(rawDatalength<14)
	{
		rawdata[0] = 0xFB;
		rawdata[1] = rawDatalength;
		memcpy(rawdata+2,rawDataBuf,rawDatalength);
		memset(rawdata+2+rawDatalength, 0xFC, 16-2-rawDatalength);
		
		#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
		DBGHEXDUMP("encrypt data:\r\n",rawdata,16);
		DBGHEXDUMP("encrypt key:\r\n",(uint8_t *)Key,16);
		
		aes_encrypt_ecb(Key, 16, rawdata, send_buff, 1);
		
		#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
		DBGHEXDUMP("encrypted:\r\n",send_buff,16);
		#endif
		
		if(start_tx)
		{			
			#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
			DBGHEXDUMP("gatt_notify:\r\n",send_buff,16);
			#endif
			
			report.primary = BLE_VendorV2;
			report.uuid = BLE_VendorV2_Notify_UUID;
			report.hdl = BLE_VendorV2_Notify_VALUE_HANDLE;
			report.value = BLE_GATT_NOTIFICATION;

			GATTDataSend(BLE_GATT_NOTIFICATION, &report, 16, send_buff);
		}
	}
}

/* 蓝牙建立连接后 */
void gap_connect_evnet(void)
{
	OS_timer_stop(EVT_ENTER_SLEEP);
	RedLed_Config(RED_LED_OFF);
	OS_timer_SetEvt(EVT_REDLED_CONTROL);
	GetConnectTimeSuccess = 0;
	ConnectPasswordCorrect = 0;
	OS_timer_start(EVT_ENTER_SLEEP, 3000, 0);
}

/* 蓝牙断开连接后 */
void gap_disconnect_evnet(void)
{
	start_tx = 0;
	if(Systerm_States == OTAMODE)
	{
		delay_ms(100);
		SystemReset();
		delay_ms(1000);
	}
	else
	{
// LoRa版本: 断开蓝牙后不立即休眠，继续LoRa通信
        OS_timer_stop(EVT_ENTER_SLEEP);
        OS_timer_start(EVT_ENTER_SLEEP, 100, 0);
	}	
}

void ota_connect_evnet(void)
{
	if(Systerm_States != OTAMODE)
	{
		wdt_disable();
		OS_timer_stop(EVT_ENTER_SLEEP);
		AllLedOff();
		Disable_Timer2_2ms();
		#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
			dbg_printf("OTA Mode\r\n");
		#endif
	}
}

void WakupConfig_Disable(void)
{
	io_irq_disable_all();
	GPIO_Set_Input(U32BIT(STATE_KEY)|U32BIT(STOP_KEY1)|U32BIT(STOP_KEY2)|U32BIT(GPIO_USER_KEY)|U32BIT(lock_hall),0);
	PIN_Pullup_Enable(T_QFN_48, U32BIT(STATE_KEY)|U32BIT(STOP_KEY1)|U32BIT(STOP_KEY2)|U32BIT(GPIO_USER_KEY));
}

/* 硬件初始化 */
void hardware_init(void)
{
	LedBlink();
	LedInit();
	MotorInit();
	lock_state = GetDoorState();
	if(lock_state==0){flg_do_closelockOK = 1;}
}

//============================================================================
// 密码管理函数实现
//============================================================================

void password_init(void)
{
    SPI_Flash_Read((uint8_t*)&g_password_storage, PASSWORD_FLASH_ADDR, sizeof(g_password_storage));
    
    if(g_password_storage.valid_flag != PASSWORD_VALID_FLAG)
    {
        memset(g_password_storage.password, 0, 6);
        g_password_storage.valid_flag = PASSWORD_VALID_FLAG;
        g_password_storage.checksum = 0;
        
        SPI_Flash_Erase_Sector(PASSWORD_FLASH_ADDR);
        SPI_Flash_Write_Page((uint8_t*)&g_password_storage, PASSWORD_FLASH_ADDR, sizeof(g_password_storage));
        
        #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
        dbg_printf("Password init: default 000000\r\n");
        #endif
    }
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("Password loaded: %02X %02X %02X %02X %02X %02X\r\n",
              g_password_storage.password[0], g_password_storage.password[1],
              g_password_storage.password[2], g_password_storage.password[3],
              g_password_storage.password[4], g_password_storage.password[5]);
    #endif
}

int password_verify(const uint8_t *pwd)
{
    int match = 1;
    int i;
    
    for(i = 0; i < 6; i++)
    {
        if(pwd[i] != g_password_storage.password[i])
        {
            match = 0;
            break;
        }
    }
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("Password verify: %s\r\n", match ? "OK" : "FAIL");
    #endif
    
    return match;
}

int password_set(const uint8_t *pwd)
{
    memcpy(g_password_storage.password, pwd, 6);
    g_password_storage.valid_flag = PASSWORD_VALID_FLAG;
    g_password_storage.checksum = 0;
    
    SPI_Flash_Erase_Sector(PASSWORD_FLASH_ADDR);
    SPI_Flash_Write_Page((uint8_t*)&g_password_storage, PASSWORD_FLASH_ADDR, sizeof(g_password_storage));
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("Password set: %02X %02X %02X %02X %02X %02X\r\n",
              pwd[0], pwd[1], pwd[2], pwd[3], pwd[4], pwd[5]);
    #endif
    
    return 0;
}

int password_set_single_byte(uint8_t pwd_byte)
{
    memset(g_password_storage.password, pwd_byte, 6);
    g_password_storage.valid_flag = PASSWORD_VALID_FLAG;
    g_password_storage.checksum = 0;
    
    SPI_Flash_Erase_Sector(PASSWORD_FLASH_ADDR);
    SPI_Flash_Write_Page((uint8_t*)&g_password_storage, PASSWORD_FLASH_ADDR, sizeof(g_password_storage));
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("Password set single byte: 0x%02X\r\n", pwd_byte);
    #endif
    
    return 0;
}

void password_reset(void)
{
    password_set_single_byte(0);
}

//============================================================================
// BLE 指令处理函数（从 4G-BLE 版本整合）
//============================================================================

/**
 * BLE 指令处理回调函数
 * 处理手机 APP 发送的指令
 * 
 * 支持指令:
 * - 0x10: 连接密码验证
 * - 0x20: 密码验证
 * - 0x21: 密码设置（新增功能）
 * - 0x30: 开锁
 * - 0x31: 上锁
 * - 0x40: 状态查询
 * - 0x50: 强制休眠
 */
uint8_t gatt_read_AppCB(uint8_t *p_data, uint8_t pdata_len)
{
    uint8_t databuf[16], datalen = 0;
    uint8_t aes_buff[16], p_value[16], len = 0;
    uint8_t key[16];
    
    if(pdata_len != 16)
        return 1;
    
    // 选择加密密钥
    if(GetConnectTimeSuccess)
    {
        memcpy(key, aes_key2, 16);
    }
    else
    {
        memcpy(key, aes_key1, 16);
    }
    
    // AES 解密
    aes_decrypt_ecb(key, 16, p_data, aes_buff, 1);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    DBGHEXDUMP("decrypted:\r\n", aes_buff, 16);
    #endif
    
    // 提取有效数据
    if(aes_buff[1] < 14)
    {
        len = aes_buff[1];
        memcpy(p_value, aes_buff + 2, len);
    }
    else
    {
        return 1;
    }
    
    // 已连接并验证时间
    if(GetConnectTimeSuccess == true)
    {
        if(ConnectPasswordCorrect == true)  // 连接密码正确
        {
            if(len == 4)  // 强制休眠命令
            {
                if((p_value[0] == 0x55) && (p_value[2] == 0x50) && (p_value[3] == computer_sum(p_value+1, 2)))
                {
                    _user.bat_volt = BatValue = GetBatCapacity();
                    
                    databuf[0] = 0xAA;
                    cmd_id++;
                    databuf[1] = cmd_id;
                    databuf[2] = 0x50;
                    databuf[3] = BatValue;
                    databuf[4] = lock_state;
                    databuf[5] = 0;
                    databuf[6] = computer_sum(databuf+1, 5);
                    datalen = 7;
                    
                    user_encrypt_notify(aes_key2, databuf, datalen);
                }
            }
            else if(len == 10)  // 0x20 密码验证 + 0x21 密码设置（新协议）
            {
                // 密码验证和密码设置使用相同的数据长度（len=10）
                // 格式: 55 cmd 0x20/0x21 pwd[0] pwd[1] pwd[2] pwd[3] pwd[4] pwd[5] sum
                if((p_value[0] == 0x55) && (p_value[9] == computer_sum(p_value+1, 8)))
                {
                    // 0x21 密码设置指令（新协议：支持6字节独立密码）
                    if(p_value[2] == 0x21)
                    {
                        // 保存6字节独立密码到Flash
                        password_set(&p_value[3]);
                        
                        // 回复成功
                        databuf[0] = 0xAA;
                        cmd_id++;
                        databuf[1] = cmd_id;
                        databuf[2] = 0x21;
                        databuf[3] = 0x00;  // 成功
                        databuf[4] = computer_sum(databuf+1, 3);
                        datalen = 5;
                        
                        user_encrypt_notify(aes_key2, databuf, datalen);
                    }
                    // 0x20 密码验证（使用Flash存储的密码）
                    else if(p_value[2] == 0x20)
                    {
                        int pwd_match = password_verify(&p_value[3]);
                        
                        if(pwd_match)
                        {
                            ConnectPasswordCorrect = true;
                            
                            OS_timer_stop(EVT_ENTER_SLEEP);
                            
                            databuf[0] = 0xAA;
                            cmd_id++;
                            databuf[1] = cmd_id;
                            databuf[2] = 0x20;
                            databuf[3] = 0x00;  // 密码正确
                            databuf[4] = computer_sum(databuf+1, 3);
                            datalen = 5;
                            
                            BlueLed_Config(BLUE_SLOW_FLASH_1);
                            OS_timer_SetEvt(EVT_BLUELED_CONTROL);
                            
                            user_encrypt_notify(aes_key2, databuf, datalen);
                            
                            OS_timer_start(EVT_ENTER_SLEEP, 6000, false);
                        }
                        else 
                        {
                            // 密码验证错误
                            databuf[0] = 0xAA;
                            cmd_id++;
                            databuf[1] = cmd_id;
                            databuf[2] = 0x20;
                            databuf[3] = 0x01;  // 密码错误
                            databuf[4] = computer_sum(databuf+1, 3);
                            datalen = 5;
                            
                            user_encrypt_notify(aes_key2, databuf, datalen);
                        }
                    }
                }
            }
            else if(len == 5)  // 开锁/上锁/状态查询命令
            {
                _user.bat_volt = BatValue = GetBatCapacity();
                
                if((p_value[0] == 0x55) && (p_value[4] == computer_sum(p_value+1, 3)))
                {
                    // 0x30 开锁命令
                    if(p_value[2] == 0x30)
                    {
                        LockOpen_Report = 1;
                        
                        _user.bat_volt = BatValue = GetBatCapacity();
                        
                        OS_timer_stop(EVT_ENTER_SLEEP);
                        
                        BlueLed_Config(BLUE_LED_OFF);
                        OS_timer_SetEvt(EVT_BLUELED_CONTROL);
                        
                        if(BatValue >= BATTERY_LOW_LEVEL)
                        {
                            if(1 == GetInputStopF())
                            {
                                MotorStop();
                                MotorStartForward();
                                OS_timer_start(EVT_MOTOR_STOP, 200, false);
                                OS_timer_SetEvt(EVT_STATE_CHECK);
                                OS_timer_start(EVT_ENTER_SLEEP, 3000, false);
                            }
                            else 
                            {
                                OS_timer_start(EVT_MOTOR_STOP, 2, false);
                            }
                        }
                        else
                        {
                            RedLed_Config(RED_FAST_FLASH_1);
                            OS_timer_SetEvt(EVT_REDLED_CONTROL);
                            OS_timer_start(EVT_ENTER_SLEEP, 600, false);
                        }
                    }
                    // 0x31 上锁命令
                    else if(p_value[2] == 0x31)
                    {
                        LockOpen_Report = 0;
                        
                        _user.bat_volt = BatValue = GetBatCapacity();
                        
                        OS_timer_stop(EVT_ENTER_SLEEP);
                        
                        if(BatValue >= BATTERY_LOW_LEVEL)
                        {
                            if(0 == key_state_lock)
                            {
                                if(1 == GetInputStopR())
                                {
                                    Systerm_States |= AUTOLOCK;
                                    OS_timer_start(EVT_AUTO_LOCK, 0, false);
                                    key_state_lock = 1;
                                }
                                else 
                                {
                                    OS_timer_start(EVT_MOTOR_STOP, 2, false);
                                }
                            }
                        }
                        else
                        {
                            RedLed_Config(RED_FAST_FLASH_1);
                            OS_timer_SetEvt(EVT_REDLED_CONTROL);
                            OS_timer_start(EVT_ENTER_SLEEP, 600, false);
                        }
                    }
                    // 0x40 状态查询命令
                    else if(p_value[2] == 0x40)
                    {
                        _user.bat_volt = BatValue = GetBatCapacity();
                        
                        databuf[0] = 0xAA;
                        cmd_id++;
                        databuf[1] = cmd_id;
                        databuf[2] = 0x40;
                        databuf[3] = BatValue;
                        databuf[4] = lock_state;
                        databuf[5] = 0x00;
                        databuf[6] = computer_sum(databuf+1, 5);
                        datalen = 7;
                        
                        user_encrypt_notify(aes_key2, databuf, datalen);
                    }
                    // 0x41 查询锁号指令（新增，保持和原固件一致）
                    else if(p_value[2] == 0x41)
                    {
                        databuf[0] = 0xAA;
                        cmd_id++;
                        databuf[1] = cmd_id;
                        databuf[2] = 0x41;
                        databuf[3] = _user.lock_sn8array.data8[0];
                        databuf[4] = _user.lock_sn8array.data8[1];
                        databuf[5] = _user.lock_sn8array.data8[2];
                        databuf[6] = _user.lock_sn8array.data8[3];
                        databuf[7] = _user.lock_sn8array.data8[4];
                        databuf[8] = _user.lock_sn8array.data8[5];
                        databuf[9] = _user.lock_sn8array.data8[6];
                        databuf[10] = _user.lock_sn8array.data8[7];
                        databuf[11] = computer_sum(databuf+1, 10);
                        datalen = 12;
                        
                        user_encrypt_notify(aes_key2, databuf, datalen);
                    }
                }
            }
        }
    } 
    else 
    {
        // 连接密码验证（0x10）- 保持和原固件完全一致
        if(len == 10)
        {
            if((p_value[0] == 0x55) && (p_value[2] == 0x10) && (p_value[9] == computer_sum(p_value+1, 8)))
            {
                GetConnectTimeSuccess = true;
                cmd_id = p_value[1];
                cmd_id++;
                OS_timer_stop(EVT_ENTER_SLEEP);
                
                // 回复指令
                databuf[0] = 0xAA;
                databuf[1] = cmd_id;
                databuf[2] = 0x10;
                databuf[3] = computer_sum(databuf+1, 2);
                datalen = 4;
                
                user_encrypt_notify(aes_key1, databuf, datalen);
                
                // 更新密钥 key2（用时间更新密钥）
                aes_key2[10] = p_value[3];
                aes_key2[11] = p_value[4];
                aes_key2[12] = p_value[5];
                aes_key2[13] = p_value[6];
                aes_key2[14] = p_value[7];
                aes_key2[15] = p_value[8];
                
                OS_timer_start(EVT_ENTER_SLEEP, 1000, false);
            }
        }
    }
    
    return 0;
}

/**
 * GATT 读写处理
 */
void GATT_ReadWrite_Process(void)
{
    gatt_read_AppCB(&gatt_buff[1], gatt_buff[0]);
}

//============================================================================

void UserEncryptInit(void)
{
	uint8_t i;
	struct gap_ble_addr dev_addr;

	GetDevAddr(&dev_addr);
	#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)	
		DBGHEXDUMP("MAC:\r\n",dev_addr.addr,6);
	#endif
	
	for(i=0;i<6;i++)
	{
		aes_key1[i] = dev_addr.addr[5-i];
		aes_key2[i] = dev_addr.addr[5-i];
	}
	
	aes_key1[6] = 0x11;
	aes_key2[6] = 0x11;
	
	for(i=0;i<9;i++)
	{
		aes_key1[i+7] = aes_key1[i+6] + 0x11;
		aes_key2[i+7] = aes_key1[i+6] + 0x11;
	}
	
	#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
	DBGHEXDUMP("Key1:\r\n",aes_key1,16);
	DBGHEXDUMP("Key2:\r\n",aes_key1,16);
	#endif
	
	// 初始化密码存储
	password_init();
	
// LoRa版本: 初始化E220模块
    E220_Init();
    g_lora_enabled = 1;
    
    // LoRa上电初始化流程
    Lora_PowerOn_Init();
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa Version Enabled\r\n");
    #endif
#endif
}

//============================================================================
// LoRa指令处理回调
//============================================================================

/**
 * 处理LoRa开锁指令
 * 在主循环中调用
 */
void lora_process_unlock(void)
{
    if(g_lora_unlock_pending)
    {
        g_lora_unlock_pending = 0;
        
        // 执行开锁动作
        // 这里调用原来的开锁逻辑
        // motor_unlock();
        
        #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
        dbg_printf("LoRa: Execute unlock command\r\n");
        #endif
        
        // 开锁完成后上报状态
        // lora_on_unlock_complete();
    }
}

/**
 * 处理LoRa上锁指令
 */
void lora_process_lock(void)
{
    if(g_lora_lock_pending)
    {
        g_lora_lock_pending = 0;
        
        // 等待上锁动作完成
        // 这里调用原来的上锁逻辑
        // motor_lock();
        
        #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
        dbg_printf("LoRa: Execute lock command\r\n");
        #endif
    }
}

//============================================================================

void user_wakeup(void)
{
	if(WakeupSource>0)
	{		
		WakupConfig_Disable();

		lock_state = GetDoorState();

		_user.key_7cnt = 0;
		_user.key_7delay = 0;

		#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
		dbg_printf("State:0x%02x\r\n", lock_state);	
		#endif
		
		_user.bat_volt = BatValue = GetBatCapacity();
		#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
		dbg_printf("battery:%d%%\r\n", BatValue);
		#endif
		
		wdt_enable(128*10);
		wdt_clear();

		Enable_Timer2_2ms();

		key_state = KEY_UP;
		key_delay_cnt = 0;
		key_lock = 0;
		
		if(BatValue < BATTERY_LOW_LEVEL)
		{
			RedLed_Config(RED_FAST_FLASH_1);
			OS_timer_SetEvt(EVT_REDLED_CONTROL);
			OS_timer_start( EVT_ENTER_SLEEP, 600, false );
			WakeupSource = 0;
		}
		else
		{
			if(WakeupSource&USER_BUTTON)
			{
				WakeupSource ^= USER_BUTTON;
				#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
				dbg_printf("Wakeup from power-key\r\n");
				#endif
				start_tx = 0;		
				UserEncryptInit();
				
				key_enable = 1;

				Systerm_States |= STARTUP;

				if(0==(Systerm_States&POWERON))
				{
					if(GPIO_Pin_Read(U32BIT(GPIO_USER_KEY)))
					{
						RedLed_Config(RED_SLOW_FLASH_1);
						OS_timer_SetEvt(EVT_REDLED_CONTROL);

						Systerm_States |= POWERON;
						OS_timer_start( EVT_START_DEVICE, 2, false );
						
						dbg_printf("SystemState:0x%02x\r\n", Systerm_States);
					}
				}
			}
			else if(WakeupSource&lock_cut){
				WakeupSource ^= lock_cut;

				timerSendFlag=0;
				
				start_tx = 0;		
				UserEncryptInit();
				
				key_enable = 1;				
				Systerm_States |= STARTUP;

				if(GPIO_Pin_Read(U32BIT(lock_hall)))
				{
					RedLed_Config(RED_SLOW_FLASH_1);
					OS_timer_SetEvt(EVT_REDLED_CONTROL);

					Systerm_States |= POWERON;
					OS_timer_start( EVT_START_DEVICE, 2, false );
					flg_cutup = 1;
                    
// LoRa版本: 上报剪断报警
                    lora_on_cut_alarm();
				}
				else{
					OS_timer_start(EVT_ENTER_SLEEP, 1, 0);
				}			
			}
			else if(WakeupSource&RTC_ALARM)
			{
				WakeupSource ^= RTC_ALARM;
				
				key_enable = 1;
				
				if(0==(Systerm_States&BATDISCHG))
				{
					Systerm_States |= BATDISCHG;
					BatDischargeOn();
					#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
					dbg_printf("Wakeup from RTC\r\n");
					#endif
					OS_timer_start( EVT_BAT_DISCHARGE, 3000, false );
				}
			}
			else if(WakeupSource & RTC_4Greport){
				WakeupSource ^= RTC_4Greport;
				
				key_enable = 1;	
				Systerm_States |= STARTUP;

// LoRa版本: 定时上报
                RedLed_Config(RED_LED_OFF);
                lora_periodic_report();
                OS_timer_stop(EVT_ENTER_SLEEP);
                OS_timer_start( EVT_START_DEVICE, 2, false );							
			}
			else
			{
				WakeupSource = 0;
			}
		}
	}
	
	// 按键处理逻辑保持不变...
	if(key_enable)
	{
		if(0==GPIO_Pin_Read(U32BIT(GPIO_USER_KEY)))
		{
			if(!key_lock)
			{
				key_state = KEY_DOWN;
			}
			if(key_delay_cnt>=200+400)
			{
				_user.key_7cnt = 0;
				_user.key_7delay = 0;
				key_delay_cnt = 0;
				key_lock = 1;
				key_state = KEY_UP;
				
// LoRa版本: 长按启用LoRa
				Systerm_States |= STARTUP;
				RedLed_Config(RED_LED_OFF);
				// E220已在UserEncryptInit中初始化
                g_lora_enabled = 1;
				OS_timer_stop(EVT_ENTER_SLEEP);
				OS_timer_start( EVT_START_DEVICE, 2, false );
			}
		}
		else
		{	
			if(key_delay_cnt>0 && key_delay_cnt<500)
			{
				if(_user.key_7delay){
					_user.key_7delay = 80;
					if(++_user.key_7cnt>=3){
						flg_reset = 1;					
					}
				}
				else{
					_user.key_7cnt = 0;
					_user.key_7delay = 50;
				}

				if(!connect_flag)
				{
					if(Systerm_States & STARTUP)
					{
						Systerm_States^=STARTUP;
					}

					OS_timer_stop(EVT_ENTER_SLEEP);

					RedLed_Config(RED_SLOW_FLASH_1);
					OS_timer_SetEvt(EVT_REDLED_CONTROL);
					flg_needred3 = 1;
					
					Systerm_States |= POWERON;
					OS_timer_start( EVT_START_DEVICE, 2, false );
				}
			}
			key_state = KEY_UP;
			key_delay_cnt = 0;
			key_lock = 0;
		}
		
		if((0==GPIO_Pin_Read(U32BIT(STATE_KEY))) && (GPIO_Pin_Read(U32BIT(lock_hall)) == 0))
		{
			uint8_t databuf[20],datalen=0;
			
			if(StateKeyState==KEY_UP)
			{	
// LoRa版本: 自动上锁
				auto_close();
				StateKeyState = KEY_DOWN;
			}
		}		
		else
		{
			StateKeyState = KEY_UP;
		}
	}
}

/* 唤醒按键回调函数 */
void  GPIO_callback(void)
{
	uint32_t SW;

	SW=GPIO_IRQ_CTRL->GPIO_INT; 

	if(SW&U32BIT(lock_hall))
	{
		WakeupSource |= lock_cut;		
	}
	
	if(SW&U32BIT(GPIO_USER_KEY))
	{
		#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
		dbg_printf("USER_KEY int\r\n");
		WakeupSource |= USER_BUTTON;
	}
}

void gpio_init_sleep(void)
{
	if(GetInputStopF())
	{
		PIN_Pullup_Enable(T_QFN_48, U32BIT(STOP_KEY1));
	}
	else 
	{
		PIN_Pullup_Disable(T_QFN_48, U32BIT(STOP_KEY1));	
	}
	
	if(GetInputStopR())
	{
		PIN_Pullup_Enable(T_QFN_48, U32BIT(STOP_KEY2));
	}
	else 
	{
		PIN_Pullup_Disable(T_QFN_48, U32BIT(STOP_KEY2));	
	}
}

/* 进入sleep前设置唤醒源 */
void WakupConfig_BeforeSleep(void)
{
	struct gap_wakeup_config pw_cfg;
	uint32_t io_int=0,io_inv=0;

SLEEPCONFIG:
		wdt_clear();
		PIN_Pullup_Enable(T_QFN_48, U32BIT(GPIO_USER_KEY)|U32BIT(STATE_KEY));
		GPIO_Set_Input(U32BIT(GPIO_USER_KEY)|U32BIT(STATE_KEY), 0);

		PIN_Pullup_Disable(T_QFN_48, U32BIT(lock_hall));	

		if(GPIO_Pin_Read(U32BIT(lock_hall)))
		{
			GPIO_Set_Input(U32BIT(lock_hall), U32BIT(lock_hall));
		}
		else
		{
			GPIO_Set_Input(U32BIT(lock_hall), 0);
		}

		delay_ms(2);
		
		if(GPIO_Pin_Read(U32BIT(lock_hall))) 
		{
			goto SLEEPCONFIG;
		}

		io_int = U32BIT(GPIO_USER_KEY);
	
		if(GPIO_Pin_Read(U32BIT(GPIO_USER_KEY)))
		{
			io_inv |= U32BIT(GPIO_USER_KEY);
		}
		else
		{
			io_inv &= ~U32BIT(GPIO_USER_KEY);
		}
	
		if(!GetInputStopF() && GetInputStopR())
		{
			PIN_Pullup_Enable(T_QFN_48, U32BIT(STOP_KEY2));
			PIN_Pullup_Disable(T_QFN_48, U32BIT(STOP_KEY1));
			
			GPIO_Set_Input(io_int, io_inv);
			delay_ms(2);
			
			if(GPIO_Pin_Read(U32BIT(GPIO_USER_KEY))) 
			{
				goto SLEEPCONFIG;
			}
		}
		else if(GetInputStopF() && !GetInputStopR()) 
		{
			PIN_Pullup_Enable(T_QFN_48, U32BIT(STOP_KEY1));
			PIN_Pullup_Disable(T_QFN_48, U32BIT(STOP_KEY2));
			
			GPIO_Set_Input(io_int, io_inv);
			delay_ms(2);

			if(GPIO_Pin_Read(U32BIT(GPIO_USER_KEY))) 
			{
				goto SLEEPCONFIG;
			}
		}
		else if(GetInputStopF() && GetInputStopR())
		{
			PIN_Pullup_Enable(T_QFN_48, U32BIT(STOP_KEY2));
			PIN_Pullup_Enable(T_QFN_48, U32BIT(STOP_KEY1));
			
			GPIO_Set_Input(io_int, io_inv);
			delay_ms(2);
			
			if(GPIO_Pin_Read(U32BIT(GPIO_USER_KEY))) 
			{
				goto SLEEPCONFIG;
			}
		}
		else if(!GetInputStopF() && !GetInputStopR())
		{
			PIN_Pullup_Disable(T_QFN_48, U32BIT(STOP_KEY2));
			PIN_Pullup_Disable(T_QFN_48, U32BIT(STOP_KEY1));
			
			GPIO_Set_Input(io_int, io_inv);
			delay_ms(2);
			
			if(GPIO_Pin_Read(U32BIT(GPIO_USER_KEY))) 
			{
				goto SLEEPCONFIG;
			}
		}
		
		pw_cfg.wakeup_type = SLEEP_WAKEUP;
		pw_cfg.wdt_wakeup_en = (bool)false;
		pw_cfg.rtc_wakeup_en = (bool)true;
		pw_cfg.timer_wakeup_en = (bool)false;
		pw_cfg.gpi_wakeup_en = (bool)true;
		pw_cfg.gpi_wakeup_cfg = io_int;
		WakeupConfig(&pw_cfg);

		io_irq_enable(io_int, GPIO_callback);
}

void FeedDog(void)
{
	wdt_clear();
}

void user_task(void)
{
// LoRa版本: 处理LoRa指令
    lora_process_unlock();
    lora_process_lock();
}
