#include "spi.h"

void spi_configuration(spi_type* SPIx)
{
    gpio_init_type gpio_init_struct;
    spi_init_type  spi_init_struct;
    gpio_default_para_init(&gpio_init_struct);

    if (SPIx == SPI1) // for magnetometer sensor
    {
        /* configure the CS pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
        gpio_init_struct.gpio_pins           = GPIO_PINS_4;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOA, &gpio_init_struct);

        gpio_bits_set(GPIOA, GPIO_PINS_4);

        /* configure the SCK pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_5;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOA, &gpio_init_struct);

        /* configure the MISO pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
        gpio_init_struct.gpio_pins           = GPIO_PINS_6;
        gpio_init_struct.gpio_pull           = GPIO_PULL_UP;
        gpio_init(GPIOA, &gpio_init_struct);

        gpio_pin_remap_config(SPI1_GMUX_0010, TRUE);

        /* configure param */
        spi_default_para_init(&spi_init_struct);
        spi_init_struct.transmission_mode      = SPI_TRANSMIT_FULL_DUPLEX;
        spi_init_struct.master_slave_mode      = SPI_MODE_MASTER;
        spi_init_struct.frame_bit_num          = SPI_FRAME_16BIT;
        spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
        spi_init_struct.mclk_freq_division     = SPI_MCLK_DIV_16;
        spi_init_struct.clock_polarity         = SPI_CLOCK_POLARITY_LOW;
        spi_init_struct.clock_phase            = SPI_CLOCK_PHASE_2EDGE;
        spi_init_struct.cs_mode_selection      = SPI_CS_SOFTWARE_MODE;
        spi_init(SPI1, &spi_init_struct);

        spi_enable(SPI1, TRUE);
    }
    else if (SPIx == SPI2) // for imu sensor
    {
        /* configure the CS pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_OUTPUT;
        gpio_init_struct.gpio_pins           = GPIO_PINS_12;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOB, &gpio_init_struct);

        gpio_bits_set(GPIOB, GPIO_PINS_12);

        /* configure the SCK pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_13;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOB, &gpio_init_struct);

        /* configure the MISO pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_INPUT;
        gpio_init_struct.gpio_pins           = GPIO_PINS_14;
        gpio_init_struct.gpio_pull           = GPIO_PULL_UP;
        gpio_init(GPIOB, &gpio_init_struct);

        /* configure the MOSI pin */
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_15;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOB, &gpio_init_struct);

        /* configure param */
        spi_default_para_init(&spi_init_struct);
        spi_init_struct.transmission_mode      = SPI_TRANSMIT_FULL_DUPLEX;
        spi_init_struct.master_slave_mode      = SPI_MODE_MASTER;
        spi_init_struct.frame_bit_num          = SPI_FRAME_8BIT;
        spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
        spi_init_struct.mclk_freq_division     = SPI_MCLK_DIV_8;
        spi_init_struct.clock_polarity         = SPI_CLOCK_POLARITY_HIGH;
        spi_init_struct.clock_phase            = SPI_CLOCK_PHASE_2EDGE;
        spi_init_struct.cs_mode_selection      = SPI_CS_SOFTWARE_MODE;
        spi_init(SPI2, &spi_init_struct);

        spi_enable(SPI2, TRUE);
    }
}

void spi_cs_high(spi_type* SPIx)
{
    if (SPIx == SPI1)
    {
        gpio_bits_set(GPIOA, GPIO_PINS_4);
    }
    else if (SPIx == SPI2)
    {
        gpio_bits_set(GPIOB, GPIO_PINS_12);
    }
}

void spi_cs_low(spi_type* SPIx)
{
    if (SPIx == SPI1)
    {
        gpio_bits_reset(GPIOA, GPIO_PINS_4);
    }
    else if (SPIx == SPI2)
    {
        gpio_bits_reset(GPIOB, GPIO_PINS_12);
    }
}
