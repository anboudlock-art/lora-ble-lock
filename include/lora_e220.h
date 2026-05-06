/*
 * lora_e220.h
 * E220-M900T22S LoRa Module Driver for SYD8811
 * Based on hardware schematic LORA_ZLS8811_V101216
 */

#ifndef LORA_E220_H_
#define LORA_E220_H_

#include "stdint.h"
#include "stdbool.h"
#include "ARMCM0.h"

/* E220 GPIO Pin Definitions (from schematic) */
#define LORA_EN_PIN     GPIO_18     /* E220 Enable/Power (same position as PWR_4G) */
#define LORA_M0_PIN     GPIO_14     /* Mode control M0 */
#define LORA_M1_PIN     GPIO_12     /* Mode control M1 */
#define LORA_AUX_PIN    GPIO_13     /* AUX busy/idle indicator */

/* UART1 pins: P0.05=RXD1(from E220 TX), P0.06=TXD1(to E220 RX) */

/* E220 Working Modes */
#define LORA_MODE_NORMAL    0   /* M0=0, M1=0: Normal transmission */
#define LORA_MODE_WOR_TX    1   /* M0=1, M1=0: WOR transmit */
#define LORA_MODE_WOR_RX    2   /* M0=0, M1=1: WOR receive */
#define LORA_MODE_SLEEP     3   /* M0=1, M1=1: Sleep/Config mode */

/* E220 Config Commands */
#define LORA_CMD_READ_REG   0xC1
#define LORA_CMD_WRITE_REG  0xC0

/* LoRa Data Frame Format (from requirements doc) */
/* TX: [ADDR_H] [ADDR_L] [CH] [DEVICE_ID_H] [DEVICE_ID_L] [TYPE] [STATUS] */
#define LORA_FRAME_SIZE     7
#define LORA_DEFAULT_ADDR_H 0x00
#define LORA_DEFAULT_ADDR_L 0x00
#define LORA_DEFAULT_CH     0x04

/* Status codes */
#define LORA_STATUS_OPEN    0x01    /* Lock opened */
#define LORA_STATUS_CLOSE   0x10    /* Lock closed */
#define LORA_STATUS_QUERY   0x11    /* Query status */
#define LORA_STATUS_BATTERY 0x00    /* Battery report */

/* Type = 0x12 (from requirements) */
#define LORA_TYPE_DEFAULT   0x12

/* LoRa state machine */
#define LORA_STATE_IDLE         0
#define LORA_STATE_INIT         1
#define LORA_STATE_READY        2
#define LORA_STATE_SENDING      3
#define LORA_STATE_WAITING      4
#define LORA_STATE_ERROR        5

/* Rx/Tx buffers */
#define LORA_MAX_BUF_SIZE   64

typedef struct {
    uint8_t data[LORA_MAX_BUF_SIZE];
    uint16_t cnt;
    uint16_t delay;
    uint8_t recvflag;
} lora_rx_buf_t;

typedef struct {
    uint8_t data[LORA_MAX_BUF_SIZE];
    uint16_t length;
} lora_tx_buf_t;

typedef struct {
    uint8_t state;
    uint8_t device_id_h;
    uint8_t device_id_l;
    uint8_t channel;
    uint8_t addr_h;
    uint8_t addr_l;
    uint8_t senddelaytime;
    uint8_t senddelaytime_set;
    uint8_t lost_cnt;
    uint8_t send_type;
    uint8_t send_status;
} lora_ctrl_t;

extern lora_ctrl_t lora_ctrl;
extern lora_rx_buf_t lora_rx_buf;
extern lora_tx_buf_t lora_tx_buf;

/* API Functions */
extern void lora_e220_init(void);
extern void lora_e220_deinit(void);
extern void lora_set_mode(uint8_t mode);
extern void lora_send_data(uint8_t *data, uint8_t len);
extern void lora_send_status(uint8_t status);
extern void lora_process(void);
extern void lora_uart_callback(void);

/* Power control */
#define LORA_POWER_ON()     GPIO_Pin_Set(U32BIT(LORA_EN_PIN))
#define LORA_POWER_OFF()    GPIO_Pin_Clear(U32BIT(LORA_EN_PIN))

/* Mode control */
#define LORA_M0_HIGH()      GPIO_Pin_Set(U32BIT(LORA_M0_PIN))
#define LORA_M0_LOW()       GPIO_Pin_Clear(U32BIT(LORA_M0_PIN))
#define LORA_M1_HIGH()      GPIO_Pin_Set(U32BIT(LORA_M1_PIN))
#define LORA_M1_LOW()       GPIO_Pin_Clear(U32BIT(LORA_M1_PIN))

/* AUX read */
#define LORA_AUX_READ()     (GPIO_Pin_Read(U32BIT(LORA_AUX_PIN)) ? 1 : 0)

#endif /* LORA_E220_H_ */
