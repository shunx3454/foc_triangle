#ifndef __BSP_OLED_H_
#define __BSP_OLED_H_

#include "at32f403a_407.h"


#ifdef __cpluscplus
extern "C"
{
#endif

void oled_init(void);
void oled_fresh(void);
void oled_clear(void);
void oled_display_on(void);
void oled_display_off(void);
void oled_draw_point(uint8_t line, uint8_t row, uint8_t state);
void oled_draw_8points(uint8_t line, uint8_t row, uint8_t points);
int8_t oled_show_char(uint8_t li, uint8_t row, uint8_t font, unsigned char ch);
int8_t oled_show_string(uint8_t li, uint8_t row, uint8_t font, unsigned char* str);
void oled_show_num(uint8_t li, uint8_t row, uint8_t font, int32_t num, uint8_t len);
void oled_show_float(uint8_t li, uint8_t row, uint8_t font, float num, uint8_t integer_len, uint8_t decimal_len);


#ifdef __cpluscplus
}
#endif

#endif