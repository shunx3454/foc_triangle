#include "ws2812.h"
#include "FreeRTOS.h"
#include "system.h"
#include "task.h"
#include "tmr.h"
#include <math.h>

#define _0_CODE_TMR_CCR_VAL (30)
#define _1_CODE_TMR_CCR_VAL (60)

uint16_t ws2812_tmr_pwm_duty[LED_STRIP_NUM * 24] __ALIGNED(8);

void led_strip_init(void)
{
    tmr_configuration(TMR1, 3, 100);
    for (uint32_t i = 0; i < LED_STRIP_NUM * 24; i++)
    {
        ws2812_tmr_pwm_duty[i] = _0_CODE_TMR_CCR_VAL;
    }
}

void led_strip_set(uint8_t led_num, uint8_t G, uint8_t R, uint8_t B)
{
    uint32_t i;
    if (led_num > LED_STRIP_NUM)
        return;
    // set G
    for (i = 0; i < 8; i++)
        ws2812_tmr_pwm_duty[led_num * 24 + i] = ((G << i) & 0x80) ? _1_CODE_TMR_CCR_VAL : _0_CODE_TMR_CCR_VAL;
    // set R
    for (i = 0; i < 8; i++)
        ws2812_tmr_pwm_duty[led_num * 24 + i + 8] = ((R << i) & 0x80) ? _1_CODE_TMR_CCR_VAL : _0_CODE_TMR_CCR_VAL;
    // set B
    for (i = 0; i < 8; i++)
        ws2812_tmr_pwm_duty[led_num * 24 + i + 16] = ((B << i) & 0x80) ? _1_CODE_TMR_CCR_VAL : _0_CODE_TMR_CCR_VAL;
}

void led_strip_fresh(void)
{
    tmr_channel_value_set(TMR1, TMR_SELECT_CHANNEL_1, 0);
    tmr_counter_value_set(TMR1, 0);
    DMA1_CHANNEL3->maddr = (uint32_t) (ws2812_tmr_pwm_duty);
    DMA1_CHANNEL3->dtcnt = 24 * LED_STRIP_NUM;

    // enable dma & counter transfer a led strip
    dma_channel_enable(DMA1_CHANNEL3, TRUE);
    tmr_counter_enable(TMR1, TRUE);
    while (dma_flag_get(DMA1_FDT3_FLAG) == RESET)
        ;
    dma_flag_clear(DMA1_FDT3_FLAG);

    // stop transfer to send stop frame to led strip
    tmr_counter_enable(TMR1, FALSE);
    dma_channel_enable(DMA1_CHANNEL3, FALSE);
    delay_ms(1);
}

void HSVtoRGB888(float h, float s, float v, uint8_t* r, uint8_t* g, uint8_t* b)
{
    float c       = v * s; // chroma
    float h_prime = fmodf(h / 60.0f, 6.0f);
    float x       = c * (1 - fabsf(fmodf(h_prime, 2) - 1));
    float r1, g1, b1;

    if (h_prime >= 0 && h_prime < 1)
    {
        r1 = c;
        g1 = x;
        b1 = 0;
    }
    else if (h_prime < 2)
    {
        r1 = x;
        g1 = c;
        b1 = 0;
    }
    else if (h_prime < 3)
    {
        r1 = 0;
        g1 = c;
        b1 = x;
    }
    else if (h_prime < 4)
    {
        r1 = 0;
        g1 = x;
        b1 = c;
    }
    else if (h_prime < 5)
    {
        r1 = x;
        g1 = 0;
        b1 = c;
    }
    else
    {
        r1 = c;
        g1 = 0;
        b1 = x;
    }

    float m = v - c;
    *r      = (uint8_t) ((r1 + m) * 255.0f + 0.5f);
    *g      = (uint8_t) ((g1 + m) * 255.0f + 0.5f);
    *b      = (uint8_t) ((b1 + m) * 255.0f + 0.5f);
}

void led_strip_set_hsv(int num, float h, float s, float v)
{
    uint8_t R, G, B;
    HSVtoRGB888(h, s, v, &R, &G, &B);
    led_strip_set(num, G, R, B);
}

void led_strip_off(void)
{
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 0, 0, 0);
    }
    led_strip_fresh();
}

void led_strip_show(void)
{
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 0, 100, 0);
        led_strip_fresh();
        vTaskDelay(50);
    }
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 0, 0, 0);
        led_strip_fresh();
        vTaskDelay(50);
    }
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 0, 0, 100);
        led_strip_fresh();
        vTaskDelay(50);
    }
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 0, 0, 0);
        led_strip_fresh();
        vTaskDelay(50);
    }
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 100, 0, 0);
        led_strip_fresh();
        vTaskDelay(50);
    }
    for (int i = 0; i < 21; i++)
    {
        led_strip_set(i, 0, 0, 0);
        led_strip_fresh();
        vTaskDelay(50);
    }
    // 三色盘出现
    for (int i = 0; i < 7; i++)
    {
        led_strip_set(i, 100, 0, 0);
        led_strip_set(i + 7, 0, 100, 0);
        led_strip_set(i + 14, 0, 0, 100);
        led_strip_fresh();
        vTaskDelay(50);
    }
    // 三色盘旋转
    for (int i = 0; i < 21 * 5; i++)
    {
        for (int l = 0; l < 7; l++)
        {
            led_strip_set((l + i) % 21, 100, 0, 0);
            led_strip_set((l + 7 + i) % 21, 0, 100, 0);
            led_strip_set((l + 14 + i) % 21, 0, 0, 100);
        }
        led_strip_fresh();
        vTaskDelay(50);
    }
    // 三色盘消失
    for (int i = 0; i < 7; i++)
    {
        led_strip_set(i, 0, 0, 0);
        led_strip_set(i + 7, 0, 0, 0);
        led_strip_set(i + 14, 0, 0, 0);
        led_strip_fresh();
        vTaskDelay(50);
    }
    // 呼吸灯 G+
    for (int i = 0; i < 100; i++)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set(l, i, 0, 0);
        }
        led_strip_fresh();
        vTaskDelay(25);
    }
    vTaskDelay(500);
    // 呼吸灯 G-
    for (int i = 100; i >= 0; i--)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set(l, i, 0, 0);
        }
        led_strip_fresh();
        vTaskDelay(25);
    }
    vTaskDelay(500);
    // 呼吸灯 R+
    for (int i = 0; i < 100; i++)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set(l, 0, i, 0);
        }
        led_strip_fresh();
        vTaskDelay(25);
    }
    vTaskDelay(500);
    // 呼吸灯 R-
    for (int i = 100; i >= 0; i--)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set(l, 0, i, 0);
        }
        led_strip_fresh();
        vTaskDelay(25);
    }
    vTaskDelay(500);
    // 呼吸灯 B+
    for (int i = 0; i < 100; i++)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set(l, 0, 0, i);
        }
        led_strip_fresh();
        vTaskDelay(25);
    }
    vTaskDelay(500);
    // 呼吸灯 B-
    for (int i = 100; i >= 0; i--)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set(l, 0, 0, i);
        }
        led_strip_fresh();
        vTaskDelay(25);
    }
    vTaskDelay(500);

    // 色环点亮
    for (int i = 0; i < 21; i++)
    {
        led_strip_set_hsv(i, i * 17, 1, 0.5);
        led_strip_fresh();
        vTaskDelay(50);
    }
    // 色环旋转5圈
    for (int i = 360 * 10; i > 0; i--)
    {
        for (int l = 0; l < 21; l++)
        {
            led_strip_set_hsv(l, (l * 17 + i) % 360, 1, 0.5);
        }
        led_strip_fresh();
        vTaskDelay(2);
    }
    // 色环熄灭
    for (int i = 0; i < 21; i++)
    {
        led_strip_set_hsv(i, 0, 1, 0.5);
        led_strip_fresh();
        vTaskDelay(50);
    }
}
