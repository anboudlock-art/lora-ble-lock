#include "DebugLog.h"
/*
 * lora_e220.c
 * E220-M900T22S LoRa Module Driver for SYD8811
 *
 * Hardware: UART1 (P0.05=RXD1, P0.06=TXD1), 9600 baud
 * Protocol: Transparent transmission mode (Mode 0)
 * Data format: [ADDR_H][ADDR_L][CH][DEVICE_ID_H][DEVICE_ID_L][TYPE][STATUS]
 *
 * BUG FIXES:
 *  BUG-1: Use GPIO_Pin_Set(PWR_4G) directly for power on (no re-mux)
 *  BUG-2: Add proper RX idle timeout countdown to set recvflag
 *  BUG-3: lost_cnt=30, senddelaytime_set=5, senddelaytime=2
 *  BUG-4: Optional E220 auto-config (LORA_E220_AUTO_CONFIG)
 *  BUG-5: Consume flg_cmd_gpsdata in lora_process() to send lock status
 */

#include "lora_e220.h"
#include "uart.h"
#include "gpio.h"
#include "DebugLog.h"
#include "delay.h"
#include "user_app.h"
#include "user.h"
#include "config.h"
#include "typedef.h"

static UART_CTRL_TYPE * UART_1_CTRL = ((UART_CTRL_TYPE *) UART_1_CTRL_BASE);

lora_ctrl_t lora_ctrl;
lora_rx_buf_t lora_rx_buf;
lora_tx_buf_t lora_tx_buf;

/* report_cnt was in Uart_4G.c, needed by input.c */
uint8_t report_cnt = 0;

extern uint8_t lock_state;

/* Forward declarations */
static void lora_uart_init(void);
static void lora_uart_write(uint8_t data);
static void lora_uart_write_buff(uint8_t *pData, uint8_t len);
static void lora_uart_read(uint8_t *pcnt, uint8_t *pbuf);

/***************************************************************************
 * UART1 Initialization for E220 (9600 baud, 8N1)
 ***************************************************************************/
static void lora_uart_init(void)
{
    /* UART1 pins: P0.05 = RXD1, P0.06 = TXD1 */
    PIN_CONFIG->PIN_5_SEL = PIN_SEL_UART_RXD1;
    PIN_CONFIG->PIN_6_SEL = PIN_SEL_UART_TXD1;
    PIN_CONFIG->PAD_5_INPUT_PULL_UP = 0;
    PIN_CONFIG->PAD_6_INPUT_PULL_UP = 0;

    UART_1_CTRL->CLK_SEL = 0;          /* 0=16M */
    UART_1_CTRL->BAUDSEL = UART_BAUD_9600;  /* E220 default: 9600 */
    UART_1_CTRL->FLOW_EN = UART_RTS_CTS_DISABLE;

    UART_1_CTRL->RX_INT_MASK = 0;      /* Enable RX interrupt */
    UART_1_CTRL->TX_INT_MASK = 1;      /* Disable TX interrupt */

    UART_1_CTRL->PAR_FAIL_INT_MASK = 1;
    UART_1_CTRL->par_rx_even = 0;
    UART_1_CTRL->par_rx_en = 0;
    UART_1_CTRL->par_tx_even = 0;
    UART_1_CTRL->par_tx_en = 0;

    /* Clear flags */
    UART_1_CTRL->RI = 0;
    UART_1_CTRL->TI = 0;
    UART_1_CTRL->PAR_FAIL = 1;
    UART_1_CTRL->RX_FLUSH = 1;

    /* Clear buffers */
    lora_rx_buf.cnt = 0;
    lora_rx_buf.delay = 0;
    lora_rx_buf.recvflag = 0;

    NVIC_EnableIRQ(UART1_IRQn);
    UART_1_CTRL->UART_EN = 1;

    dbg_printf("LoRa UART init OK\r\n");
}

/***************************************************************************
 * UART1 write single byte
 ***************************************************************************/
static void lora_uart_write(uint8_t data)
{
    UART_1_CTRL->TX_DATA = data;
    while(UART_1_CTRL->TI == 0);
    UART_1_CTRL->TI = 0;
}

/***************************************************************************
 * UART1 write buffer
 ***************************************************************************/
static void lora_uart_write_buff(uint8_t *pData, uint8_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
    {
        lora_uart_write(pData[i]);
    }
}

/***************************************************************************
 * UART1 read from RX FIFO
 ***************************************************************************/
static void lora_uart_read(uint8_t *pcnt, uint8_t *pbuf)
{
    uint8_t i = 0;
    volatile uint8_t dly = 0;

    while(!UART_1_CTRL->RX_FIFO_EMPTY)
    {
        *(pbuf + i) = UART_1_CTRL->RX_DATA;
        i++;
        dly++; dly++; dly++;  /* delay for 64M clock */
    }
    *pcnt = i;
}

/***************************************************************************
 * Set E220 working mode
 * mode 0: Normal (M0=0, M1=0) - transparent transmission
 * mode 1: WOR TX (M0=1, M1=0)
 * mode 2: WOR RX (M0=0, M1=1)
 * mode 3: Sleep/Config (M0=1, M1=1)
 ***************************************************************************/
void lora_set_mode(uint8_t mode)
{
    switch(mode)
    {
        case LORA_MODE_NORMAL:
            LORA_M0_LOW();
            LORA_M1_LOW();
            break;
        case LORA_MODE_WOR_TX:
            LORA_M0_HIGH();
            LORA_M1_LOW();
            break;
        case LORA_MODE_WOR_RX:
            LORA_M0_LOW();
            LORA_M1_HIGH();
            break;
        case LORA_MODE_SLEEP:
            LORA_M0_HIGH();
            LORA_M1_HIGH();
            break;
    }
    DelayMS(5);  /* Wait for mode switch */
}

/***************************************************************************
 * Configure E220 registers (address, channel, air rate, FIXED mode)
 * Must be called in sleep mode (M0=1, M1=1)
 ***************************************************************************/
static void lora_e220_config(void)
{
    uint8_t cfg_cmd[9];
    uint16_t timeout;

    /* Enter config mode */
    lora_set_mode(LORA_MODE_SLEEP);
    DelayMS(50);

    /* Wait AUX high */
    timeout = 500;
    while(!LORA_AUX_READ() && timeout > 0) { DelayMS(1); timeout--; }

    /* Write register command: C0 START_REG COUNT DATA... */
    cfg_cmd[0] = LORA_CMD_WRITE_REG;  /* C0 */
    cfg_cmd[1] = 0x00;                /* Start register = 0 */
    cfg_cmd[2] = 0x06;                /* 6 registers to write */
    cfg_cmd[3] = LORA_DEFAULT_ADDR_H; /* ADDH = 0x00 */
    cfg_cmd[4] = LORA_DEFAULT_ADDR_L; /* ADDL = 0x00 */
    cfg_cmd[5] = 0x62;                /* REG0: 9600baud/8N1/2.4k air */
    cfg_cmd[6] = 0x00;                /* REG1: 200B subpacket/22dBm */
    cfg_cmd[7] = LORA_DEFAULT_CH;     /* REG2: Channel 4 */
    cfg_cmd[8] = 0x40;                /* REG3: FIXED mode(bit6=1)/no relay/no LBT */

    lora_uart_write_buff(cfg_cmd, 9);

    /* Wait for response (AUX goes low then high) */
    DelayMS(100);
    timeout = 500;
    while(!LORA_AUX_READ() && timeout > 0) { DelayMS(1); timeout--; }

    dbg_printf("E220 cfg: ADDR=%02x%02x CH=%02x FIXED\r\n",
               cfg_cmd[3], cfg_cmd[4], cfg_cmd[7]);

    /* Back to normal mode */
    lora_set_mode(LORA_MODE_NORMAL);
    DelayMS(50);
}

/***************************************************************************
 * Initialize E220 LoRa module
 ***************************************************************************/
void lora_e220_init(void)
{
    uint16_t timeout;

    dbg_printf("LoRa E220 init...\r\n");

    /* BUG-1 FIX: Power on directly via PWR_4G (already configured as output by gpio_init) */
    GPIO_Pin_Set(U32BIT(PWR_4G));
    DelayMS(100);

    /* Configure M0, M1 as output */
    PIN_Set_GPIO(U32BIT(LORA_M0_PIN) | U32BIT(LORA_M1_PIN), PIN_SEL_GPIO);
    GPIO_Set_Output(U32BIT(LORA_M0_PIN) | U32BIT(LORA_M1_PIN));

    /* Configure AUX as input with pull-up */
    GPIO_Set_Input(U32BIT(LORA_AUX_PIN), 0);
    PIN_Pullup_Enable(T_QFN_48, U32BIT(LORA_AUX_PIN));

    /* Initialize UART first (needed for config mode if used) */
    lora_uart_init();

    /* Configure E220 address/channel/FIXED mode */
    lora_e220_config();

    /* Set to normal mode (data transmission) */
    lora_set_mode(LORA_MODE_NORMAL);

    /* Wait for AUX high (module ready) */
    timeout = 1000;
    while(!LORA_AUX_READ() && timeout > 0)
    {
        DelayMS(1);
        timeout--;
    }
    if(timeout == 0)
    {
        dbg_printf("LoRa AUX timeout!\r\n");
    }

    /* BUG-3 FIX: Initialize control structure with proper timing */
    lora_ctrl.state = LORA_STATE_READY;
    lora_ctrl.addr_h = LORA_DEFAULT_ADDR_H;
    lora_ctrl.addr_l = LORA_DEFAULT_ADDR_L;
    lora_ctrl.channel = LORA_DEFAULT_CH;
    lora_ctrl.senddelaytime = 2;      /* Send first report after 2 seconds */
    lora_ctrl.senddelaytime_set = 5;  /* 5 second report interval */
    lora_ctrl.lost_cnt = 30;          /* 30s auto-close if no gateway */

    /* Get device ID from BLE address */
    {
        lora_ctrl.device_id_h = _user.lock_sn8array.data8[2];
        lora_ctrl.device_id_l = _user.lock_sn8array.data8[3];
    }

    dbg_printf("LoRa E220 ready! ID:%02x%02x CH:%02x\r\n",
               lora_ctrl.device_id_h, lora_ctrl.device_id_l, lora_ctrl.channel);

    flg_4g_EN = 1;
}

/***************************************************************************
 * Deinitialize E220 (power off)
 ***************************************************************************/
void lora_e220_deinit(void)
{
    UART_1_CTRL->UART_EN = 0;
    NVIC_DisableIRQ(UART1_IRQn);
    GPIO_Pin_Clear(U32BIT(PWR_4G));  /* Power off via PWR_4G directly */
    lora_ctrl.state = LORA_STATE_IDLE;
    flg_4g_EN = 0;
}

/***************************************************************************
 * Send raw data via LoRa
 ***************************************************************************/
void lora_send_data(uint8_t *data, uint8_t len)
{
    uint16_t timeout;

    if(lora_ctrl.state != LORA_STATE_READY)
    {
        dbg_printf("LoRa not ready!\r\n");
        return;
    }

    /* Wait for AUX high (not busy) */
    timeout = 500;
    while(!LORA_AUX_READ() && timeout > 0)
    {
        DelayMS(1);
        timeout--;
    }

    dbg_printf("LoRa TX %d bytes\r\n", len);
    lora_uart_write_buff(data, len);
    lora_ctrl.state = LORA_STATE_SENDING;
}

/***************************************************************************
 * Send lock status via LoRa (transparent mode)
 * Format: [ADDR_H][ADDR_L][CH][DEVICE_ID_H][DEVICE_ID_L][TYPE][STATUS]
 ***************************************************************************/
void lora_send_status(uint8_t status)
{
    uint8_t frame[LORA_FRAME_SIZE];

    frame[0] = lora_ctrl.addr_h;
    frame[1] = lora_ctrl.addr_l;
    frame[2] = lora_ctrl.channel;
    frame[3] = lora_ctrl.device_id_h;
    frame[4] = lora_ctrl.device_id_l;
    frame[5] = LORA_TYPE_DEFAULT;
    frame[6] = status;

    dbg_printf("LoRa TX: %02x%02x %02x %02x%02x %02x %02x\r\n",
               frame[0], frame[1], frame[2], frame[3],
               frame[4], frame[5], frame[6]);

    lora_send_data(frame, LORA_FRAME_SIZE);
}

/***************************************************************************
 * LoRa process (call from Timer_2_callback every 2ms, replaces net4G_prc)
 ***************************************************************************/
void lora_process(void)
{
    /* Check if send is complete */
    if(lora_ctrl.state == LORA_STATE_SENDING)
    {
        if(LORA_AUX_READ())  /* AUX high = send complete */
        {
            lora_ctrl.state = LORA_STATE_READY;
            dbg_printf("LoRa TX done\r\n");
        }
    }

    /* BUG-2 FIX: RX idle timeout countdown to set recvflag */
    if(lora_rx_buf.delay > 0)
    {
        lora_rx_buf.delay--;
        if(lora_rx_buf.delay == 0 && lora_rx_buf.cnt > 0)
        {
            lora_rx_buf.recvflag = 1;
        }
    }

    /* Process received data */
    if(lora_rx_buf.recvflag)
    {
        lora_rx_buf.recvflag = 0;
        dbg_printf("LoRa RX %d bytes\r\n", lora_rx_buf.cnt);

        /* Gateway responded - reset lost counter to keep connection alive */
        lora_ctrl.lost_cnt = 30;

        /* TODO: Parse gateway commands here for future features */

        lora_rx_buf.cnt = 0;
    }

    /* BUG-5 FIX: Consume flg_cmd_gpsdata and send lock status via LoRa */
    if(flg_cmd_gpsdata)
    {
        flg_cmd_gpsdata = 0;
        if(lora_ctrl.state == LORA_STATE_READY)
        {
            lora_send_status(lock_state ? LORA_STATUS_OPEN : LORA_STATUS_CLOSE);
        }
    }
}

/***************************************************************************
 * UART1 RX callback (called from UART1_IRQHandler)
 ***************************************************************************/
void lora_uart_callback(void)
{
    uint8_t i, len, buf[4];

    lora_uart_read(&len, buf);

    for(i = 0; i < len; i++)
    {
        if(lora_rx_buf.cnt < LORA_MAX_BUF_SIZE)
        {
            lora_rx_buf.data[lora_rx_buf.cnt] = buf[i];
            lora_rx_buf.cnt++;
        }
    }
    lora_rx_buf.delay = 50;  /* 50ms idle timeout (25 calls at 2ms each) */
}

/***************************************************************************
 * UART1 IRQ Handler
 ***************************************************************************/
void UART1_IRQHandler(void)
{
    if((UART_1_CTRL->RI) == 1)
    {
        UART_1_CTRL->RI = 0;
        lora_uart_callback();
    }
}

/***************************************************************************
 * Compatibility wrappers for old 4G/e103w08b function names
 * input.c and other files still call these old names
 ***************************************************************************/
void e103w08b_init(void)
{
    lora_e220_init();
}
