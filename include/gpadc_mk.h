#ifndef _GPADC_MK_H_
#define _GPADC_MK_H_

#include "ARMCM0.h"

#define GPADC_CH_MAX	11

typedef enum{
	ONESHOT_MODE,
	AVE_MODE,
}GPADC_MODE;

extern GPADC_CTRL_TYPE * GPADC;

extern void GPADC_Init(GPADC_MODE adc_mode);
extern void GPADC_start(void);
extern void GPADC_stop(void);
extern void GPADC_channel_sel(uint8_t ch);
extern uint16_t GPADC_get_value(void);

extern void GPADC_disable(void);
//extern uint16_t ADC_get_value(void);

#endif

