#ifndef __SVPWM_H_
#define __SVPWM_H_

#include "at32f403a_407.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void  svpwm_update(void);
    float svpwm_phaseA_DutyH(void);
    float svpwm_phaseB_DutyH(void);
    float svpwm_phaseC_DutyH(void);
    float svpwm_phaseA_DutyL(void);
    float svpwm_phaseB_DutyL(void);
    float svpwm_phaseC_DutyL(void);
    void  svpwm_set_udc(float udc);
    void  svpwm_set_Uref1(float u_alpha, float u_beta);
    void  svpwm_set_Uref2(float u_mod, float radian);
    void  svpwm_set_voltage(float Ud, float Uq, float ElectricalAngle);

#ifdef __cplusplus
}
#endif

#endif