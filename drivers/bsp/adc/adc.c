#include "adc.h"
#include <stdio.h>

__IO uint32_t adccom_ordinary_complete                               = 0;
uint32_t      adccom_ordinary_valuetab[ADC_ORDINARY_CHANNEL_MAX_NUM] = {0};

void adc_configuration(void)
{
    gpio_init_type       gpio_init_struct;
    adc_base_config_type adc_base_struct;

    gpio_default_para_init(&gpio_init_struct);

    /* GPIO of ADC12 config */
    /*  gpio mux for adc
     *   PA0 => ADC1-CH0
     *   PA1 => ADC2-CH1
     *   PA2 => ADC1-CH2
     */
    /*gpio--------------------------------------------------------------------*/
    /* configure the IN0 pin */
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = GPIO_PINS_0;
    gpio_init(GPIOA, &gpio_init_struct);

    /* configure the IN1 pin */
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = GPIO_PINS_1;
    gpio_init(GPIOA, &gpio_init_struct);

    /* configure the IN2 pin */
    gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
    gpio_init_struct.gpio_pins = GPIO_PINS_2;
    gpio_init(GPIOA, &gpio_init_struct);

    /*adc_common_settings-------------------------------------------------------------*/
    crm_adc_clock_div_set(CRM_ADC_DIV_6);
    adc_combine_mode_select(ADC_PREEMPT_SMLT_ONLY_MODE);

    /*adc_settings--------------------------------------------------------------------*/
    adc_base_default_para_init(&adc_base_struct);
    adc_base_struct.sequence_mode           = FALSE;
    adc_base_struct.repeat_mode             = FALSE;
    adc_base_struct.data_align              = ADC_RIGHT_ALIGNMENT;
    adc_base_struct.ordinary_channel_length = ADC_ORDINARY_CHANNEL_MAX_NUM;
    adc_base_config(ADC1, &adc_base_struct);
    adc_base_config(ADC2, &adc_base_struct);

    /* adc_ordinary_conversionmode-------------------------------------------- */
    adc_ordinary_channel_set(ADC1, ADC_CHANNEL_2, 1, ADC_SAMPLETIME_55_5);
    adc_ordinary_conversion_trigger_set(ADC1, ADC12_ORDINARY_TRIG_SOFTWARE, TRUE);

    /* adc_preempt_conversionmode-------------------------------------------- */
    adc_preempt_channel_length_set(ADC1, 1);
    adc_preempt_channel_length_set(ADC2, 1);
    adc_preempt_channel_set(ADC1, ADC_CHANNEL_0, 1, ADC_SAMPLETIME_55_5);
    adc_preempt_channel_set(ADC2, ADC_CHANNEL_1, 1, ADC_SAMPLETIME_55_5);
    adc_preempt_conversion_trigger_set(ADC1, ADC12_PREEMPT_TRIG_TMR4TRGOUT, TRUE);
    adc_preempt_conversion_trigger_set(ADC2, ADC12_PREEMPT_TRIG_TMR4TRGOUT, TRUE);

    /* enable adc------------------------------------------------------------  */
    adc_enable(ADC1, TRUE);
    adc_enable(ADC2, TRUE);

    /* adc calibration-------------------------------------------------------- */
    adc_calibration_init(ADC1);
    adc_calibration_init(ADC2);
    while (adc_calibration_init_status_get(ADC1))
        ;
    adc_calibration_start(ADC1);
    while (adc_calibration_status_get(ADC1))
        ;

    while (adc_calibration_init_status_get(ADC2))
        ;
    adc_calibration_start(ADC2);
    while (adc_calibration_status_get(ADC2))
        ;
}