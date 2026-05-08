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
 * 11_led_pwm_breath_my : RED LED を 1 秒で明 → 1 秒で暗 を繰り返す呼吸動作。
 *
 * 実装は SysTick 10kHz の software PWM:
 *   - 1 tick = 100us。PWM 周期 = 256 ticks (= 25.6ms, ~39Hz refresh、目視ちらつかない)
 *   - duty 0..255 を、1秒で 0→255、1秒で 255→0 の三角波で更新
 *   - 1 step = 10000 ticks / 255 ≒ 39 ticks (= 約 3.9ms ごとに duty±1)
 *   - LED は active LOW: PortClear が ON、PortSet が OFF
 *   - main は __WFI() で待機、すべてハンドラ内で完結
 */

#define PWM_TICK_HZ      10000U          /* SysTick frequency */
#define PWM_PERIOD       256U            /* PWM resolution (8-bit) */
#define PWM_MAX_DUTY     (PWM_PERIOD - 1U)
#define TICKS_PER_STEP   (PWM_TICK_HZ / PWM_MAX_DUTY)  /* ~39 */

static volatile uint8_t  g_pwm_count    = 0;
static volatile int16_t  g_red_duty     = 0;
static volatile int8_t   g_dir          = 1;     /* +1 brighten, -1 dim */
static volatile uint16_t g_step_tick    = 0;

void SysTick_Handler(void)
{
    /* Per-tick PWM: turn LED on/off based on current duty. */
    if ((int16_t)g_pwm_count < g_red_duty)
    {
        GPIO_PortClear(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN); /* ON  (active LOW) */
    }
    else
    {
        GPIO_PortSet(BOARD_LED_RED_GPIO, 1U << BOARD_LED_RED_GPIO_PIN);   /* OFF */
    }
    g_pwm_count++; /* uint8_t wraps at 256 → PWM_PERIOD */

    /* Duty update: 1 step ≒ 39 ticks (1 sec / 255 steps). */
    g_step_tick++;
    if (g_step_tick >= TICKS_PER_STEP)
    {
        g_step_tick = 0;
        g_red_duty += g_dir;
        if (g_red_duty >= (int16_t)PWM_MAX_DUTY)
        {
            g_red_duty = PWM_MAX_DUTY;
            g_dir      = -1;
        }
        else if (g_red_duty <= 0)
        {
            g_red_duty = 0;
            g_dir      = 1;
        }
    }
}

int main(void)
{
    BOARD_InitHardware();

    /* RED は active LOW、まず OFF (= HIGH) で初期化 */
    LED_RED_INIT(LOGIC_LED_OFF);

    SysTick_Config(SystemCoreClock / PWM_TICK_HZ);

    while (1)
    {
        __WFI();
    }
}
