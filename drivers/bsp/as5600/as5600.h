#ifndef __AS5600_H_
#define __AS5600_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "at32f403a_407.h"

/* AS5600 REGISTERS LIST */
#define AS5600_ZMCO 0x00
#define AS5600_ZPOS_H 0x01
#define AS5600_ZPOS_L 0x02
#define AS5600_MPOS_H 0x03
#define AS5600_MPOS_L 0x04
#define AS5600_MANG_H 0X05
#define AS5600_MANG_L 0X06
#define AS5600_CONF_H 0x07
#define AS5600_CONF_L 0x08

#define AS5600_RAW_ANGLE_H 0X0C
#define AS5600_RAW_ANGLE_L 0X0D
#define AS5600_ANGLE_H 0X0E
#define AS5600_ANGLE_L 0X0F

#define AS5600_STATUS 0X0B
#define AS5600_AGC 0X1A
#define AS5600_MAGNITUDE_H 0X1B
#define AS5600_MAGNITUDE_L 0X1C
#define AS5600_BURN


void as5600_init(void);
int32_t as5600_read_raw_data(void);
float as5600_read_angle(void);
void as5600_reset_zero(void);
float as5600_get_speed(float period);
float as5600_get_position(void);
float as5600_get_angle_highspeed(void);

#ifdef __cplusplus
}
#endif

#endif