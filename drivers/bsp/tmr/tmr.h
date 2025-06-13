#ifndef __BSP_TMR_H_
#define __BSP_TMR_H_

#include "at32f403a_407.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void tmr_configuration(tmr_type* TMRx, uint16_t div, uint16_t period);
    void TMR4_GLOBAL_IRQHandler(void);
    void TMR3_GLOBAL_IRQHandler(void);
    void TMR2_GLOBAL_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TMR_H_ */