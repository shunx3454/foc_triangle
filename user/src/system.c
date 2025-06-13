#include "system.h"
#include "tmr.h"
#include <stdio.h>

/* global variable */
volatile uint32_t timebase_ticks;

/**
 * @brief  nvic config
 * @param  none
 * @retval none
 */
void nvic_config(void)
{
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

    NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    nvic_irq_enable(TMR4_GLOBAL_IRQn, 1, 0);
    nvic_irq_enable(TMR2_GLOBAL_IRQn, 1, 0);
    nvic_irq_enable(TMR3_GLOBAL_IRQn, 4, 0);
    nvic_irq_enable(DMA1_Channel2_IRQn, 4, 0);
}

/**
 * @brief  this function configures the source of the time base
 *         the time source is configured to have 1ms time base
 * @param  none
 * @retval none
 */
void systick_config(void)
{
    // crm_clocks_freq_type crm_clocks;
    // uint32_t             frequency = 0;
    //
    ///* get crm_clocks */
    // crm_clocks_freq_get(&crm_clocks);
    //
    // frequency = crm_clocks.ahb_freq / 8;
    //
    ///* config systick clock source */
    // systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_DIV8);
    ///* system tick config */
    // SysTick->LOAD = (uint32_t) ((frequency / 1000) - 1UL);
    // SysTick->VAL  = 0UL;
    // SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    tmr_configuration(TMR3, 240, 1000);
}

/**
 * @brief  print system clock frequence
 * @param  none
 * @retval none
 */
void system_clock_print(void)
{
    crm_clocks_freq_type crm_clock_freq;
    crm_clocks_freq_get(&crm_clock_freq);
    printf("\r\n###### clock frequence ######\r\n");
    printf("system clock frequence: %ldMHz\r\n", crm_clock_freq.sclk_freq / 1000000U);
    printf("ahb    clock frequence: %ldMHz\r\n", crm_clock_freq.ahb_freq / 1000000U);
    printf("apb1   clock frequence: %ldMHz\r\n", crm_clock_freq.apb1_freq / 1000000U);
    printf("apb2   clock frequence: %ldMHz\r\n", crm_clock_freq.apb2_freq / 1000000U);
    printf("\r\n");
}

/**
 * @brief  this function provides minimum delay (in milliseconds) based
 *         on variable incremented.
 * @param  delay variable specifies the delay time length, in milliseconds.
 * @retval none
 */
void delay_ms(uint32_t delay)
{
    uint32_t start_tick = timebase_get();

    if (delay < 0xFFFFFFFFU)
    {
        delay += 1;
    }

    while ((timebase_get() - start_tick) < delay)
    {
    }
}

/**
 * @brief  this function is called to increment a global variable "timebase_ticks"
 *         used as application time base.
 * @param  none
 * @retval none
 */
void timebase_increase(void)
{
    timebase_ticks++;
}

/**
 * @brief  this function is called at timebase handler, eg: SysTick_Handler
 * @param  none
 * @retval none
 */
void systick_handler(void)
{
    timebase_increase();
}

/**
 * @brief  provides a tick value in millisecond.
 * @param  none
 * @retval tick value
 */
uint32_t timebase_get(void)
{
    return timebase_ticks;
}

/* support printf function, usemicrolib is unnecessary */
#if (__ARMCC_VERSION > 6000000)
__asm(".global __use_no_semihosting\n\t");
void _sys_exit(int x)
{
    x = x;
}
/* __use_no_semihosting was requested, but _ttywrch was */
void _ttywrch(int ch)
{
    ch = ch;
}
FILE __stdout;
#else
#ifdef __CC_ARM
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};
FILE __stdout;
void _sys_exit(int x)
{
    x = x;
}
/* __use_no_semihosting was requested, but _ttywrch was */
void _ttywrch(int ch)
{
    ch = ch;
}
#endif
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE* f)
#endif

/**
 * @brief  retargets the c library printf function to the usart.
 * @param  none
 * @retval none
 */
PUTCHAR_PROTOTYPE
{
    while (usart_flag_get(USART1, USART_TDBE_FLAG) == RESET)
        ;
    usart_data_transmit(USART1, (uint16_t) ch);
    while (usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
        ;
    return ch;
}

#if (defined(__GNUC__) && !defined(__clang__)) || (defined(__ICCARM__))
#if defined(__GNUC__) && !defined(__clang__)
int _write(int fd, char* pbuffer, int size)
#elif defined(__ICCARM__)
#pragma module_name = "?__write"
int __write(int fd, char* pbuffer, int size)
#endif
{
    for (int i = 0; i < size; i++)
    {
        while (usart_flag_get(USART1, USART_TDBE_FLAG) == RESET)
            ;
        usart_data_transmit(USART1, (uint16_t) (*pbuffer++));
        while (usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
            ;
    }

    return size;
}
#endif

void TMR3_GLOBAL_IRQHandler(void)
{
    if (tmr_interrupt_flag_get(TMR3, TMR_OVF_FLAG) != RESET)
    {
        systick_handler();
        tmr_flag_clear(TMR3, TMR_OVF_FLAG);
    }
}