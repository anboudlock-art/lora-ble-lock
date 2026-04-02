/**
 * E220-900T22S LoRa模块驱动
 * 
 * 模块特性:
 * - 工作频段: 850~930MHz
 * - 发射功率: 22dBm
 * - 传输距离: 5km
 * - 通讯接口: UART (TTL 3.3V)
 * 
 * 工作模式:
 * - 模式0: 透明传输模式 (M0=0, M1=0)
 * - 模式1: WOR发送模式 (M0=1, M1=0)
 * - 模式2: WOR接收模式 (M0=0, M1=1)
 * - 模式3: 休眠配置模式 (M0=1, M1=1)
 */

#ifndef __E220_DRIVER_H
#define __E220_DRIVER_H

#include <stdint.h>

//============================================================================
// 硬件引脚定义 (根据实际硬件连接修改)
//============================================================================
// 请根据实际硬件原理图修改以下引脚定义
#define E220_M0_PIN     14      // 模式控制引脚M0 (GPIO编号)
#define E220_M1_PIN     15      // 模式控制引脚M1 (GPIO编号)
#define E220_AUX_PIN    16      // 状态指示引脚AUX (GPIO编号)

// 使用U32BIT宏转换
#define E220_M0_BIT     (1 << E220_M0_PIN)
#define E220_M1_BIT     (1 << E220_M1_PIN)
#define E220_AUX_BIT    (1 << E220_AUX_PIN)

//============================================================================
// E220工作模式定义
//============================================================================
typedef enum {
    E220_MODE_TRANSPARENT = 0,    // 透明传输模式 (M0=0, M1=0)
    E220_MODE_WOR_TX     = 1,     // WOR发送模式 (M0=1, M1=0)
    E220_MODE_WOR_RX     = 2,     // WOR接收模式 (M0=0, M1=1)
    E220_MODE_CONFIG     = 3      // 休眠配置模式 (M0=1, M1=1)
} E220_WorkMode_t;

//============================================================================
// 锁状态定义
//============================================================================
typedef enum {
    LOCK_STATUS_UNLOCKED = 0x01,    // 开锁
    LOCK_STATUS_LOCKED   = 0x10,    // 上锁
    LOCK_STATUS_CUT      = 0x11     // 剪断报警
} Lock_Status_t;

//============================================================================
// LoRa指令定义
//============================================================================
#define LORA_CMD_UNLOCK         0x01    // 开锁指令
#define LORA_CMD_LOCK           0x10    // 上锁指令
#define LORA_CMD_CUT_ALARM      0x11    // 剪断报警

//============================================================================
// 协议常量
//============================================================================
#define LORA_GATEWAY_ADDR       0x0000  // 网关地址
#define LORA_BROADCAST_ADDR     0x0000  // 广播地址
#define LORA_DEFAULT_CHANNEL    0x04    // 默认信道

#define LORA_RESPONSE_OK        0x4F4B  // "OK" 响应

//============================================================================
// E220配置参数结构体
//============================================================================
typedef struct {
    uint16_t address;           // 模块地址 (0x0000~0xFFFE, 0xFFFF为广播地址)
    uint8_t  channel;           // 信道 (0~83, 频率=850.125+channel*1MHz)
    uint16_t crypt_key;         // 加密密钥 (0x0000~0xFFFF)
    uint8_t  uart_baud;         // 波特率 (3=9600)
    uint8_t  uart_parity;       // 校验位 (0=8N1)
    uint8_t  air_rate;          // 空中速率 (3=4.8k)
    uint8_t  tx_power;          // 发射功率 (0=22dBm)
    uint8_t  trans_mode;        // 传输方式 (0=透明传输, 1=定点传输)
} E220_Config_t;

//============================================================================
// LoRa网络配置结构体 (存储在Flash中)
//============================================================================
typedef struct {
    uint8_t  valid_flag;        // 有效标志 (0xAA表示有效)
    uint16_t local_addr;        // 本地地址
    uint8_t  channel;           // 信道
    uint8_t  reserved[4];       // 保留
} Lora_Network_Config_t;

#define LORA_NETWORK_FLASH_ADDR    0x0F0100    // LoRa网络配置存储地址
#define LORA_NETWORK_VALID_FLAG    0xAA        // 有效标志

//============================================================================
// E220数据缓冲区结构体
//============================================================================
#define E220_MAX_DATA_LEN    128

typedef struct {
    uint8_t  rx_buffer[E220_MAX_DATA_LEN];
    uint16_t rx_len;
    uint8_t  tx_buffer[E220_MAX_DATA_LEN];
    uint16_t tx_len;
    uint8_t  rx_ready;          // 接收完成标志
} E220_Data_t;

//============================================================================
// LoRa状态机结构体
//============================================================================
typedef struct {
    uint8_t  current_mode;          // 当前工作模式
    uint8_t  mode_switch_timer;     // 模式切换计时器 (单位: 10ms)
    uint8_t  in_config_mode;        // 是否在配置模式
    uint8_t  config_done;           // 配置完成标志
    uint8_t  wait_ok_timer;         // 等待OK响应计时器
    uint8_t  report_retry_cnt;      // 上报重试计数
    uint8_t  lock_state;            // 锁状态
    uint8_t  battery_level;         // 电量百分比
} Lora_State_t;

//============================================================================
// 全局变量声明
//============================================================================
extern E220_Config_t g_e220_config;
extern E220_Data_t   g_e220_data;
extern Lora_State_t  g_lora_state;
extern Lora_Network_Config_t g_lora_network;

//============================================================================
// 函数声明
//============================================================================

/**
 * E220模块初始化
 * 包括GPIO初始化、UART初始化、模块参数配置
 */
void E220_Init(void);

/**
 * 设置E220工作模式
 * @param mode 工作模式
 */
void E220_SetMode(E220_WorkMode_t mode);

/**
 * 获取当前工作模式
 * @return 当前模式
 */
E220_WorkMode_t E220_GetMode(void);

/**
 * 配置E220模块参数
 * @param config 配置参数结构体
 * @return 0=成功, 其他=失败
 */
int E220_Config(E220_Config_t *config);

/**
 * 读取E220模块参数
 * @param config 用于存储读取的配置参数
 * @return 0=成功, 其他=失败
 */
int E220_ReadConfig(E220_Config_t *config);

/**
 * 发送数据 (透明传输模式)
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
uint16_t E220_SendData(const uint8_t *data, uint16_t len);

/**
 * 发送数据到网关 (定点传输模式)
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
uint16_t E220_SendToGateway(const uint8_t *data, uint16_t len);

/**
 * 接收数据
 * @param data 数据缓冲区
 * @param max_len 最大接收长度
 * @return 实际接收的字节数
 */
uint16_t E220_ReceiveData(uint8_t *data, uint16_t max_len);

/**
 * 检查模块是否空闲
 * @return 1=空闲, 0=忙碌
 */
uint8_t E220_IsIdle(void);

/**
 * 等待模块空闲
 * @param timeout_ms 超时时间(毫秒)
 * @return 0=空闲, 其他=超时
 */
int E220_WaitIdle(uint32_t timeout_ms);

//============================================================================
// LoRa协议函数
//============================================================================

/**
 * LoRa上电初始化
 * 进入模式3，读取锁状态，获取地址和信道
 */
void Lora_PowerOn_Init(void);

/**
 * 上报锁状态到网关
 * @param status 锁状态 (01=开锁, 10=上锁, 11=剪断)
 */
void Lora_ReportStatus(uint8_t status);

/**
 * 上报锁状态和电量到网关
 * @param status 锁状态
 * @param battery 电量百分比
 */
void Lora_ReportStatusWithBattery(uint8_t status, uint8_t battery);

/**
 * 处理接收到的LoRa指令
 * @param data 接收数据
 * @param len 数据长度
 */
void Lora_ProcessCommand(const uint8_t *data, uint16_t len);

/**
 * LoRa模式切换处理 (在定时器中调用)
 * 实现: 模式3 ↔ 模式0 每3秒切换
 */
void Lora_ModeSwitchHandler(void);

/**
 * LoRa状态机处理 (在定时器中调用)
 */
void Lora_StateHandler(void);

/**
 * 保存LoRa网络配置到Flash
 */
void Lora_SaveNetworkConfig(void);

/**
 * 从Flash读取LoRa网络配置
 * @return 0=成功, 其他=失败
 */
int Lora_LoadNetworkConfig(void);

/**
 * UART接收中断回调函数
 * 在UART中断处理中调用此函数
 */
void E220_UART_RxCallback(uint8_t data);

/**
 * 帧结束处理函数
 * 在UART空闲超时后调用
 */
void E220_RxTimeoutHandler(void);

#endif /* __E220_DRIVER_H */

// LoRa控制变量（定义在user_app.c中）
extern uint8_t g_lora_unlock_pending;  // 待开锁标志
extern uint8_t g_lora_lock_pending;    // 待上锁标志
extern uint8_t g_lora_enabled;         // LoRa使能标志
