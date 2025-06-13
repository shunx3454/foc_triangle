
#include "mt6701.h"
#include "spi.h"

#define MT6701_SPIX SPI1

__IO uint8_t magant_state = 0;

void mt6701_init(void)
{
    spi_configuration(MT6701_SPIX);
}

uint16_t mt6701_get_raw_data(void)
{
    while (spi_i2s_flag_get(MT6701_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
    ;
    spi_cs_low(MT6701_SPIX);
    spi_i2s_data_transmit(MT6701_SPIX, 0xFFFF);

    while (spi_i2s_flag_get(MT6701_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
        ;
    uint16_t data = spi_i2s_data_receive(MT6701_SPIX);
    while (spi_i2s_flag_get(MT6701_SPIX, SPI_I2S_BF_FLAG) != RESET)
        ;
    spi_cs_high(MT6701_SPIX);
    
    return data >> 2;
}

float mt6701_get_angle(void)
{
    return (float) mt6701_get_raw_data() * 3.834952e-4f;
}

uint8_t mt6701_get_state(void)
{
    return magant_state;
}