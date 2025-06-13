#ifndef __PID_H__
#define __PID_H__

#include "at32f403a_407.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        float kp;              // proportional gain
        float ki;              // integral gain
        float kd;              // derivative gain
        float pre_error;       // previous error
        float integral;        // integral term
        float integral_max;    // maximum integral term
        float integral_detach; // de
        float output;          // output value
    } pid_t;

    void pid_init(pid_t* pid, float kp, float ki, float kd, float integral_max, float integral_detach);
    void pid_update(pid_t* pid, float target, float measured);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H__ */