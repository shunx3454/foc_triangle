file(GLOB BSP_SRC
    drivers/bsp/led/led.c
    drivers/bsp/uart/uart.c
    drivers/bsp/mt6701/mt6701.c
    # drivers/bsp/i2c/i2c.c
    drivers/bsp/spi/spi.c
    drivers/bsp/adc/adc.c
    drivers/bsp/tmr/tmr.c
    drivers/bsp/foc/*.c
    # drivers/bsp/oled/oled.c
    # drivers/bsp/oled/font.c
    drivers/bsp/icm42688p/icm42688p.c
    drivers/bsp/ws2812/ws2812.c
)



set(BSP_INC
    drivers/bsp/led
    drivers/bsp/uart
    drivers/bsp/mt6701
    # drivers/bsp/i2c
    drivers/bsp/spi
    drivers/bsp/adc
    drivers/bsp/tmr
    drivers/bsp/foc
    # drivers/bsp/oled
    drivers/bsp/icm42688p
    drivers/bsp/ws2812
)