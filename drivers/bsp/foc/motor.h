#ifndef ___MOTOR_H_
#define ___MOTOR_H_

#include "at32f403a_407.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern __IO float Ia, Ib, Ic, Id, Iq;
    extern __IO float dutyA, dutyB, dutyC;
    extern __IO float motor_angle;
    extern __IO float motor_velocity;

    void    motor_init(uint16_t tmr_div, uint16_t tmr_period, float udc);
    void    motor_set_udc(float udc);
    void    motor_set_voltage(float Ud, float Uq);
    void    motor_move(void);
    void    motor_align_zero(void);
    void    motor_current_colibration(void);
    void    motor_current_convert(void);
    void    motor_angle_speed_update(float period);

#ifdef __cplusplus
}
#endif

#endif