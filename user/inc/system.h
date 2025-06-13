#ifndef __SYSTEM_H_
#define __SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* includes ------------------------------------------------------------------*/
#include "at32f403a_407.h"

void nvic_config(void);
void systick_config(void);
void system_clock_print(void);
void systick_handler(void);
void delay_ms(uint32_t delay);
void timebase_increase(void);
uint32_t timebase_get(void);

#ifdef __cplusplus
}
#endif


#endif