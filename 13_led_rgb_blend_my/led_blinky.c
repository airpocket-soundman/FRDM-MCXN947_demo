/*
 * Copyright 2019 NXP
 * Modifications copyright 2026 (FRDM-MCXN947_demo)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_gpio.h"
#include "board.h"
#include "app.h"

/*
 * 13_led_rgb_blend_my : RGB LED の cross-fade。
 *   Phase 0 (1 sec) : R fade-out + G fade-in
 *   Phase 1 (1 sec) : G fade-out + B fade-in
 *   Phase 2 (1 sec) : B fade-out + R fade-in
 *   ... 繰り返し(計 3 秒で 1 周期)
 *
 * Software PWM 3ch:
 *   - SysTick 10kHz、PWM 周期 = 256 tick (~25.6ms, ~39Hz refresh)
 *   - PHASE_TICKS = 10000 = 1 sec
 *   - 各 tick で fade 進行に応じて r/g/b の duty を更新
 *   - LED は active LOW (PortClear = ON)
 */

#define PWM_TICK_HZ  10000U
#define PWM_PERIOD   256U
#define PWM_MAX_DUTY (PWM_PERIOD - 1U)
#define PHASE_TICKS  10000U /* 1 sec per phase */

static volatile uint8_t  g_pwm_count = 0;
static volatile uint8_t  g_r_duty    = PWM_MAX_DUTY; /* 起動直後 = R 全開 */
static volatile uint8_t  g_g_duty    = 0;
static volatile uint8_t  g_b_duty    = 0;

static volatile uint16_t g_phase_tick = 0;
static volatile uint8_t  g_phase      = 0; /* 0:R->G, 1:G->B, 2:B->R */

static inline void apply_pwm(uint8_t cnt)
{
    if (cnt < g_r_duty)
    {
        GPIO_PortClear(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN);
    }
    else
    {
        GPIO_PortSet(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN);
    }
    if (cnt < g_g_duty)
    {
        GPIO_PortClear(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN);
    }
    else
    {
        GPIO_PortSet(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN);
    }
    if (cnt < g_b_duty)
    {
        GPIO_PortClear(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN);
    }
    else
    {
        GPIO_PortSet(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN);
    }
}

void SysTick_Handler(void)
{
    /* 1) per-tick PWM */
    apply_pwm(g_pwm_count);
    g_pwm_count++;

    /* 2) duty を fade 進行に従って更新
     *    progress = g_phase_tick / PHASE_TICKS in [0, 1)
     *    out_d = (1 - progress) * MAX, in_d = progress * MAX */
    uint16_t out_d = (uint16_t)(PWM_MAX_DUTY - (g_phase_tick * PWM_MAX_DUTY) / PHASE_TICKS);
    uint16_t in_d  = (uint16_t)((g_phase_tick * PWM_MAX_DUTY) / PHASE_TICKS);

    switch (g_phase)
    {
        case 0: /* R out, G in, B off */
            g_r_duty = (uint8_t)out_d;
            g_g_duty = (uint8_t)in_d;
            g_b_duty = 0;
            break;
        case 1: /* G out, B in, R off */
            g_g_duty = (uint8_t)out_d;
            g_b_duty = (uint8_t)in_d;
            g_r_duty = 0;
            break;
        case 2: /* B out, R in, G off */
            g_b_duty = (uint8_t)out_d;
            g_r_duty = (uint8_t)in_d;
            g_g_duty = 0;
            break;
        default:
            break;
    }

    /* 3) phase 切替 */
    g_phase_tick++;
    if (g_phase_tick >= PHASE_TICKS)
    {
        g_phase_tick = 0;
        g_phase      = (uint8_t)((g_phase + 1U) % 3U);
    }
}

int main(void)
{
    BOARD_InitHardware();
    LED_RED_INIT(LOGIC_LED_OFF);
    LED_GREEN_INIT(LOGIC_LED_OFF);
    LED_BLUE_INIT(LOGIC_LED_OFF);

    SysTick_Config(SystemCoreClock / PWM_TICK_HZ);
    while (1)
    {
        __WFI();
    }
}
