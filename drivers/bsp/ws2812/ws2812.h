#ifndef __BSP_WS2812_H_
#define __BSP_WS2812_H_
#include "at32f403a_407.h"


#define LED_STRIP_NUM (21)
extern uint16_t ws2812_tmr_pwm_duty[LED_STRIP_NUM * 24];


#ifdef __cplusplus
extern "C" {
#endif

void led_strip_init(void);
void led_strip_set(uint8_t led_num, uint8_t G, uint8_t R, uint8_t B);
void led_strip_fresh(void);
void led_strip_show(void);
void HSVtoRGB888(float h, float s, float v, uint8_t* r, uint8_t* g, uint8_t* b);
void led_strip_set_hsv(int num, float h, float s, float v);
void led_strip_off(void);

#ifdef __cplusplus
}
#endif
#endif