
file(GLOB AT32F403A_DRIVER_SRC
    startup_at32f403a_407.s
    drivers/libraries/cmsis/cm4/device_support/system_at32f403a_407.c
    drivers/libraries/drivers/src/at32f403A_407_debug.c
    drivers/libraries/drivers/src/at32f403A_407_dma.c
    drivers/libraries/drivers/src/at32f403A_407_flash.c
    drivers/libraries/drivers/src/at32f403A_407_gpio.c
    drivers/libraries/drivers/src/at32f403A_407_crm.c
    drivers/libraries/drivers/src/at32f403A_407_pwc.c
    drivers/libraries/drivers/src/at32f403A_407_usart.c
    drivers/libraries/drivers/src/at32f403A_407_misc.c
    drivers/libraries/drivers/src/at32f403A_407_spi.c
    drivers/libraries/drivers/src/at32f403A_407_tmr.c
    drivers/libraries/drivers/src/at32f403A_407_sdio.c
    drivers/libraries/drivers/src/at32f403A_407_i2c.c
    drivers/libraries/drivers/src/at32f403A_407_adc.c
)



set(AT32F403A_DRIVER_INC 
    drivers/libraries/drivers/inc
    drivers/libraries/cmsis/cm4/core_support
    drivers/libraries/cmsis/cm4/device_support
)