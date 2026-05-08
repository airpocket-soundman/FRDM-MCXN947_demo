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
 * 12_led_rgb_sequence_my : RGB LED を 1 秒ずつ R → G → B → R ... と順次点灯。
 *
 *   PORT0_10 RED, PORT0_27 GREEN, PORT1_2 BLUE (いずれも active LOW)
 *   SysTick = 1kHz、1000 tick (= 1 sec) ごとに状態遷移
 */

#define TICK_HZ 1000U

static volatile uint32_t g_tick  = 0;
static volatile uint8_t  g_state = 0; /* 0:R, 1:G, 2:B */

static void apply_state(uint8_t s)
{
    LED_RED_OFF();
    LED_GREEN_OFF();
    LED_BLUE_OFF();
    switch (s)
    {
        case 0:
            LED_RED_ON();
            break;
        case 1:
            LED_GREEN_ON();
            break;
        case 2:
            LED_BLUE_ON();
            break;
        default:
            break;
    }
}

void SysTick_Handler(void)
{
    g_tick++;
    if (g_tick >= TICK_HZ)
    {
        g_tick  = 0;
        g_state = (uint8_t)((g_state + 1U) % 3U);
        apply_state(g_state);
    }
}

int main(void)
{
    BOARD_InitHardware();

    LED_RED_INIT(LOGIC_LED_OFF);
    LED_GREEN_INIT(LOGIC_LED_OFF);
    LED_BLUE_INIT(LOGIC_LED_OFF);

    apply_state(g_state); /* 初期は RED ON */

    SysTick_Config(SystemCoreClock / TICK_HZ);

    while (1)
    {
        __WFI();
    }
}
