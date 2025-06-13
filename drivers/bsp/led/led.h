#ifndef __BSP_LED_H_
#define __BSP_LED_H_

#include "at32f403a_407.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LED_GPIO_PORT GPIOB
#define LED_GPIO_PINS (GPIO_PINS_0 | GPIO_PINS_1)

    typedef enum
    {
        LED0 = GPIO_PINS_0,
        LED1 = GPIO_PINS_1,
    } LED_TypeDef;

    void led_init(void);
    void led_toggle(LED_TypeDef LEDx);
    void led_on(LED_TypeDef LEDx);
    void led_off(LED_TypeDef LEDx);

#ifdef __cplusplus
}
#endif

#endif