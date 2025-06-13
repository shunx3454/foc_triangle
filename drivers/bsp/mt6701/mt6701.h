#ifndef __BSP_MT6701_H_
#define __BSP_MT6701_H_

#include "at32f403a_407.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern __IO uint8_t magant_state;

void mt6701_init(void);
uint16_t mt6701_get_raw_data(void);
float mt6701_get_angle(void);
uint8_t mt6701_get_state(void);

#ifdef __cplusplus
}
#endif

#endif