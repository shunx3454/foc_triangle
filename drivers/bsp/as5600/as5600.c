#include "as5600.h"
#include "i2c_application.h"
#include "system.h"
#include <stdio.h>

#define _PI 3.1415927f
#define ANGLE_RESOLUTION 0.00153435538f // 1/4095*2*PI
#define AVERAGE_CNT 20

#define AS5600_I2C_ADDR (0x36 << 1)
#define AS5600_I2C_TIMEOUT (0xFFFFFF)

static i2c_handle_type as5600_i2c;
static int32_t         angle_adc         = 0;
static int32_t         offset            = 0;
static int32_t         last_angle_adc    = 0;
static int32_t         speed_adc         = 0;
static int32_t         position_adc      = 0;
static int32_t         last_position_adc = 0;
static __IO uint8_t    angle_update_flag = 0;

uint8_t as5600_read_reg(uint8_t reg_addr)
{
    uint8_t         data;
    i2c_status_type i2c_res =
        i2c_memory_read(&as5600_i2c, I2C_MEM_ADDR_WIDIH_8, AS5600_I2C_ADDR, reg_addr, &data, 1, 1000);
    return (i2c_res == I2C_OK) ? data : -1;
}

int8_t as5600_read_regs(uint8_t reg, uint8_t* pdata, uint16_t len)
{
    i2c_status_type i2c_res =
        i2c_memory_read(&as5600_i2c, I2C_MEM_ADDR_WIDIH_8, AS5600_I2C_ADDR, reg, pdata, len, 1000);
    return (i2c_res == I2C_OK) ? 0 : -1;
}

int8_t as5600_write_reg(uint8_t reg_addr, uint8_t* pdata, uint16_t len)
{
    i2c_status_type i2c_res =
        i2c_memory_write(&as5600_i2c, I2C_MEM_ADDR_WIDIH_8, AS5600_I2C_ADDR, reg_addr, pdata, len, 1000);
    return (i2c_res == I2C_OK) ? 0 : -1;
}

void as5600_init(void)
{
    uint8_t data;
    /* as5600_i2c device */
    as5600_i2c.i2cx = I2C1;
    i2c_config(&as5600_i2c);

    printf("as5600 registers\r\n");
    printf("STATUS:%#X\r\n", as5600_read_reg(AS5600_STATUS));
    printf("AGC:%#X\r\n", as5600_read_reg(AS5600_AGC));
    printf("ZMCO:%#X\r\n", as5600_read_reg(AS5600_ZMCO));
    data = 0x08; // HYST:01
    if (as5600_write_reg(AS5600_CONF_L, &data, 1) == 0)
        delay_ms(5);
    else
        printf("as5600 Write failed\r\n");
    printf("CONF_L:%#X\r\n", as5600_read_reg(AS5600_CONF_L));
    data = 0x12; // SF:10 FTH:100
    if (as5600_write_reg(AS5600_CONF_H, &data, 1) == 0)
        delay_ms(5);
    else
        printf("as5600 Write failed\r\n");
    printf("CONF_H:%#X\r\n", as5600_read_reg(AS5600_CONF_H));
}

int32_t as5600_read_raw_data(void)
{
    uint8_t data[2];
    as5600_read_regs(AS5600_RAW_ANGLE_H, data, 2);
    return (int32_t) data[0] << 8 | data[1];
}

void as5600_reset_zero(void)
{
    offset = as5600_read_raw_data();
}

float as5600_read_angle(void)
{
    static uint16_t average_cnt = 0;
    int32_t         delta_angle;
    int32_t         angle_adc;

    angle_adc         = as5600_read_raw_data();
    angle_adc         = (angle_adc >= offset) ? (angle_adc - offset) : (angle_adc + 4096 - offset);
    angle_update_flag = 1;

    delta_angle    = angle_adc - last_angle_adc;
    last_angle_adc = angle_adc;
    if (delta_angle > 2000)
        delta_angle -= 4096;
    else if (delta_angle < -2000)
        delta_angle += 4096;

    position_adc += delta_angle;
    average_cnt++;
    if (average_cnt == AVERAGE_CNT)
    {
        average_cnt       = 0;
        speed_adc         = position_adc - last_position_adc;
        last_position_adc = position_adc;
    }
    return angle_adc * ANGLE_RESOLUTION;
}

float as5600_get_speed(float period)
{
    return speed_adc * ANGLE_RESOLUTION / (period * (float) AVERAGE_CNT);
}

float as5600_get_position(void)
{
    return position_adc * ANGLE_RESOLUTION;
}

float as5600_get_angle_highspeed(void)
{
    static int16_t angle_adc_update_count = 0;
    static int16_t angle_adc_increment    = 0;

    if (angle_update_flag == 1)
    {
        angle_update_flag = 0;

        if (last_angle_adc - angle_adc > 2048 && angle_adc_increment >= 0)
        {
            angle_adc_increment = angle_adc - last_angle_adc + 4096;
        }
        else if (angle_adc - last_angle_adc > 2048 && angle_adc_increment <= 0)
        {
            angle_adc_increment = angle_adc - last_angle_adc - 4096;
        }
        else
        {
            angle_adc_increment = angle_adc - last_angle_adc;
        }
        if (angle_adc_update_count == 0)
            angle_adc_update_count = 1;

        angle_adc_increment    = angle_adc_increment / angle_adc_update_count;
        angle_adc_update_count = 0;
    }
    else
    {
        angle_adc_update_count++;
    }
    return (angle_adc + angle_adc_increment * angle_adc_update_count) * ANGLE_RESOLUTION;
}