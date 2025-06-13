#include "tmr.h"
#include "ws2812.h"
#include <stdio.h>

void tmr_configuration(tmr_type* TMRx, uint16_t div, uint16_t period)
{
    gpio_init_type         gpio_init_struct;
    tmr_output_config_type tmr_output_struct;
    dma_init_type          dma_init_struct;

    if (TMRx == TMR4)
    {
        /*  gpio mux for pwm
         *   PA6 => TMR4_CH1 // 未输出
         *   PB7 => TMR4_CH2 => PWMA
         *   PB8 => TMR4_CH3 => PWMB
         *   PB9 => TMR4_CH4 => PWMC
         */
        gpio_default_para_init(&gpio_init_struct);
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_7 | GPIO_PINS_8 | GPIO_PINS_9;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOB, &gpio_init_struct);

        /* configure counter settings */
        tmr_cnt_dir_set(TMRx, TMR_COUNT_TWO_WAY_2);
        tmr_clock_source_div_set(TMRx, TMR_CLOCK_DIV1);
        tmr_period_buffer_enable(TMRx, FALSE);
        tmr_base_init(TMRx, period, div - 1);

        /* configure primary mode settings */
        tmr_primary_mode_select(TMRx, TMR_PRIMARY_SEL_COMPARE); // 仅通道1比较事件传递至触发输出

        /* configure channel 1 output settings */
        tmr_output_struct.oc_mode          = TMR_OUTPUT_CONTROL_PWM_MODE_B;
        tmr_output_struct.oc_output_state  = TRUE;
        tmr_output_struct.occ_output_state = FALSE;
        tmr_output_struct.oc_polarity      = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.occ_polarity     = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.oc_idle_state    = FALSE;
        tmr_output_struct.occ_idle_state   = FALSE;
        tmr_output_channel_config(TMRx, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
        tmr_channel_value_set(TMRx, TMR_SELECT_CHANNEL_1, 0);

        /* configure channel 2 output settings */
        tmr_output_struct.oc_mode          = TMR_OUTPUT_CONTROL_PWM_MODE_B;
        tmr_output_struct.oc_output_state  = TRUE;
        tmr_output_struct.occ_output_state = FALSE;
        tmr_output_struct.oc_polarity      = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.occ_polarity     = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.oc_idle_state    = FALSE;
        tmr_output_struct.occ_idle_state   = FALSE;
        tmr_output_channel_config(TMRx, TMR_SELECT_CHANNEL_2, &tmr_output_struct);
        tmr_channel_value_set(TMRx, TMR_SELECT_CHANNEL_2, period);

        /* configure channel 3 output settings */
        tmr_output_struct.oc_mode          = TMR_OUTPUT_CONTROL_PWM_MODE_B;
        tmr_output_struct.oc_output_state  = TRUE;
        tmr_output_struct.occ_output_state = FALSE;
        tmr_output_struct.oc_polarity      = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.occ_polarity     = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.oc_idle_state    = FALSE;
        tmr_output_struct.occ_idle_state   = FALSE;
        tmr_output_channel_config(TMRx, TMR_SELECT_CHANNEL_3, &tmr_output_struct);
        tmr_channel_value_set(TMRx, TMR_SELECT_CHANNEL_3, period);

        /* configure channel 4 output settings */
        tmr_output_struct.oc_mode          = TMR_OUTPUT_CONTROL_PWM_MODE_B;
        tmr_output_struct.oc_output_state  = TRUE;
        tmr_output_struct.occ_output_state = FALSE;
        tmr_output_struct.oc_polarity      = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.occ_polarity     = TMR_OUTPUT_ACTIVE_HIGH;
        tmr_output_struct.oc_idle_state    = FALSE;
        tmr_output_struct.occ_idle_state   = FALSE;
        tmr_output_channel_config(TMRx, TMR_SELECT_CHANNEL_4, &tmr_output_struct);
        tmr_channel_value_set(TMRx, TMR_SELECT_CHANNEL_4, period);

        /* compare ch1 interrupt enable */
        tmr_interrupt_enable(TMRx, TMR_C1_INT, FALSE);

        /* enable tmr4 */
        tmr_counter_enable(TMRx, TRUE);
    }
    else if (TMRx == TMR3)
    {
        /* configure counter settings */
        tmr_cnt_dir_set(TMRx, TMR_COUNT_UP);
        tmr_clock_source_div_set(TMRx, TMR_CLOCK_DIV1);
        tmr_period_buffer_enable(TMRx, FALSE);
        tmr_base_init(TMRx, period, div - 1);

        /* overflow interrupt enable */
        tmr_flag_clear(TMRx, TMR_OVF_INT);
        tmr_interrupt_enable(TMRx, TMR_OVF_INT, TRUE);

        /* enable tmr3 */
        tmr_counter_enable(TMRx, TRUE);
    }
    else if (TMRx == TMR2)
    {
        /* configure counter settings */
        tmr_cnt_dir_set(TMRx, TMR_COUNT_UP);
        tmr_clock_source_div_set(TMRx, TMR_CLOCK_DIV1);
        tmr_base_init(TMRx, period, div - 1);

        /* overflow interrupt enable */
        tmr_flag_clear(TMRx, TMR_OVF_INT);
        tmr_interrupt_enable(TMRx, TMR_OVF_INT, TRUE);

        /* enable tmr2 */
        tmr_counter_enable(TMRx, TRUE);
    }
    else if (TMRx == TMR1)
    {
        /*  gpio mux for pwm
         *   PA8 => TMR1_CH1
         */
        gpio_default_para_init(&gpio_init_struct);
        gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
        gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
        gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
        gpio_init_struct.gpio_pins           = GPIO_PINS_8;
        gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
        gpio_init(GPIOA, &gpio_init_struct);

        gpio_pin_remap_config(TMR1_MUX_01, TRUE);

        /* TMR1 CH1 DMA config ---------------------------------------------- */
        /* flexible function enable */
        dma_flexible_config(DMA1, FLEX_CHANNEL3, DMA_FLEXIBLE_TMR1_OVERFLOW);
        dma_default_para_init(&dma_init_struct);
        dma_reset(DMA1_CHANNEL3);
        dma_init_struct.direction             = DMA_DIR_MEMORY_TO_PERIPHERAL;
        dma_init_struct.memory_data_width     = DMA_MEMORY_DATA_WIDTH_HALFWORD;
        dma_init_struct.memory_inc_enable     = TRUE;
        dma_init_struct.peripheral_base_addr  = (uint32_t) &(TMR1->c1dt);
        dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
        dma_init_struct.peripheral_inc_enable = FALSE;
        dma_init_struct.priority              = DMA_PRIORITY_MEDIUM;
        dma_init_struct.loop_mode_enable      = FALSE;
        dma_init(DMA1_CHANNEL3, &dma_init_struct);

        /* enable dma transfer complete interrupt */
        dma_flag_clear(DMA1_FDT3_FLAG);
        dma_channel_enable(DMA1_CHANNEL3, FALSE);

        /* configure counter settings ----------------------------------------- */
        tmr_cnt_dir_set(TMRx, TMR_COUNT_UP);
        tmr_clock_source_div_set(TMRx, TMR_CLOCK_DIV1);
        tmr_base_init(TMRx, period, div - 1);

        /* configure channel 1 output settings */
        tmr_output_struct.oc_mode          = TMR_OUTPUT_CONTROL_PWM_MODE_B;
        tmr_output_struct.oc_output_state  = TRUE;
        tmr_output_struct.occ_output_state = FALSE;
        tmr_output_struct.oc_polarity      = TMR_OUTPUT_ACTIVE_LOW;
        tmr_output_struct.occ_polarity     = TMR_OUTPUT_ACTIVE_LOW;
        tmr_output_struct.oc_idle_state    = FALSE;
        tmr_output_struct.occ_idle_state   = FALSE;
        tmr_output_channel_config(TMRx, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
        tmr_channel_value_set(TMRx, TMR_SELECT_CHANNEL_1, 0);

        tmr_dma_request_enable(TMRx, TMR_OVERFLOW_DMA_REQUEST, TRUE);

        /* output enable */
        tmr_output_enable(TMRx, TRUE);

        tmr_counter_enable(TMRx, FALSE);
    }
}
