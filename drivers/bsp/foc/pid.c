
#include "pid.h"
#include <math.h>

void pid_init(pid_t* pid, float kp, float ki, float kd, float integral_max, float integral_detach)
{
    pid->kp              = kp;
    pid->ki              = ki;
    pid->kd              = kd;
    pid->integral_max    = integral_max;
    pid->pre_error       = 0.f;
    pid->integral        = 0.f;
    pid->output          = 0.f;
    pid->integral_detach = integral_detach;
}

void pid_update(pid_t* pid, float target, float measured)
{
    float error = target - measured;
    pid->integral += error;
    if (pid->integral > pid->integral_max)
    {
        pid->integral = pid->integral_max;
    }
    else if (pid->integral < -pid->integral_max)
    {
        pid->integral = -pid->integral_max;
    }
    if (pid->integral_detach != 0)
    {
        if (fabsf(error) > pid->integral_detach)
            pid->integral = 0;
    }
    pid->output    = pid->kp * error + pid->ki * pid->integral + pid->kd * (error - pid->pre_error);
    pid->pre_error = error;
}