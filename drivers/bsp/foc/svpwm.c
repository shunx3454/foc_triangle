#include "svpwm.h"
#include "dsp/fast_math_functions.h"

#define SQRT3 1.7320508f

static float   Udc              = 12.0f;
static float   U_alpha          = 0.0f;
static float   U_beta           = 0.0f;
static uint8_t svpwmSectorIndex = 1;

static uint8_t svpwmSectorTable[]          = {2, 6, 1, 4, 3, 5};
static uint8_t svpwmTimerCompareTable[][3] = {
    {1, 2, 3}, {2, 1, 3}, {3, 1, 2}, {3, 2, 1}, {2, 3, 1}, {1, 3, 2},
};
static float   A;
static float   B;
static float   C;
static uint8_t X;
static uint8_t Y;
static uint8_t Z;
static uint8_t N;
static float   T1                         = 0;
static float   T2                         = 0;
static float   ABCphaseChangeTimeStart[3] = {0, 0, 0};

void svpwm_update(void)
{
    A = U_beta;
    B = (SQRT3 * U_alpha - U_beta) * 0.5f;
    C = -(SQRT3 * U_alpha + U_beta) * 0.5f;

    if (A > 0)
        X = 1;
    else
        X = 0;

    if (B > 0)
        Y = 1;
    else
        Y = 0;

    if (C > 0)
        Z = 1;
    else
        Z = 0;

    N = 4 * Z + 2 * Y + X;
    if (N == 0 || N == 7)
        N = 1;

    svpwmSectorIndex = svpwmSectorTable[N - 1];
    switch (svpwmSectorIndex)
    {
        case 1:
            T1 = (B / Udc);
            T2 = (A / Udc);
            break;
        case 2:
            T1 = (-B / Udc);
            T2 = (-C / Udc);
            break;
        case 3:
            T1 = (A / Udc);
            T2 = (C / Udc);
            break;
        case 4:
            T1 = (-A / Udc);
            T2 = (-B / Udc);
            break;
        case 5:
            T1 = (C / Udc);
            T2 = (B / Udc);
            break;
        case 6:
            T1 = (-C / Udc);
            T2 = (-A / Udc);
            break;

        default:
            break;
    }

    // 防止过调制，给定最大占空比，留出电流采样时间
    if (T1 + T2 > 0.9f)
    {
        float t = T1;
        T1      = 0.9f * T1 / (T1 + T2);
        T2      = 0.9f * T2 / (t + T2);
    }

    ABCphaseChangeTimeStart[0] = (1.0f - T1 - T2) * 0.25f;
    ABCphaseChangeTimeStart[1] = ABCphaseChangeTimeStart[0] + T1 * 0.5f;
    ABCphaseChangeTimeStart[2] = ABCphaseChangeTimeStart[1] + T2 * 0.5f;
}

float svpwm_phaseA_DutyH(void)
{
    return 1 - ABCphaseChangeTimeStart[svpwmTimerCompareTable[svpwmSectorIndex - 1][0] - 1] * 2.0f;
}
float svpwm_phaseB_DutyH(void)
{
    return 1 - ABCphaseChangeTimeStart[svpwmTimerCompareTable[svpwmSectorIndex - 1][1] - 1] * 2.0f;
}
float svpwm_phaseC_DutyH(void)
{
    return 1 - ABCphaseChangeTimeStart[svpwmTimerCompareTable[svpwmSectorIndex - 1][2] - 1] * 2.0f;
}
float svpwm_phaseA_DutyL(void)
{
    return ABCphaseChangeTimeStart[svpwmTimerCompareTable[svpwmSectorIndex - 1][0] - 1] * 2.0f;
}
float svpwm_phaseB_DutyL(void)
{
    return ABCphaseChangeTimeStart[svpwmTimerCompareTable[svpwmSectorIndex - 1][1] - 1] * 2.0f;
}
float svpwm_phaseC_DutyL(void)
{
    return ABCphaseChangeTimeStart[svpwmTimerCompareTable[svpwmSectorIndex - 1][2] - 1] * 2.0f;
}

void svpwm_set_udc(float udc)
{
    Udc = udc;
}
void svpwm_set_Uref1(float u_alpha, float u_beta)
{
    U_alpha = u_alpha;
    U_beta  = u_beta;
}

void svpwm_set_Uref2(float u_mod, float radian)
{
    U_beta  = u_mod * arm_sin_f32(radian);
    U_alpha = u_mod * arm_cos_f32(radian);
}

void svpwm_set_voltage(float Ud, float Uq, float ElectricalAngle)
{
    U_alpha = Ud * arm_cos_f32(ElectricalAngle) - Uq * arm_sin_f32(ElectricalAngle);
    U_beta  = Ud * arm_sin_f32(ElectricalAngle) + Uq * arm_cos_f32(ElectricalAngle);
}