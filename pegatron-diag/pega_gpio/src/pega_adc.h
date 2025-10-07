#ifndef _PEGA_ADC_H_
#define _PEGA_ADC_H_

#include "pega_defines.h"

#ifdef __cplusplus
extern "C" {
#endif
//==============================================================================
//0~3.3v (10bits ADC)
typedef enum 
{
  SAR_ADC_CH0 = 0,    //Check Bclk output whether is ready; (DV1)
  SAR_ADC_CH1,		  //Main board temperature detection
  SAR_ADC_CH2,		  //To detect 5V is ready (乘以 0.0147 就是VBAT 電壓值)
  SAR_ADC_CH3,		  //VAC detection
  SAR_ADC_CH_MAX,
}SarADCEnumType;

typedef struct
{
    int channel_value;
	int adc_value;
}ADC_CONFIG_READ_ADC;
//==============================================================================
#define ADC_CC1_CH    SAR_ADC_CH0
#define ADC_CC2_CH    SAR_ADC_CH1
//==============================================================================
int  Pega_SarADC_init(void);
int  Pega_SarADC_uninit(void);
int  Pega_SarADC_Value_Get(const int eChannel, int *val);
void Pega_SarADC_Value_Print(const int eChannel);
//==============================================================================

#ifdef __cplusplus
}
#endif
#endif //_PEGA_ADC_H_