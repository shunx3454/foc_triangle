#include "qmi8658.h"
#include "i2c_application.h"
#include <stdio.h>

#define LOCAL_G (9.807f)

#define QMI8658_I2C_ADDR 0x6b
/* QMI8658 REGs */
/* Chip Information Register */
#define QMI8658_WHO_AM_I 0x00
#define QMI8658_REVISION_ID 0x01
/* Configuration Registers */
#define QMI8658_CTRL1 0x02
#define QMI8658_CTRL2 0x03
#define QMI8658_CTRL3 0x04
#define QMI8658_CTRL5 0x06
#define QMI8658_CTRL7 0x08
#define QMI8658_CTRL8 0x09
#define QMI8658_CTRL9 0x0A
/* Host Controlled Calibration Registers  */
#define QMI8658_CAL1_L 0x0B
#define QMI8658_CAL1_H 0x0C
#define QMI8658_CAL2_L 0x0D
#define QMI8658_CAL2_H 0x0E
#define QMI8658_CAL3_L 0x0F
#define QMI8658_CAL3_H 0x10
#define QMI8658_CAL4_L 0x11
#define QMI8658_CAL4_H 0x12
/* FIFO Registers */
#define QMI8658_FIFO_WTM_TH 0x13
#define QMI8658_FIFO_CTRL 0x14
#define QMI8658_FIFO_SMPL_CNT 0x15
#define QMI8658_FIFO_STATUS 0x16
#define QMI8658_FIFO_DATA 0x17
/* Status Registers */
#define QMI8658_STATUS_INT 0x2D
#define QMI8658_STATUS0 0x2E
#define QMI8658_STATUS1 0x2F
/* Timestamp Register */
#define QMI8658_TIMESTAMP_LOW 0x30
#define QMI8658_TIMESTAMP_MID 0x31
#define QMI8658_TIMESTAMP_HIGH 0x32
/* Data Output Registers */
#define QMI8658_TEMP_L 0x33
#define QMI8658_TEMP_H 0x34
#define QMI8658_AX_L 0x35
#define QMI8658_AX_H 0x36
#define QMI8658_AY_L 0x37
#define QMI8658_AY_H 0x38
#define QMI8658_AZ_L 0x39
#define QMI8658_AZ_H 0x3A
#define QMI8658_GX_L 0x3B
#define QMI8658_GX_H 0x3C
#define QMI8658_GY_L 0x3D
#define QMI8658_GY_H 0x3E
#define QMI8658_GZ_L 0x3F
#define QMI8658_GZ_H 0x40
/* COD Indication and General Purpose Registers */
#define QMI8658_COD_STATUS 0x46
#define QMI8658_dQW_L 0x49
#define QMI8658_dQW_H 0x4A
#define QMI8658_dQX_L 0x4B
#define QMI8658_dQX_H 0x4C
#define QMI8658_dQY_L 0x4D
#define QMI8658_dQY_H 0x4E
#define QMI8658_dQZ_L 0x4F
#define QMI8658_dQZ_H 0x50
#define QMI8658_dVX_L 0x51
#define QMI8658_dVX_H 0x52
#define QMI8658_dVY_L 0x53
#define QMI8658_dVY_H 0x54
#define QMI8658_dVZ_L 0x55
#define QMI8658_dVZ_H 0x56
/* Activity Detection Output Registers */
#define QMI8658_TAP_STATUS 0x59
#define QMI8658_STEP_CNT_LOW 0x5A
#define QMI8658_STEP_CNT_MIDL 0x5B
#define QMI8658_STEP_CNT_HIGH 0x5C
/* Reset Register */
#define QMI8658_RESET 0x60

i2c_handle_type qmi8658_i2c;
float acc_scal;
float gyro_scal;

void qmi8658_if_init(void)
{
    qmi8658_i2c.i2cx = I2C2;
    i2c_config(&qmi8658_i2c);
}

void qmi8658_delay_ms(uint32_t xms)
{
    extern void delay_ms(uint32_t);
    delay_ms(xms);
}

int8_t qmi8658_write_reg(uint8_t reg, uint8_t val)
{
    if (i2c_memory_write(&qmi8658_i2c, I2C_MEM_ADDR_WIDIH_8,
                         QMI8658_I2C_ADDR << 1, reg, &val, 1, 0x1000) == I2C_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int8_t qmi8658_read_reg(uint8_t reg, uint8_t *val)
{
    if (i2c_memory_read(&qmi8658_i2c, I2C_MEM_ADDR_WIDIH_8,
                        QMI8658_I2C_ADDR << 1, reg, val, 1, 0x1000) == I2C_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

int8_t qmi8658_read_regs(uint8_t reg_base, uint8_t* buf, uint8_t len)
{
    if(i2c_memory_read(&qmi8658_i2c, I2C_MEM_ADDR_WIDIH_8,
                        QMI8658_I2C_ADDR << 1, reg_base, buf, len, 0x1000) == I2C_OK)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void qmi8658_init(void)
{
    uint8_t val = 0;
    qmi8658_if_init();

    qmi8658_read_reg(QMI8658_WHO_AM_I, &val);
    if(val != 0x05)
    {
        printf("not find qmi8658\r\n");
        return;
    }
    else
    {
        printf("found qmi8658\r\n");
    }

    /* software reset */
    qmi8658_write_reg(QMI8658_RESET, 0x0B);
    /* wait 15ms */
    qmi8658_delay_ms(15);

    /* check  0x4D regist equal 0x80 if reset successful */
    qmi8658_read_reg(0x4D, &val);
    if(val != 0x80)
    {
        printf("qmi8658 reset fail\r\n");
        return;
    }

    /* read reversion id */
    qmi8658_read_reg(QMI8658_REVISION_ID, &val);
    printf("qmi8658 reversion id:%#x\r\n", val);
    /* CTRL1 */
    qmi8658_write_reg(QMI8658_CTRL1, 0x40);      /* ...,read register address auto-incress, Little-Endian,... */

    /* CTRL2:Acc config */
    acc_scal = 0.00048828125f;/* 16/32768 */
    qmi8658_write_reg(QMI8658_CTRL2, 0x33);     /* No Acc Self-Test, +-16g, ODR=1KHZ */

    /* CTRL3:Gyro config */
    gyro_scal = 0.0625f;/* 2048/32768 */
    qmi8658_write_reg(QMI8658_CTRL3, 0x73);     /* No Gyro Self-Test, +-2048dps, ODR=896HZ */

    /* CTRL5: Acc&Gyro Lowpass Filter */
    //qmi8658_write_reg(QMI8658_CTRL5, 0x0);      /* No Acc&Gyro Lowpass Filter */
    qmi8658_write_reg(QMI8658_CTRL5, 0x77);      /* 13.37% of ODR Filter for Acc & Gyro */

    /* CTRL8: Motion Detection Control */
    qmi8658_write_reg(QMI8658_CTRL8, 0x0);      /* ...,No Any Motion detection,... */

    /* CTRL7: Enable Sensors and Configure Data Reads */
    qmi8658_write_reg(QMI8658_CTRL7, 0x03);     /* ...,Enable Acc&Gyro */
}

int8_t qmi8658_is_data_update(void)
{
    uint8_t val = 0;
    qmi8658_read_reg(QMI8658_STATUS0, &val);
    return ((val & 0x03) == 0x03 ? 1 : 0);
}
/* Acc Unit:m/s^2 Gyro: dps */
void qmi8658_read_sensor_data(float acc[], float gyro[])
{
    uint8_t raw_data[12];
    qmi8658_read_regs(QMI8658_AX_L, raw_data, 12);
    acc[0] = *(int16_t*)&raw_data[0] * acc_scal * LOCAL_G;
    acc[1] = *(int16_t*)&raw_data[2] * acc_scal * LOCAL_G;
    acc[2] = *(int16_t*)&raw_data[4] * acc_scal * LOCAL_G;
    gyro[0] = *(int16_t*)&raw_data[6] * gyro_scal;
    gyro[1] = *(int16_t*)&raw_data[8] * gyro_scal;
    gyro[2] = *(int16_t*)&raw_data[10] * gyro_scal;
}