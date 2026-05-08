/*
 * Copyright 2013 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * Modifications copyright 2026 (FRDM-MCXN947_demo)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * 31_onboard_touch_my : touch_sensing demo ベース。FreeMASTER 関連を切って
 * オンボード TSI 電極(El_1 = ch3)の値を SDK debug console (LPUART4 / 115200 8N1)
 * に PRINTF で流し続ける + delta を RGB LED の白色明るさにマップする版。
 *
 * シリアル出力(値の行):
 *   raw=<u32> base=<u32> delta=<+/-><u32> nt=<u32> touch=<u32>
 * Output rate: nt_task() 100 ループに 1 回(UART 帯域節約)
 *
 * LED マッピング:
 *   delta(正側のみ、0..10000 にクランプ) -> duty 0..255 に linear map
 *   3 LED 同 duty で SW PWM 駆動 -> 白色で強度を視覚化
 *   PWM: SysTick 10kHz、8-bit duty、~39Hz refresh
 *   LED duty 更新は毎ループ(滑らかな反応のため、PRINTF 間引きとは独立)
 */

#include <stdio.h>
#include <stdlib.h>
#include "fsl_device_registers.h"
#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"

#include "clock_config.h"
#include "pin_mux.h"
#include "board.h"
#include "main.h"
#include "nt.h"
#include "nt_setup.h"

static void CTIMERInit(void);

#if defined(__ICCARM__)
uint8_t nt_memory_pool[3700];
#else
uint8_t nt_memory_pool[3700] __attribute__((aligned(4)));
#endif

/*
 * SW PWM 用 duty (g_duty): main loop が delta から計算して書き込み、
 * SysTick_Handler が読んで 3 LED を on/off する。
 * 1 byte の volatile への単純代入なので tearing なし。
 */
static volatile uint8_t g_duty = 0U;

void SysTick_Handler(void)
{
    static uint8_t pwm_count = 0U;
    pwm_count++; /* uint8_t は 256 で wrap = PWM 周期 */

    if (pwm_count < g_duty)
    {
        /* ON (active LOW: PortClear で点灯) */
        GPIO_PortClear(BOARD_LED_RED_GPIO,   1U << BOARD_LED_RED_GPIO_PIN);
        GPIO_PortClear(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN);
        GPIO_PortClear(BOARD_LED_BLUE_GPIO,  1U << BOARD_LED_BLUE_GPIO_PIN);
    }
    else
    {
        /* OFF */
        GPIO_PortSet(BOARD_LED_RED_GPIO,   1U << BOARD_LED_RED_GPIO_PIN);
        GPIO_PortSet(BOARD_LED_GREEN_GPIO, 1U << BOARD_LED_GREEN_GPIO_PIN);
        GPIO_PortSet(BOARD_LED_BLUE_GPIO,  1U << BOARD_LED_BLUE_GPIO_PIN);
    }
}

int main(void)
{
    int32_t result;

    /* Init board hardware (debug console = LPUART4 @ 115200 8N1) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    LED_RED_INIT(LOGIC_LED_OFF);
    LED_GREEN_INIT(LOGIC_LED_OFF);
    LED_BLUE_INIT(LOGIC_LED_OFF);

    NT_OSA_Init();

    PRINTF("\r\n31_onboard_touch_my\r\n");
    PRINTF("Streaming TSI ch3 (El_1) raw/base/delta + mapping to white-LED SW PWM.\r\n");

    if ((result = nt_init(&System_0, nt_memory_pool, sizeof(nt_memory_pool))) != NT_SUCCESS)
    {
        LED_RED_ON();
        PRINTF("nt_init failed: %d\r\n", (int)result);
        while (1) { }
    }

    nt_enable();

    /* オンボード単独使用: FRDM-TOUCH 別売ボード分の電極を無効化 */
    (void)nt_electrode_disable(&El_2);
    (void)nt_electrode_disable(&El_3);
    (void)nt_electrode_disable(&El_4);
    (void)nt_electrode_disable(&El_5);
    (void)nt_electrode_disable(&El_6);
    (void)nt_electrode_disable(&El_7);
    (void)nt_electrode_disable(&El_8);
    (void)nt_electrode_disable(&El_9);
    (void)nt_electrode_disable(&El_10);
    (void)nt_electrode_disable(&El_11);

    CTIMERInit();

    /* SW PWM start: SysTick 10kHz (100us tick, PWM 周期 25.6ms ≒ 39Hz refresh) */
    SysTick_Config(SystemCoreClock / 10000U);

    /*
     * Baseline 学習プロトコル: 起動時 N サンプル平均で my_base を確定し、以後固定。
     * 自前 touch 判定: delta >= MY_TOUCH_ON で ON、< MY_TOUCH_OFF で OFF (ヒステリシス)。
     * LED duty: delta を 0..DELTA_FULL_SCALE にクランプして 0..255 に linear map。
     */
    #define BASELINE_LEARN_SAMPLES 256U
    #define MY_TOUCH_ON            3000U
    #define MY_TOUCH_OFF           1500U
    #define DELTA_FULL_SCALE       10000U

    uint64_t base_sum    = 0;
    uint32_t base_count  = 0;
    uint32_t my_base     = 0;
    bool     base_locked = false;
    bool     my_touch    = false;
    uint32_t print_div   = 0;

    while (1)
    {
        nt_task();

        uint32_t raw    = nt_electrode_get_raw_signal(&El_1);
        int32_t  status = nt_electrode_get_last_status(&El_1);

        /* baseline 学習(起動直後の INIT 状態を抜けたら毎ループ raw を蓄積) */
        if (!base_locked && status != NT_ELECTRODE_STATE_INIT)
        {
            base_sum += raw;
            base_count++;
            if (base_count >= BASELINE_LEARN_SAMPLES)
            {
                my_base     = (uint32_t)(base_sum / base_count);
                base_locked = true;
                PRINTF("# baseline locked: base=%u (n=%u)\r\n",
                       (unsigned int)my_base, (unsigned int)base_count);
            }
        }

        /* 毎ループ: delta 計算 + LED duty 更新 + 自前 touch hysteresis */
        unsigned int delta_abs  = 0U;
        char         delta_sign = '?';

        if (base_locked)
        {
            if (raw >= my_base)
            {
                delta_abs  = (unsigned int)(raw - my_base);
                delta_sign = '+';
            }
            else
            {
                delta_abs  = (unsigned int)(my_base - raw);
                delta_sign = '-';
            }

            /* delta(正側のみ採用、0..FULL_SCALE クランプ) -> duty 0..255 linear */
            uint32_t mag = (delta_sign == '+') ? delta_abs : 0U;
            if (mag > DELTA_FULL_SCALE)
            {
                mag = DELTA_FULL_SCALE;
            }
            g_duty = (uint8_t)((mag * 255U) / DELTA_FULL_SCALE);

            /* 自前 touch hysteresis(立ち上がり 3000、立ち下がり 1500) */
            if (!my_touch && delta_sign == '+' && delta_abs >= MY_TOUCH_ON)
            {
                my_touch = true;
            }
            else if (my_touch && (delta_sign != '+' || delta_abs < MY_TOUCH_OFF))
            {
                my_touch = false;
            }
        }
        else
        {
            g_duty = 0U; /* 学習中は LED 消灯 */
        }

        /* PRINTF は 100 ループに 1 回 (UART 帯域節約) */
        if (++print_div < 100U)
        {
            continue;
        }
        print_div = 0;

        if (!base_locked)
        {
            PRINTF("# learning... raw=%u n=%u/%u (do not touch)\r\n",
                   (unsigned int)raw, (unsigned int)base_count,
                   (unsigned int)BASELINE_LEARN_SAMPLES);
            continue;
        }

        PRINTF("raw=%u base=%u delta=%c%u nt=%u touch=%u\r\n",
               (unsigned int)raw, (unsigned int)my_base,
               delta_sign, delta_abs,
               (unsigned int)status, (unsigned int)my_touch);
    }
}

void CTIMER0_IRQHandler(void)
{
    nt_trigger();
    CTIMER0->IR |= CTIMER_IR_MR0INT(1U);
    __DSB();
    __ISB();
}

void TSI_END_OF_SCAN_DriverIRQHandler(void)
{
    TSI_DRV_IRQHandler(0);
}

void TSI_OUT_OF_SCAN_DriverIRQHandler(void)
{
    TSI_DRV_IRQHandler(0);
}

static void CTIMERInit(void)
{
    /* CTimer0 を FRO HF で駆動(touch demo と同じ設定)*/
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

    SYSCON->AHBCLKCTRLSET[1] |= SYSCON_AHBCLKCTRL1_TIMER0_MASK;
    SYSCON->PRESETCTRLSET[1] = SYSCON_PRESETCTRL1_TIMER0_RST_MASK;
    while (0u == (SYSCON->PRESETCTRL1 & SYSCON_PRESETCTRL1_TIMER0_RST_MASK))
    {
    }
    SYSCON->PRESETCTRLCLR[1] = SYSCON_PRESETCTRL1_TIMER0_RST_MASK;
    while (SYSCON_PRESETCTRL1_TIMER0_RST_MASK ==
           (SYSCON->PRESETCTRL1 & SYSCON_PRESETCTRL1_TIMER0_RST_MASK))
    {
    }

    CTIMER0->MCR |= CTIMER_MCR_MR0R(1U) | CTIMER_MCR_MR0I(1U);
    CTIMER0->MR[0] = (nt_kernel_data.rom->time_period * CLOCK_GetFreq(kCLOCK_FroHf)) / 1000;
    CTIMER0->IR    = CTIMER_IR_MR0INT_MASK;

    NVIC_SetPriority(CTIMER0_IRQn, 1U);
    NVIC_EnableIRQ(CTIMER0_IRQn);
    CTIMER0->TCR |= CTIMER_TCR_CEN_MASK;
}
