#include "i2c.h"
#include "i2c_application.h"

void i2c_lowlevel_init(i2c_handle_type* hi2c)
{
    gpio_init_type gpio_init_struct;
    gpio_default_para_init(&gpio_init_struct);

    if (hi2c->i2cx == I2C1) // for as5600
    {
        /* configure the i2c1 sda & scl pin */
        /* PB9 => I2C1_SDA */
        /* PB8 => I2C1_SCL */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_8 | GPIO_PINS_9;
        gpio_init_struct.gpio_pull           = GPIO_PULL_UP;
        gpio_init(GPIOB, &gpio_init_struct);

        gpio_pin_remap_config(I2C1_GMUX_0001, TRUE);

        /* 120MHZ clock source，config i2c2 400KMHZ */
        i2c_init(I2C1, I2C_FSMODE_DUTY_16_9, 400000);
    }
    else if (hi2c->i2cx == I2C2) // for ssd1306 oled
    {
        /* configure the i2c2 sda & scl pin */
        /* PB11 => I2C2_SDA */
        /* PB10 => I2C2_SCL */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_10 | GPIO_PINS_11;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOB, &gpio_init_struct);

        /* 120MHZ clock source，config i2c2 400KMHZ */
        i2c_init(I2C2, I2C_FSMODE_DUTY_16_9, 400000);
    }
}