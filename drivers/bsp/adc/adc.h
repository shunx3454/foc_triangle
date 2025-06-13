#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "at32f403a_407.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define ADC_ORDINARY_CHANNEL_MAX_NUM  1

extern __IO uint32_t adccom_ordinary_complete;
extern uint32_t adccom_ordinary_valuetab[ADC_ORDINARY_CHANNEL_MAX_NUM];

void adc_configuration(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADC_H */