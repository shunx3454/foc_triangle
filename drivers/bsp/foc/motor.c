#include "motor.h"
#include "adc.h"
#include "arm_math.h"
#include "mt6701.h"
#include "svpwm.h"
#include "system.h"
#include "tmr.h"
#include <math.h>
#include <stdio.h>

#define _1_DIV_SQRT_3 0.57735027f            // 1/sqrt(3)
#define TWO_PI 6.2831853f                    // pi*2
#define POLE_PAIRS_NUM (7.0F)                // motor pole pairs
#define CURRENT_CALCU_SCAL (0.001611328125f) // 1/4096 * 3.3 / 50(current sensor amplifier) / 0.01(sampling resistor)
#define MAGNETIC_POLARITY (1.f)             // 电机与磁编码器极性

static __IO int32_t motor_ia_adc_offset = 0;
static __IO int32_t motor_ib_adc_offset = 0;
static __IO int32_t motor_ia_adc        = 0;
static __IO int32_t motor_ib_adc        = 0;

__IO float Ia, Ib, Ic, Id, Iq;
__IO float dutyA, dutyB, dutyC;
__IO float OffsetElectricalAngle  = 0;
__IO float motor_electrical_angle = 0;
__IO float motor_angle            = 0;
__IO float motor_velocity         = 0;

__IO const float Idq_lowpass_filter_alpha   = 0.3f;
__IO const float speed_lowpass_filter_alpha = 0.5f;

float normalize_angle(float x)
{
    while (x >= TWO_PI)
        x -= TWO_PI;
    while (x < 0.0f)
        x += TWO_PI;
    return x;
}

void motor_init(uint16_t tmr_div, uint16_t tmr_period, float udc)
{
    mt6701_init();
    adc_configuration();

    tmr_configuration(TMR4, tmr_div, tmr_period);
    motor_current_colibration();
    motor_set_udc(udc);
    motor_align_zero();

    tmr_interrupt_enable(TMR4, TMR_C1_INT, TRUE);
}

void motor_set_udc(float udc)
{
    svpwm_set_udc(udc);
}

void motor_set_voltage(float Ud, float Uq)
{
    svpwm_set_voltage(Ud, Uq, motor_electrical_angle);
    svpwm_update();
    dutyA = svpwm_phaseA_DutyH();
    dutyB = svpwm_phaseB_DutyH();
    dutyC = svpwm_phaseC_DutyH();
    // printf("%.4f, %.4f, %.4f\r\n", dutyA, dutyB, dutyC);
}

void motor_move(void)
{
    tmr_channel_value_set(TMR4, TMR_SELECT_CHANNEL_2, svpwm_phaseA_DutyL() * tmr_period_value_get(TMR4));
    tmr_channel_value_set(TMR4, TMR_SELECT_CHANNEL_3, svpwm_phaseB_DutyL() * tmr_period_value_get(TMR4));
    tmr_channel_value_set(TMR4, TMR_SELECT_CHANNEL_4, svpwm_phaseC_DutyL() * tmr_period_value_get(TMR4));
}

void motor_align_zero(void)
{
    motor_set_voltage(3, 0);
    motor_move();
    delay_ms(3000);
    OffsetElectricalAngle = mt6701_get_angle() * MAGNETIC_POLARITY;
    motor_set_voltage(0, 0);
    motor_move();
    printf("motor aligned\r\n");
    delay_ms(500);
}

void motor_current_colibration(void)
{
    motor_ia_adc_offset = 0;
    motor_ib_adc_offset = 0;
    for (int i = 0; i < 1000; i++)
    {
        // adc_preempt_software_trigger_enable(ADC1, TRUE);
        while ((adc_flag_get(ADC1, ADC_PCCE_FLAG) == RESET) || (adc_flag_get(ADC2, ADC_PCCE_FLAG) == RESET))
            ;
        adc_flag_clear(ADC1, ADC_PCCE_FLAG);
        adc_flag_clear(ADC2, ADC_PCCE_FLAG);
        motor_ia_adc_offset += adc_preempt_conversion_data_get(ADC1, ADC_PREEMPT_CHANNEL_1);
        motor_ib_adc_offset += adc_preempt_conversion_data_get(ADC2, ADC_PREEMPT_CHANNEL_1);
    }
    motor_ia_adc_offset /= 1000;
    motor_ib_adc_offset /= 1000;
    printf("Ia_adc_offset = %ld, Ib_adc_offset = %ld\r\n", motor_ia_adc_offset, motor_ib_adc_offset);
}

void motor_current_convert(void)
{
    {
        while ((adc_flag_get(ADC1, ADC_PCCE_FLAG) == RESET) || (adc_flag_get(ADC2, ADC_PCCE_FLAG) == RESET))
            ;
        adc_flag_clear(ADC1, ADC_PCCE_FLAG);
        adc_flag_clear(ADC2, ADC_PCCE_FLAG);
        motor_ia_adc = adc_preempt_conversion_data_get(ADC1, ADC_PREEMPT_CHANNEL_1);
        motor_ib_adc = adc_preempt_conversion_data_get(ADC2, ADC_PREEMPT_CHANNEL_1);
    }
    // 相电流极性！！！
    Ia = (float) (motor_ia_adc_offset - motor_ia_adc) * CURRENT_CALCU_SCAL; // Ia
    Ib = (float) (motor_ib_adc_offset - motor_ib_adc) * CURRENT_CALCU_SCAL; // Ib
    Ic = -(Ia + Ib);                                                        // Ic

    // Clarke and park transform
    float Ialpha = Ia;
    float Ibeta  = (Ia + 2.0f * Ib) * _1_DIV_SQRT_3;
    float d      = Ialpha * arm_cos_f32(motor_electrical_angle) + Ibeta * arm_sin_f32(motor_electrical_angle);  // Id
    float q      = -Ialpha * arm_sin_f32(motor_electrical_angle) + Ibeta * arm_cos_f32(motor_electrical_angle); // Iq

    // path to low pass filter
    Id = d * Idq_lowpass_filter_alpha + (1.0f - Idq_lowpass_filter_alpha) * Id;
    Iq = q * Idq_lowpass_filter_alpha + (1.0f - Idq_lowpass_filter_alpha) * Iq;
}

void motor_angle_speed_update(float period)
{
    static float last_angle      = 0;
    static float velocity_period = 0;

    // get motor angle
    motor_angle            = normalize_angle(mt6701_get_angle() * MAGNETIC_POLARITY - OffsetElectricalAngle);
    motor_electrical_angle = normalize_angle(motor_angle * POLE_PAIRS_NUM);

    // get motor angle speed
    velocity_period += period;
    if (velocity_period > 0.001f)
    {
        float velocity = motor_angle - last_angle;
        if (velocity > PI)
            velocity -= 2.0f * PI;
        else if (velocity < -PI)
            velocity += 2.0f * PI;
        velocity /= velocity_period;
        velocity_period = 0;

        motor_velocity = velocity * speed_lowpass_filter_alpha + (1.f - speed_lowpass_filter_alpha) * motor_velocity;
        last_angle = motor_angle;
    }
}