#include "gpadc.h"
#include "string.h"

#include "DebugLog.h"
#include "battery.h"


/**********************************************************
    1. get vbat value
        struct adc_cfg_t cfg;
        uint16_t result, ref_vol;

        memset((void*)&cfg, 0, sizeof(cfg));
        cfg.src = ADC_TRANS_SOURCE_VBAT;
        cfg.ref_sel = ADC_REFERENCE_INTERNAL;
        cfg.int_ref_cfg = ADC_INTERNAL_REF_1_2;
        cfg.clk_sel = ADC_SAMPLE_CLK_24M_DIV13;
        cfg.clk_div = 0x3f;
        adc_init(&cfg);
        adc_enable(NULL, NULL, 0);

        adc_get_result(ADC_TRANS_SOURCE_VBAT, 0, &result);
				ref_vol = adc_get_ref_voltage(ADC_REFERENCE_INTERNAL);
        // vbat_vol = (result * 4 * ref_vol) / 1024 mV.
**************************************************************/				

// ADC voltage levels
#define BATT_ADC_LEVEL_3V           404
#define BATT_ADC_LEVEL_2D7V         2.7*483/3.6
#define BATT_ADC_LEVEL_2D5V         2.5*483/3.6
#define BATT_ADC_LEVEL_2V           268

static uint16_t battMinLevel = BATT_ADC_LEVEL_2V; // For VDD/3 measurements
static uint16_t battMaxLevel = BATT_ADC_LEVEL_3V; // For VDD/3 measurements

uint16_t adc_value;
//uint8_t adc_state;

static uint8_t BatLevel = 100;


uint16_t ADC_get_value(void)
{
	GPADC_start();
	
	while(GPADC->EVENTS==0);
	
	GPADC->EVENTS = 0;
	
	return GPADC->adc_data_hclk;	//·µ»ØÔ­Ê¼Êý¾Ý
}

/*********************************************************************
 * @fn      battMeasure
 *
 * @brief   Measure the battery level with the ADC and return
 *          it as a percentage 0-100%.
 * //by 2020.12.13
 * @return  Battery level.
 */
uint8_t battMeasure( void )
{
	uint8_t channel, percent=BatLevel;
//	uint32_t t;
	float val=0;
	
	 /**
   * Battery level conversion from ADC to a percentage:
   *
   * The maximum ADC value for the battery voltage level is 511 for a
   * 10-bit conversion.  The ADC value is references vs. 1.25v and
   * this maximum value corresponds to a voltage of 3.75v.
   *
   * For a coin cell battery 3.0v = 100%.  The minimum operating
   * voltage of the CC2540 is 2.0v so 2.0v = 0%.
   *
   * To convert a voltage to an ADC value use:
   * (v/3)/1.25 * 511 = adc
   * 3.0v = 409 ADC
   * 2.0v = 273 ADC
   *
   * We need to map ADC values from 409-273 to 100%-0%.
   * Normalize the ADC values to zero:
   * 409 - 273 = 136
   * And convert ADC range to percentage range:
   * percent/adc = 100/136 = 25/34
   * Resulting in the final equation, with round:
   * percent = ((adc - 273) * 25) + 33 / 34
   **/
	 
	//adc init
	GPADC_Init(ONESHOT_MODE);
	
	channel=8;
	GPADC_channel_sel(channel);
	
  // stup adc ¡¢enable adc to start AD-convert
	//GPADC_start();
	
	adc_value = ADC_get_value();
	
	// disable adc
	GPADC_disable();
	
	adc_value = adc_value&0xfffe;
	
	val=(float)adc_value*3.6/483;
	
#if defined(_DEBUG_) || defined(_SYD_RTT_DEBUG_)	
	dbg_printf("ch:%02d adc : %04x vat: %4.3f\r\n",channel,adc_value,val);
#endif
	
	// if (adc_value >= battMaxLevel)
	// {
	// 	percent = 100;
	// }
	// else if (adc_value <= battMinLevel)
	// {
	// 	percent = 0;
	// }
	// else
	// {
	// 	uint16_t range =  34;	//range = 136/4 = 34

	// 	// optional if you want to keep it even, otherwise just take floor of divide
	// 	// range += (range & 1);
	// 	//range /= 20; // divide by 10
		
	// 	percent = (uint8_t) ((((adc_value - battMinLevel) * 25) + (range - 1)) / range);
	// }

	if (adc_value > BATT_ADC_LEVEL_2D7V)
	{
		percent = 100;
	}
	else if (adc_value <= BATT_ADC_LEVEL_2D7V)
	{
		percent = 20;
    if(adc_value <= BATT_ADC_LEVEL_2D5V){
      percent = 0;
    }
	}

  return percent;
}

/*******************************************************************************
* Function Name  : GetBatCapacity
* Description    : Measure the battery level with the ADC and return a percentage.
* Input          : None. 
* Return         : Bat level.
*******************************************************************************/
uint8_t GetBatCapacity(void)
{
  uint8_t level;
  level = battMeasure();
  //If level has gone down
  if (level < BatLevel)
  {
    //Update level
    BatLevel = level;
  }  
  return level;
}




/********************** (C) COPYRIGHT 2020-12-13 ***********************/


