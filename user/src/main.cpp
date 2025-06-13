/**
 **************************************************************************
 * @file     main.c
 * @brief    main program
 **************************************************************************
 *                       Copyright notice & Disclaimer
 *
 * The software Board Support Package (BSP) that is made available to
 * download from Artery official website is the copyrighted work of Artery.
 * Artery authorizes customers to use, copy, and distribute the BSP
 * software and its related documentation for the purpose of design and
 * development in conjunction with Artery microcontrollers. Use of the
 * software is governed by this copyright notice and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
 * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
 * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
 * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
 * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
 *
 **************************************************************************
 */

#include "FreeRTOS.h"
#include "MahonyAHRS.h"
#include "adc.h"
#include "at32f403a_407.h"
#include "at32f403a_407_clock.h"
#include "icm42688p.h"
#include "led.h"
#include "motor.h"
#include "mt6701.h"
#include "pid.h"
#include "system.h"
#include "task.h"
#include "tmr.h"
#include "uart.h"
#include "ws2812.h"
#include <math.h>
#include <stdio.h>

#define VELOCITY_RPS_TO_RPM (9.5492965855f)
#define BAT_ADC_SCAL (0.0088623f) // 1 / 4096 * 3.3 * 11
#define STABLE_PITCH_ANGLE1 (-29.6f)
#define STABLE_PITCH_ANGLE2 (-30.5f)
#define STABLE_PITCH_ANGLE3 (89.6f)
#define TORQUE_POLARITY (1.f)   // 力矩与角速度极性
#define VELOCITY_POLARITY (1.f) // 速度极性

Mahony       imu_filter;
pid_t        id_pid, iq_pid, pitch_pid, gyro_z_pid, zero_speed_pid;
TaskHandle_t battay_task_hd, led_strip_hd, uart_info_hd, app_main_hd, imu_hd;

__IO int   close_loop_flag  = 0;
__IO int   open_loop_flag   = 0;
__IO int   low_voltage_flag = 0;
__IO float battary_voltage;
__IO float imu_temp;
__IO float AccX, AccY, AccZ;
__IO float GyroX, GyroY, GyroZ;
__IO float pitch, roll, yaw;
__IO float target_iq = 0, target_id = 0, target_speed = 0, target_pitch = 0;

float _signf(float x);
void  led_cmd(void* param);
void  idq_cmd(void* param);
void  pid_cmd(void* param);
void  speed_cmd(void* param);
void  battary_check(void* param);
void  led_strip_loop(void* param);
void  uart_info_loop(void* param);
void  app_main(void* param);

int main(void)
{
    system_clock_config();
    periph_clock_config();
    systick_config();
    nvic_config();

    uart_debug_init(921600);
    system_clock_print();

    led_init();
    led_strip_init();

    icm42688p_init();
    imu_filter.begin(1000);
    tmr_configuration(TMR2, 240, 1000);
    // delay_ms(3000); // 等待姿态融合收敛稳定，由于motor_init有延时3S，这里不用延时了

    pid_init(&id_pid, 1.62, 0.43, 0, 100, 0);
    pid_init(&iq_pid, 1.62, 0.43, 0, 100, 0);
    // pid_init(&id_pid, 4.8, 1.28, 0, 10, 0);
    // pid_init(&iq_pid, 4.8, 1.28, 0, 10, 0);
    pid_init(&gyro_z_pid, 0.1, 0, 0, 500, 0);
    pid_init(&pitch_pid, 4.8, 0, 0.15, 500, 0);
    pid_init(&zero_speed_pid, 0.068, 0, 0.3, 500, 50);

    motor_init(1, 12000, 12);

    xTaskCreate(battary_check, "battary", 256, NULL, 5, &battay_task_hd);
    xTaskCreate(uart_info_loop, "uart info", 256, NULL, 3, &uart_info_hd);
    xTaskCreate(app_main, "app main", 256, NULL, 10, &app_main_hd);
    vTaskStartScheduler();
}

void app_main(void* param)
{
    // 判断是否闪灯
    static int ticks = 0;
    int        flag  = 0;
    while (ticks++ < 1000)
    {
        vTaskDelay(1);
        if (ticks % 200 == 0)
        {
            led_toggle(LED1);
        }
        if (abs(pitch + 70.f) < 5.f)
        {
            flag++;
            if (flag > 100)
            {
                break;
            }
        }
    }
    led_off(LED1);
    if (flag < 100)
    {
        xTaskCreate(led_strip_loop, "led strip", 256, NULL, 5, &led_strip_hd);
    }
    else
    {
        led_strip_off();
    }
    // xTaskCreate(led_strip_loop, "led strip", 256, NULL, 5, &led_strip_hd);

    for (;;)
    {
        // 进入/离开 闭环判断
        if (abs(STABLE_PITCH_ANGLE1 - pitch) < 1.f && roll < 0)
        {
            // pid 极性正常
            pitch_pid.kp      = abs(pitch_pid.kp);
            pitch_pid.ki      = abs(pitch_pid.ki);
            pitch_pid.kd      = abs(pitch_pid.kd);
            zero_speed_pid.kp = abs(zero_speed_pid.kp);
            zero_speed_pid.ki = abs(zero_speed_pid.ki);
            zero_speed_pid.kd = abs(zero_speed_pid.kd);

            target_pitch    = STABLE_PITCH_ANGLE1;
            close_loop_flag = 1;
            open_loop_flag  = 0;
        }
        else if (abs(STABLE_PITCH_ANGLE2 - pitch) < 1.f && roll > 0)
        {
            // 需要反转pid极性
            pitch_pid.kp      = -abs(pitch_pid.kp);
            pitch_pid.ki      = -abs(pitch_pid.ki);
            pitch_pid.kd      = -abs(pitch_pid.kd);
            zero_speed_pid.kp = -abs(zero_speed_pid.kp);
            zero_speed_pid.ki = -abs(zero_speed_pid.ki);
            zero_speed_pid.kd = -abs(zero_speed_pid.kd);

            target_pitch    = STABLE_PITCH_ANGLE2;
            close_loop_flag = 1;
            open_loop_flag  = 0;
        }
        else if (target_pitch == STABLE_PITCH_ANGLE1 && abs(STABLE_PITCH_ANGLE1 - pitch) > 20.f)
        {
            close_loop_flag = 0;
        }
        else if (target_pitch == STABLE_PITCH_ANGLE2 && abs(STABLE_PITCH_ANGLE2 - pitch) > 20.f)
        {
            close_loop_flag = 0;
        }

        // 进入开环启摆
        if (!close_loop_flag && !open_loop_flag && abs(motor_velocity) < 1.f && abs(GyroZ) < 10.f)
        {
            open_loop_flag = 1;
        }
        if (open_loop_flag)
        {
            target_iq = _signf(GyroZ) * 0.3f * TORQUE_POLARITY;
        }
        if (!open_loop_flag && !close_loop_flag)
        {
            target_iq = -_signf(motor_velocity) * 0.3f;
            if (abs(motor_velocity) < 10.f)
            {
                target_iq = 0;
            }
        }
        vTaskDelay(5);
    }
}

void TMR2_GLOBAL_IRQHandler(void)
{
    if (tmr_flag_get(TMR2, TMR_OVF_FLAG) != RESET)
    {
        tmr_flag_clear(TMR2, TMR_OVF_FLAG);
        // IMU update 约耗时50us
        icm42688p_get_data((float*) &imu_temp, (float*) &AccX, (float*) &AccY, (float*) &AccZ, (float*) &GyroX,
                           (float*) &GyroY, (float*) &GyroZ);
        imu_filter.updateIMU(GyroX, GyroY, GyroZ, AccX, AccY, AccZ);
        pitch = imu_filter.getPitch();
        roll  = imu_filter.getRoll();
        yaw   = imu_filter.getYaw();
    }
}

void TMR4_GLOBAL_IRQHandler(void)
{
    static uint32_t count = 0;
    if (tmr_interrupt_flag_get(TMR4, TMR_C1_INT) != RESET)
    {
        tmr_flag_clear(TMR4, TMR_C1_INT);
        motor_move();
        motor_angle_speed_update(0.0001f);
        motor_current_convert();

        if ((count % 50) == 0)
        {
            if (close_loop_flag)
            {
                led_on(LED1);
                pid_update(&zero_speed_pid, 0.f, VELOCITY_POLARITY * motor_velocity);
                if (abs(zero_speed_pid.output) > 8.f)
                {
                    zero_speed_pid.output = 8.f * _signf(zero_speed_pid.output);
                }
                pid_update(&pitch_pid, target_pitch - zero_speed_pid.output, pitch);
            }
            else
            {
                led_off(LED1);
                zero_speed_pid.output   = 0;
                zero_speed_pid.integral = 0;
                pitch_pid.output        = 0;
                pitch_pid.integral      = 0;
            }
        }

        if ((count % 10) == 0)
        {
            if (close_loop_flag)
            {
                pid_update(&gyro_z_pid, pitch_pid.output, GyroZ);
                target_iq = TORQUE_POLARITY * gyro_z_pid.output;
            }
            else
            {
                gyro_z_pid.integral = 0;
                gyro_z_pid.output   = 0;
            }
        }

        pid_update(&id_pid, target_id, Id);
        pid_update(&iq_pid, target_iq, Iq);
        if (abs(id_pid.output) > 11.f)
            id_pid.output = _signf(id_pid.output) * 11.f;
        if (abs(iq_pid.output) > 11.f)
            iq_pid.output = _signf(iq_pid.output) * 11.f;
        motor_set_voltage(id_pid.output, iq_pid.output);

        count++;
        count %= 1000;
    }
}

void uart_info_loop(void* param)
{
    uart_cmd_register((uint8_t*) "LED", 0, led_cmd);
    uart_cmd_register((uint8_t*) "CUR", 2, idq_cmd);
    uart_cmd_register((uint8_t*) "PID", 2, pid_cmd);
    uart_cmd_register((uint8_t*) "SPEED", 1, speed_cmd);
    for (;;)
    {
        uart_cmd_parse();
        printf("%.4f, %.4f, %.4f, %.4f, %.4f, ", Ia, Ib, Ic, Id, Iq);
        printf("%.4f, ", motor_angle);
        printf("%.4f, ", motor_velocity);
        printf("%.4f, ", battary_voltage);
        printf("%.4f, %.4f, %.4f, ", pitch, roll, yaw);
        printf("%.4f", GyroZ);
        printf("\r\n");
        vTaskDelay(10);
    }
}

void led_strip_loop(void* param)
{
    // RGB LED SHOW
    for (;;)
    {
        led_strip_show();
    }
}

void battary_check(void* param)
{
    for (;;)
    {
        adc_ordinary_software_trigger_enable(ADC1, TRUE);
        while (adc_flag_get(ADC1, ADC_CCE_FLAG) == RESET)
            ;
        adc_flag_clear(ADC1, ADC_CCE_FLAG);
        battary_voltage = adc_ordinary_conversion_data_get(ADC1) * BAT_ADC_SCAL;
        if (battary_voltage < 9.5f && abs(GyroZ) < 10.f)
        {
            low_voltage_flag = 1;
        }
        else
        {
            low_voltage_flag = 0;
        }
        if (low_voltage_flag)
        {
            led_on(LED0);
            vTaskDelay(200);
            led_off(LED0);
            vTaskDelay(200);
            led_on(LED0);
            vTaskDelay(200);
            led_off(LED0);
            vTaskDelay(600);
        }
        else
        {
            vTaskDelay(1000);
        }
    }
}

float _signf(float x)
{
    if (x > 0)
        return 1.f;
    else if (x < 0)
        return -1.f;
    else
        return 0.f;
}

void led_cmd(void* param)
{
    if (param == NULL)
    {
        led_toggle(LED0);
        printf("No param\r\n");
        return;
    }
}

void idq_cmd(void* param)
{
    int32_t* ptr = (int32_t*) param;
    if (ptr == NULL)
    {
        printf("idq NULL param\r\n");
        return;
    }

    target_id = (float) (*(ptr)) / 1000.0f;
    target_iq = (float) (*(ptr + 1)) / 1000.0f;
    // printf("target id:%.4f\r\ntarget iq:%.4f\r\n", target_id, target_iq);
}

void pid_cmd(void* param)
{
    int32_t* ptr = (int32_t*) param;

    if (ptr == NULL)
    {
        printf("No PID param\r\n");
        return;
    }

    id_pid.kp = (float) (*(ptr)) / 1000.0f;
    iq_pid.kp = id_pid.kp;
    id_pid.ki = (float) (*(ptr + 1)) / 1000.0f;
    iq_pid.ki = id_pid.ki;
    printf("kp:%.4f\r\nki:%.4f\r\n", id_pid.kp, id_pid.ki);
}

void speed_cmd(void* param)
{
    int32_t* ptr = (int32_t*) param;

    if (ptr == NULL)
    {
        printf("No PID param\r\n");
        return;
    }
    target_speed = ((float) *(int32_t*) ptr) / VELOCITY_RPS_TO_RPM;
    // printf("target speed:%ld\r\n", *(int32_t*) ptr);
}
