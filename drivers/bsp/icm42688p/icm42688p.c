#include "icm42688p.h"
#include "spi.h"
#include "system.h"
#include <math.h>
#include <stdio.h>

#define ICM42688P_SPIX (SPI2)

static float   temp;
static int16_t AccX_ADC;
static int16_t AccY_ADC;
static int16_t AccZ_ADC;
static int16_t GyroX_ADC;
static int16_t GyroY_ADC;
static int16_t GyroZ_ADC;

static int16_t biasGX, biasGY, biasGZ;

static float acc_full_scal;
static float gyro_full_scal;

void icm42688p_write_reg(uint8_t reg, uint8_t data)
{
    spi_cs_low(ICM42688P_SPIX);

    // send reg addr
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(ICM42688P_SPIX, reg);
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
        ;
    spi_i2s_data_receive(ICM42688P_SPIX);

    // send data
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(ICM42688P_SPIX, data);
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
        ;
    spi_i2s_data_receive(ICM42688P_SPIX);

    // end spi
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_BF_FLAG) != RESET)
        ;
    spi_cs_high(ICM42688P_SPIX);
}

uint8_t icm42688p_read_reg(uint8_t reg)
{
    uint8_t data;
    spi_cs_low(ICM42688P_SPIX);

    // send reg addr
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(ICM42688P_SPIX, reg | 0x80);
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
        ;
    spi_i2s_data_receive(ICM42688P_SPIX);

    // receive data
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(ICM42688P_SPIX, 0xff);
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
        ;
    data = spi_i2s_data_receive(ICM42688P_SPIX);

    // end spi
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_BF_FLAG) != RESET)
        ;
    spi_cs_high(ICM42688P_SPIX);
    return data;
}

void icm42688p_burst_read_reg(uint8_t reg, uint8_t* pdata, uint16_t len)
{
    spi_cs_low(ICM42688P_SPIX);

    // send reg addr
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
        ;
    spi_i2s_data_transmit(ICM42688P_SPIX, reg | 0x80);
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
        ;
    spi_i2s_data_receive(ICM42688P_SPIX);

    // receive datas
    while (len--)
    {
        while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_TDBE_FLAG) == RESET)
            ;
        spi_i2s_data_transmit(ICM42688P_SPIX, 0xff);
        while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_RDBF_FLAG) == RESET)
            ;
        *pdata++ = spi_i2s_data_receive(ICM42688P_SPIX);
    }

    // end spi
    while (spi_i2s_flag_get(ICM42688P_SPIX, SPI_I2S_BF_FLAG) != RESET)
        ;
    spi_cs_high(ICM42688P_SPIX);
}

int power(int m, int n)
{
    int res = 1;
    while (n--)
    {
        res *= m;
    }
    return res;
}

void icm42688p_read_raw_data(void)
{
    uint8_t dat[14];
    icm42688p_burst_read_reg(ICM42688P_TEMP_DATA1, dat, 14);
    temp      = (float) ((((int16_t) dat[0] << 8) | dat[1]) / 132.48f) + 25.f;
    AccX_ADC  = ((int16_t) dat[2] << 8) | dat[3];
    AccY_ADC  = ((int16_t) dat[4] << 8) | dat[5];
    AccZ_ADC  = ((int16_t) dat[6] << 8) | dat[7];
    GyroX_ADC = ((int16_t) dat[8] << 8) | dat[9];
    GyroY_ADC = ((int16_t) dat[10] << 8) | dat[11];
    GyroZ_ADC = ((int16_t) dat[12] << 8) | dat[13];
}

void icm42688p_init(void)
{
    int32_t biasGX_ADC = 0, biasGY_ADC = 0, biasGZ_ADC = 0;
    spi_configuration(ICM42688P_SPIX);

    // reset sensor
    icm42688p_write_reg(ICM42688P_DEVICE_CONFIG, 0x01);
    delay_ms(5);

    // who am i
    if (icm42688p_read_reg(ICM42688P_WHO_AM_I) != 0x47)
    {
        printf("Not found icm42688p!\r\n");
        return;
    }
    printf("Found icm42688p!\r\n");

    // power on
    icm42688p_set_bank(BANK0);
    icm42688p_start();

    // set full scal
    icm42688p_acc_config(ACCEL_FS_16G, ACCEL_ODR_SEL_2KHZ);
    icm42688p_gyro_config(GYRO_FS_SEL_2000DPS, GYRO_ODR_SEL_2KHZ);
    delay_ms(20);

    // set ui filter
    icm42688p_set_lowpass_filter(ODR_2nd, BW15);
    delay_ms(20);

    // set filter
    icm42688p_set_Notch_filter(1000, GYRO_NF_BW_1499HZ);
    icm42688p_set_Anti_alias_filter(170);
    delay_ms(20);

    icm42688p_set_bank(BANK0);

    for (int32_t i = 0; i < 1000; i++)
    {
        icm42688p_read_raw_data();
        biasGX_ADC += GyroX_ADC;
        biasGY_ADC += GyroY_ADC;
        biasGZ_ADC += GyroZ_ADC;
        delay_ms(1);
    }
    biasGX = biasGX_ADC / 1000;
    biasGY = biasGY_ADC / 1000;
    biasGZ = biasGZ_ADC / 1000;
}

void icm42688p_start(void)
{
    // Temperature sensor enable
    // gyroscope in Low Noise (LN) Mode
    // accelerometer in Low Noise (LN) Mode
    icm42688p_write_reg(ICM42688P_PWR_MGMT0, 0x0F);
    delay_ms(20);
}

void icm42688p_gyro_config(GYRO_FS_SEL gyro_fs_sel, GYRO_ODR_SEL gyro_odr_sel)
{
    gyro_full_scal = 2000.f / (float) power(2, gyro_fs_sel);
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG0, gyro_fs_sel << 5 | gyro_odr_sel);
}

void icm42688p_acc_config(ACCEL_FS_SEL acc_fs_sel, ACCEL_ODR_SEL acc_odr_sel)
{
    acc_full_scal = 16.f / (float) power(2, acc_fs_sel);
    icm42688p_write_reg(ICM42688P_ACCEL_CONFIG0, acc_fs_sel << 5 | acc_odr_sel);
}

void icm42688p_set_bank(BANK_SEL bank_sel)
{
    icm42688p_write_reg(ICM42688P_REG_BANK_SEL, bank_sel);
}

void icm42688p_set_lowpass_filter(UI_FILTER_ODR odr, UI_FILTER_BW bw)
{
    uint8_t reg = icm42688p_read_reg(ICM42688P_GYRO_CONFIG1);
    reg &= ~0x0c;
    reg |= odr << 2;
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG1, reg);
    reg = icm42688p_read_reg(ICM42688P_ACCEL_CONFIG1);
    reg &= ~0x18;
    reg |= odr << 1;
    icm42688p_write_reg(ICM42688P_ACCEL_CONFIG1, reg);

    reg = 0;
    reg |= bw << 4 | bw;
    icm42688p_write_reg(ICM42688P_GYRO_ACCEL_CONFIG0, reg);
}

void icm42688p_set_Notch_filter(uint32_t nf_freq, GYRO_NF_BW bw)
{
    __IO int16_t NF_COSWZ;
    __IO int16_t NF_COSWZ_SEL;
    __IO uint8_t GYRO_CONFIG_STATIC9_reg;

    icm42688p_set_bank(BANK3);
    __IO uint8_t CLKDIV = icm42688p_read_reg(ICM42688P_CLKDIV);
    float        Fdrv   = 1.92e6f / (float) CLKDIV;
    float        COSWZ  = cos(2 * M_PI * nf_freq / Fdrv);

    if (fabsf(COSWZ) <= 0.875f)
    {
        NF_COSWZ_SEL = 0;
        NF_COSWZ     = round(COSWZ * 256);
    }
    else
    {
        NF_COSWZ_SEL = 1;
        if (COSWZ > 0.875)
            NF_COSWZ = round(8 * (1 - COSWZ) * 256);
        else
            NF_COSWZ = round(-8 * (1 + COSWZ) * 256);
    }

    icm42688p_set_bank(BANK1);
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC6, NF_COSWZ & 0xff);
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC7, NF_COSWZ & 0xff);
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC8, NF_COSWZ & 0xff);
    GYRO_CONFIG_STATIC9_reg = icm42688p_read_reg(ICM42688P_GYRO_CONFIG_STATIC9);
    if (NF_COSWZ & 0x100)
    {
        GYRO_CONFIG_STATIC9_reg |= 0x3;
    }
    else
    {
        GYRO_CONFIG_STATIC9_reg &= ~0x3;
    }
    if (NF_COSWZ_SEL)
    {
        GYRO_CONFIG_STATIC9_reg |= 0x1c;
    }
    else
    {
        GYRO_CONFIG_STATIC9_reg &= ~0x1c;
    }
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC9, GYRO_CONFIG_STATIC9_reg);
    icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC10, bw << 4);
}

void icm42688p_set_Anti_alias_filter(uint32_t aaf_bw)
{
    uint8_t AAF_DELT;
    uint8_t AAF_DELTSQR;
    uint8_t AAF_BITSHIFT;
    if (aaf_bw == 170)
    {
        AAF_DELT     = 4;
        AAF_DELTSQR  = 16;
        AAF_BITSHIFT = 11;

        // set gyor aaf
        icm42688p_set_bank(BANK1);
        icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC3, AAF_DELT);
        icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC4, AAF_DELTSQR & 0xff);
        icm42688p_write_reg(ICM42688P_GYRO_CONFIG_STATIC5, (AAF_BITSHIFT << 4) | ((AAF_DELTSQR >> 8) & 0xf));

        // set acc aaf
        icm42688p_set_bank(BANK2);
        icm42688p_write_reg(ICM42688P_ACCEL_CONFIG_STATIC2, AAF_DELT << 1);
        icm42688p_write_reg(ICM42688P_ACCEL_CONFIG_STATIC3, AAF_DELTSQR & 0xff);
        icm42688p_write_reg(ICM42688P_ACCEL_CONFIG_STATIC4, (AAF_BITSHIFT << 4) | ((AAF_DELTSQR >> 8) & 0xf));
    }
}

void icm42688p_get_data(float* _temp, float* AccX, float* AccY, float* AccZ, float* GyroX, float* GyroY, float* GyroZ)
{
    icm42688p_read_raw_data();
    *_temp = temp;
    *AccX  = (float) (AccX_ADC) * (acc_full_scal / (float) 32767);
    *AccY  = (float) (AccY_ADC) * (acc_full_scal / (float) 32767);
    *AccZ  = (float) (AccZ_ADC) * (acc_full_scal / (float) 32767);
    *GyroX = ((float) (GyroX_ADC - biasGX) * (gyro_full_scal / (float) 32767));
    *GyroY = ((float) (GyroY_ADC - biasGY) * (gyro_full_scal / (float) 32767));
    *GyroZ = ((float) (GyroZ_ADC - biasGZ) * (gyro_full_scal / (float) 32767));
}
