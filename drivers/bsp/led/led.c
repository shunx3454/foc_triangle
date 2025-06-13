#include "led.h"

void led_init(void)
{
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);

    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
    gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
    gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
    gpio_init_struct.gpio_pins           = LED_GPIO_PINS;
    gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
    gpio_init(LED_GPIO_PORT, &gpio_init_struct);
    gpio_bits_set(LED_GPIO_PORT, LED_GPIO_PINS);
}

void led_toggle(LED_TypeDef LEDx)
{
    if (LEDx == LED0)
    {
        LED_GPIO_PORT->odt ^= LED0;
    }
    else if (LEDx == LED1)
    {
        LED_GPIO_PORT->odt ^= LED1;
    }
}

void led_on(LED_TypeDef LEDx)
{
    if(LEDx == LED0)
    {
        gpio_bits_reset(LED_GPIO_PORT, LED0);
    }
    else if(LEDx == LED1)
    {
        gpio_bits_reset(LED_GPIO_PORT, LED1);
    }
}

void led_off(LED_TypeDef LEDx)
{
    if(LEDx == LED0)
    {
        gpio_bits_set(LED_GPIO_PORT, LED0);
    }
    else if(LEDx == LED1)
    {
        gpio_bits_set(LED_GPIO_PORT, LED1);
    }
}