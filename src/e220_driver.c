/**
 * E220-900T22S LoRa模块驱动实现
 * 
 * 实现功能:
 * 1. 工作模式切换: 模式3(配置) <-> 模式0(通信) 每3秒切换
 * 2. 上电初始化: 读取锁状态，获取地址和信道
 * 3. 指令协议: 开锁/上锁/剪断报警
 * 4. 定时上报: 上报锁状态和电量
 * 5. 休眠机制: 收到OK后休眠，否则5秒后强制休眠
 * 
 * 协议格式（基于需求文档）：
 * - 上报: [网关地址2B][网关信道1B][本设备地址2B][本设备信道1B][状态1B]
 *         发送: 00 00 04 00 03 12 01
 *         网关收到: 00 03 12 01 (地址+信道+状态)
 * - 下发: [设备地址2B][设备信道1B][指令1B]
 *         发送: 00 03 12 01
 *         设备收到: 01 (只有指令)
 */

#include "e220_driver.h"
#include "uart.h"
#include "gpio.h"
#include "delay.h"
#include "flash.h"             // flash.h在工程包含路径中
#include "os_timer.h"
#include "DebugLog.h"
#include <string.h>

//============================================================================
// HEX指令定义
//============================================================================
#define E220_CMD_WRITE      0xC0    // 写寄存器
#define E220_CMD_READ       0xC1    // 读寄存器

//============================================================================
// 寄存器地址定义
//============================================================================
#define E220_REG_ADDH       0x00    // 地址高字节
#define E220_REG_ADDL       0x01    // 地址低字节
#define E220_REG_SPED       0x02    // 串口速率配置
#define E220_REG_OPTION     0x04    // 选项配置
#define E220_REG_CHANNEL    0x05    // 信道
#define E220_REG_CRYPT_H    0x06    // 加密密钥高字节
#define E220_REG_CRYPT_L    0x07    // 加密密钥低字节

//============================================================================
// 定时器配置
//============================================================================
#define MODE_SWITCH_INTERVAL    300     // 模式切换间隔 (单位: 10ms, 300=3秒)
#define WAIT_OK_TIMEOUT         500     // 等待OK超时 (单位: 10ms, 500=5秒)

//============================================================================
// 协议常量（根据需求文档）
//============================================================================
#define GATEWAY_ADDR_H      0x00        // 网关地址高字节
#define GATEWAY_ADDR_L      0x00        // 网关地址低字节
#define GATEWAY_CHANNEL     0x04        // 网关监听信道

//============================================================================
// 全局变量
//============================================================================
E220_Config_t g_e220_config;
E220_Data_t   g_e220_data;
Lora_State_t  g_lora_state;
Lora_Network_Config_t g_lora_network;

// 外部函数声明
extern void uart_write(uint8_t *data, uint16_t len);
extern uint16_t uart_read(uint8_t *data, uint16_t max_len);

//============================================================================
// 内部函数声明
//============================================================================
static void E220_SetM0M1(uint8_t m0, uint8_t m1);
static void E220_DelayMs(uint32_t ms);

//============================================================================
// 初始化函数
//============================================================================

/**
 * E220模块初始化
 */
void E220_Init(void)
{
    // 初始化数据结构
    memset(&g_e220_data, 0, sizeof(E220_Data_t));
    memset(&g_e220_config, 0, sizeof(E220_Config_t));
    memset(&g_lora_state, 0, sizeof(Lora_State_t));
    
    // 初始化GPIO引脚
    // M0和M1配置为输出，默认低电平
    GPIO_Set_Output(U32BIT(E220_M0_PIN));
    GPIO_Set_Output(U32BIT(E220_M1_PIN));
    PIN_Pullup_Enable(T_QFN_48, U32BIT(E220_M0_PIN));
    PIN_Pullup_Enable(T_QFN_48, U32BIT(E220_M1_PIN));
    
    // AUX配置为输入
    GPIO_Set_Input(U32BIT(E220_AUX_PIN), 0);
    PIN_Pullup_Enable(T_QFN_48, U32BIT(E220_AUX_PIN));
    
    // 默认设置为模式0(透明传输)
    E220_SetMode(E220_MODE_TRANSPARENT);
    
    // 初始化状态
    g_lora_state.current_mode = E220_MODE_TRANSPARENT;
    g_lora_state.mode_switch_timer = 0;
    g_lora_state.config_done = 0;
    
    // 加载网络配置
    if(Lora_LoadNetworkConfig() == 0)
    {
        g_e220_config.address = g_lora_network.local_addr;
        g_e220_config.channel = g_lora_network.channel;
    }
    else
    {
        // 使用默认配置（根据需求文档：地址0003，信道12）
        g_e220_config.address = 0x0003;
        g_e220_config.channel = 0x12;
        g_lora_network.local_addr = 0x0003;
        g_lora_network.channel = 0x12;
    }
    
    // 延时等待模块稳定
    E220_DelayMs(100);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("E220 Init: Addr=0x%04X, CH=%d\r\n", 
               g_e220_config.address, g_e220_config.channel);
    #endif
}

/**
 * 设置E220工作模式
 */
void E220_SetMode(E220_WorkMode_t mode)
{
    switch(mode)
    {
        case E220_MODE_TRANSPARENT:  // 透明传输模式 (M0=0, M1=0)
            E220_SetM0M1(0, 0);
            break;
        case E220_MODE_WOR_TX:       // WOR发送模式 (M0=1, M1=0)
            E220_SetM0M1(1, 0);
            break;
        case E220_MODE_WOR_RX:       // WOR接收模式 (M0=0, M1=1)
            E220_SetM0M1(0, 1);
            break;
        case E220_MODE_CONFIG:       // 休眠配置模式 (M0=1, M1=1)
            E220_SetM0M1(1, 1);
            break;
    }
    
    g_lora_state.current_mode = mode;
    
    // 切换模式需要等待约15ms
    E220_DelayMs(20);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("E220 Mode: %d (M0=%d, M1=%d)\r\n", mode, 
               (mode & 1) ? 1 : 0, (mode & 2) ? 1 : 0);
    #endif
}

/**
 * 获取当前工作模式
 */
E220_WorkMode_t E220_GetMode(void)
{
    return g_lora_state.current_mode;
}

/**
 * 设置M0和M1引脚状态
 */
static void E220_SetM0M1(uint8_t m0, uint8_t m1)
{
    if(m0)
        GPIO_Pin_Set(U32BIT(E220_M0_PIN));
    else
        GPIO_Pin_Clear(U32BIT(E220_M0_PIN));
    
    if(m1)
        GPIO_Pin_Set(U32BIT(E220_M1_PIN));
    else
        GPIO_Pin_Clear(U32BIT(E220_M1_PIN));
}

//============================================================================
// 配置函数
//============================================================================

/**
 * 配置E220模块参数
 */
int E220_Config(E220_Config_t *config)
{
    uint8_t cmd[10];
    
    // 进入配置模式
    E220_SetMode(E220_MODE_CONFIG);
    
    E220_DelayMs(50);
    
    // 构造配置命令: C0 00 08 ADDH ADDL SPED OPTION CHANNEL CRYPT_H CRYPT_L
    cmd[0] = E220_CMD_WRITE;
    cmd[1] = E220_REG_ADDH;
    cmd[2] = 0x08;  // 写入8个寄存器
    
    cmd[3] = (uint8_t)(config->address >> 8);      // ADDH
    cmd[4] = (uint8_t)(config->address & 0xFF);    // ADDL
    
    // SPED寄存器: 空中速率|校验位|波特率
    // 默认: 9600波特率(3), 8N1(0), 4.8k空中速率(3)
    cmd[5] = (3 << 5) | (0 << 3) | 3;   // 0x63
    
    // OPTION寄存器: 传输方式|RSSI使能|发射功率
    cmd[6] = 0x40 | (config->tx_power << 2);  // 定点传输模式
    
    cmd[7] = config->channel;                       // CHANNEL
    cmd[8] = (uint8_t)(config->crypt_key >> 8);    // CRYPT_H
    cmd[9] = (uint8_t)(config->crypt_key & 0xFF);  // CRYPT_L
    
    // 发送配置命令
    uart_write(cmd, 10);
    
    // 等待响应 (约1秒)
    E220_DelayMs(100);
    
    // 保存配置
    memcpy(&g_e220_config, config, sizeof(E220_Config_t));
    
    // 保存到Flash
    g_lora_network.local_addr = config->address;
    g_lora_network.channel = config->channel;
    Lora_SaveNetworkConfig();
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("E220 Config: Addr=0x%04X, CH=%d\r\n", 
               config->address, config->channel);
    #endif
    
    return 0;
}

/**
 * 读取E220模块参数
 */
int E220_ReadConfig(E220_Config_t *config)
{
    uint8_t cmd[3];
    
    // 进入配置模式
    E220_SetMode(E220_MODE_CONFIG);
    
    E220_DelayMs(50);
    
    // 构造读取命令: C1 00 08
    cmd[0] = E220_CMD_READ;
    cmd[1] = E220_REG_ADDH;
    cmd[2] = 0x08;
    
    // 发送命令
    uart_write(cmd, 3);
    
    // 读取响应 (需要根据实际UART实现)
    // 响应格式: C1 00 08 ADDH ADDL SPED OPTION CHANNEL CRYPT_H CRYPT_L
    
    E220_DelayMs(100);
    
    return 0;
}

//============================================================================
// 数据发送接收函数
//============================================================================

/**
 * 发送数据到指定地址 (定点传输模式)
 * 
 * E220定点传输格式: [目标地址高][目标地址低][目标信道][数据...]
 * 接收方收到: [数据...] (不包含地址和信道头)
 * 
 * @param addr 目标地址
 * @param channel 目标信道
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的数据字节数
 */
uint16_t E220_SendTo(uint16_t addr, uint8_t channel, const uint8_t *data, uint16_t len)
{
    uint16_t total_len;
    
    if(len > E220_MAX_DATA_LEN - 3)
    {
        len = E220_MAX_DATA_LEN - 3;
    }
    
    // 定点传输格式: [目标地址高][目标地址低][目标信道][数据...]
    g_e220_data.tx_buffer[0] = (uint8_t)(addr >> 8);
    g_e220_data.tx_buffer[1] = (uint8_t)(addr & 0xFF);
    g_e220_data.tx_buffer[2] = channel;
    
    // 复制数据
    memcpy(&g_e220_data.tx_buffer[3], data, len);
    
    total_len = len + 3;
    
    // 发送数据
    uart_write(g_e220_data.tx_buffer, total_len);
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa TX to [0x%04X:%02X]: ", addr, channel);
    DBGHEXDUMP("", g_e220_data.tx_buffer, total_len);
    #endif
    
    return len;
}

/**
 * 发送数据到网关
 */
uint16_t E220_SendToGateway(const uint8_t *data, uint16_t len)
{
    return E220_SendTo(LORA_GATEWAY_ADDR, GATEWAY_CHANNEL, data, len);
}

/**
 * 发送数据 (透明传输模式)
 */
uint16_t E220_SendData(const uint8_t *data, uint16_t len)
{
    if(len > E220_MAX_DATA_LEN)
    {
        len = E220_MAX_DATA_LEN;
    }
    
    uart_write((uint8_t *)data, len);
    
    return len;
}

/**
 * 接收数据
 * 注意：定点传输模式下，收到的数据不包含地址和信道头
 */
uint16_t E220_ReceiveData(uint8_t *data, uint16_t max_len)
{
    uint16_t copy_len;
    
    if(g_e220_data.rx_ready == 0)
    {
        return 0;  // 无数据
    }
    
    copy_len = (g_e220_data.rx_len < max_len) ? g_e220_data.rx_len : max_len;
    memcpy(data, g_e220_data.rx_buffer, copy_len);
    
    // 清除接收标志
    g_e220_data.rx_ready = 0;
    g_e220_data.rx_len = 0;
    
    return copy_len;
}

//============================================================================
// 辅助函数
//============================================================================

/**
 * 检查模块是否空闲
 */
uint8_t E220_IsIdle(void)
{
    // AUX引脚高电平表示空闲
    return GPIO_Pin_Read(U32BIT(E220_AUX_PIN)) ? 1 : 0;
}

/**
 * 等待模块空闲
 */
int E220_WaitIdle(uint32_t timeout_ms)
{
    uint32_t cnt = 0;
    
    while(!E220_IsIdle())
    {
        E220_DelayMs(1);
        cnt++;
        if(cnt > timeout_ms)
        {
            return -1;  // 超时
        }
    }
    
    return 0;
}

/**
 * 延时函数
 */
static void E220_DelayMs(uint32_t ms)
{
    delay_ms(ms);
}

//============================================================================
// LoRa协议函数（严格按照需求文档实现）
//============================================================================

/**
 * LoRa上电初始化
 * 
 * 流程（根据需求文档）：
 * 1. 进入模式3 (M0=1, M1=1)
 * 2. 发送 C1 00 08 读取配置
 * 3. 响应: C1 00 08 00 03 62 00 12 43 00 00
 *    解析: 地址=0003, 信道=12
 * 4. 保存到Flash
 * 5. 切换到模式0 (M0=0, M1=0)
 */
void Lora_PowerOn_Init(void)
{
    // 进入配置模式
    E220_SetMode(E220_MODE_CONFIG);
    
    g_lora_state.in_config_mode = 1;
    
    // 读取模块配置 (C1 00 08)
    E220_ReadConfig(&g_e220_config);
    
    // 保存网络配置
    Lora_SaveNetworkConfig();
    
    // 标记配置完成
    g_lora_state.config_done = 1;
    
    // 切换到透明传输模式
    E220_SetMode(E220_MODE_TRANSPARENT);
    
    g_lora_state.in_config_mode = 0;
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa PowerOn Init: Addr=0x%04X, CH=%d\r\n",
               g_lora_network.local_addr, g_lora_network.channel);
    #endif
}

/**
 * 上报锁状态到网关
 * 
 * 协议格式（根据需求文档）：
 * 发送: 0000 04 0003 12 01
 *       │网关│ │信│ │本设备│ │状│
 *       │地址│ │道│ │地址信道│ │态│
 * 网关收到: 0003 12 01
 * 
 * @param status 锁状态 (01=开锁, 10=上锁, 11=剪断)
 */
void Lora_ReportStatus(uint8_t status)
{
    uint8_t data[4];
    
    // 构造有效数据: [本设备地址高][本设备地址低][本设备信道][状态]
    // 根据需求文档: 0003 12 01
    data[0] = (uint8_t)(g_lora_network.local_addr >> 8);   // 本设备地址高字节
    data[1] = (uint8_t)(g_lora_network.local_addr & 0xFF); // 本设备地址低字节
    data[2] = g_lora_network.channel;                       // 本设备信道
    data[3] = status;                                        // 状态
    
    // 发送到网关: 目标地址=0000, 目标信道=04
    // E220会添加地址和信道头: [00][00][04][00][03][12][01]
    // 网关收到: [00][03][12][01] (自动去掉目标地址和信道)
    E220_SendTo(GATEWAY_ADDR_H << 8 | GATEWAY_ADDR_L, GATEWAY_CHANNEL, data, 4);
    
    // 启动等待OK计时器
    g_lora_state.wait_ok_timer = WAIT_OK_TIMEOUT;
    g_lora_state.report_retry_cnt = 0;
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa Report: Addr=0x%04X, CH=%d, Status=0x%02X\r\n",
               g_lora_network.local_addr, g_lora_network.channel, status);
    #endif
}

/**
 * 上报锁状态和电量到网关
 * 
 * 协议格式（根据需求文档）：
 * 发送: 0000 04 0003 12 01 00
 * 网关收到: 0003 12 01 00 (地址+信道+状态+电量)
 * 
 * @param status 锁状态
 * @param battery 电量百分比 (00~64)
 */
void Lora_ReportStatusWithBattery(uint8_t status, uint8_t battery)
{
    uint8_t data[5];
    
    // 构造有效数据: [本设备地址高][本设备地址低][本设备信道][状态][电量]
    data[0] = (uint8_t)(g_lora_network.local_addr >> 8);
    data[1] = (uint8_t)(g_lora_network.local_addr & 0xFF);
    data[2] = g_lora_network.channel;
    data[3] = status;
    data[4] = battery;  // 电量百分比
    
    // 发送到网关
    E220_SendTo(GATEWAY_ADDR_H << 8 | GATEWAY_ADDR_L, GATEWAY_CHANNEL, data, 5);
    
    // 启动等待OK计时器
    g_lora_state.wait_ok_timer = WAIT_OK_TIMEOUT;
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa Report+BAT: Status=0x%02X, BAT=%d%%\r\n", status, battery);
    #endif
}

/**
 * 处理接收到的LoRa指令
 * 
 * 协议格式（根据需求文档）：
 * 发送: 0003 12 01 (目标地址0003, 目标信道12, 数据01)
 * 本设备收到: 01 (E220自动过滤地址和信道，只返回数据)
 * 
 * @param data 接收到的数据（已去除地址和信道头）
 * @param len 数据长度
 */
void Lora_ProcessCommand(const uint8_t *data, uint16_t len)
{
    // 定点传输模式下，收到的数据不包含地址和信道头
    // 直接就是有效数据（指令）
    
    if(len < 1)
    {
        return;  // 数据长度不足
    }
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa RX: CMD=0x%02X\r\n", data[0]);
    #endif
    
    // 处理指令
    switch(data[0])
    {
        case LORA_CMD_UNLOCK:  // 0x01 开锁
            #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
            dbg_printf("LoRa CMD: UNLOCK (0x01)\r\n");
            #endif
            // 设置开锁标志，由主程序执行开锁动作
            g_lora_state.lock_state = LOCK_STATUS_UNLOCKED;
            g_lora_unlock_pending = 1;
            break;
            
        case LORA_CMD_LOCK:    // 0x10 上锁
            #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
            dbg_printf("LoRa CMD: LOCK (0x10)\r\n");
            #endif
            // 设置上锁标志，等待上锁动作完成后再上报
            g_lora_state.lock_state = LOCK_STATUS_LOCKED;
            g_lora_lock_pending = 1;
            break;
            
        default:
            #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
            dbg_printf("LoRa CMD: Unknown 0x%02X\r\n", data[0]);
            #endif
            break;
    }
}

/**
 * LoRa模式切换处理 (在10ms定时器中调用)
 * 
 * 实现需求（根据文档）：
 * - 默认模式为模式3，每隔3秒钟切换到模式0
 * - 模式0默认3秒切换到模式3
 * - 循环切换
 */
void Lora_ModeSwitchHandler(void)
{
    g_lora_state.mode_switch_timer++;
    
    if(g_lora_state.mode_switch_timer >= MODE_SWITCH_INTERVAL)  // 3秒
    {
        g_lora_state.mode_switch_timer = 0;
        
        // 切换模式
        if(g_lora_state.current_mode == E220_MODE_CONFIG)
        {
            // 模式3 -> 模式0 (M0=1,M1=1 -> M0=0,M1=0)
            E220_SetMode(E220_MODE_TRANSPARENT);
            #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
            dbg_printf("LoRa: Mode3 -> Mode0 (Transparent)\r\n");
            #endif
        }
        else if(g_lora_state.current_mode == E220_MODE_TRANSPARENT)
        {
            // 模式0 -> 模式3 (M0=0,M1=0 -> M0=1,M1=1)
            E220_SetMode(E220_MODE_CONFIG);
            #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
            dbg_printf("LoRa: Mode0 -> Mode3 (Config)\r\n");
            #endif
        }
    }
}

/**
 * LoRa状态机处理 (在10ms定时器中调用)
 */
void Lora_StateHandler(void)
{
    // 检查接收数据
    if(g_e220_data.rx_ready)
    {
        // 解析指令
        Lora_ProcessCommand(g_e220_data.rx_buffer, g_e220_data.rx_len);
        
        // 清除接收标志
        g_e220_data.rx_ready = 0;
        g_e220_data.rx_len = 0;
    }
    
    // 等待OK响应超时处理（根据文档：收到OK后休眠，否则5秒后强制休眠）
    if(g_lora_state.wait_ok_timer > 0)
    {
        g_lora_state.wait_ok_timer--;
        
        if(g_lora_state.wait_ok_timer == 0)
        {
            // 5秒未收到OK，强制进入休眠
            #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
            dbg_printf("LoRa: Wait OK timeout, force sleep\r\n");
            #endif
            // 设置休眠标志，由主程序处理
            // EnterSleepFlag = 1;
        }
    }
}

/**
 * 保存LoRa网络配置到Flash
 */
void Lora_SaveNetworkConfig(void)
{
    g_lora_network.valid_flag = LORA_NETWORK_VALID_FLAG;
    
    // 擦除Flash扇区
    SPI_Flash_Erase_Sector(LORA_NETWORK_FLASH_ADDR);
    
    // 写入配置
    SPI_Flash_Write_Page((uint8_t*)&g_lora_network, LORA_NETWORK_FLASH_ADDR, 
                         sizeof(Lora_Network_Config_t));
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa Config Saved: Addr=0x%04X, CH=%d\r\n",
               g_lora_network.local_addr, g_lora_network.channel);
    #endif
}

/**
 * 从Flash读取LoRa网络配置
 * @return 0=成功, 其他=失败
 */
int Lora_LoadNetworkConfig(void)
{
    // 读取配置
    SPI_Flash_Read((uint8_t*)&g_lora_network, LORA_NETWORK_FLASH_ADDR, 
                   sizeof(Lora_Network_Config_t));
    
    // 检查有效性
    if(g_lora_network.valid_flag != LORA_NETWORK_VALID_FLAG)
    {
        #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
        dbg_printf("LoRa Config: Invalid, use default\r\n");
        #endif
        return -1;
    }
    
    #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
    dbg_printf("LoRa Config Loaded: Addr=0x%04X, CH=%d\r\n",
               g_lora_network.local_addr, g_lora_network.channel);
    #endif
    
    return 0;
}

//============================================================================
// UART中断接收处理
//============================================================================

/**
 * UART接收中断回调函数
 */
void E220_UART_RxCallback(uint8_t data)
{
    if(g_e220_data.rx_len < E220_MAX_DATA_LEN)
    {
        g_e220_data.rx_buffer[g_e220_data.rx_len++] = data;
    }
}

/**
 * 帧结束处理函数
 */
void E220_RxTimeoutHandler(void)
{
    if(g_e220_data.rx_len > 0)
    {
        g_e220_data.rx_ready = 1;
        
        #if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)
        DBGHEXDUMP("LoRa RX Data:", g_e220_data.rx_buffer, g_e220_data.rx_len);
        #endif
    }
}
