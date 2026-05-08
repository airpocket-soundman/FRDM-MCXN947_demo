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
 * 14_led_rgb_button_my : SW2 または SW3 を押すたびに点灯色が R → G → B → R ...
 * と切り替わる。
 *
 *   - SysTick 1kHz、10ms 周期 (= 100Hz サンプリング) でボタン状態を見る
 *     → サンプリング間隔自体が簡易 debounce
 *   - SW2 / SW3 どちらでも前進(同等扱い)
 *   - 起動直後は RED 点灯、いずれも active LOW で 1 色排他
 */

#define TICK_HZ    1000U
#define SAMPLE_DIV 10U /* 1kHz / 10 = 100Hz button sampling */

static volatile uint32_t g_tick     = 0;
static volatile uint32_t g_prev_sw2 = 1U;
static volatile uint32_t g_prev_sw3 = 1U;
static volatile uint8_t  g_state    = 0; /* 0:R, 1:G, 2:B */

static void apply_state(uint8_t s)
{
    LED_RED_OFF();
    LED_GREEN_OFF();
    LED_BLUE_OFF();
    switch (s % 3U)
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
    if ((g_tick % SAMPLE_DIV) != 0U)
    {
        return; /* sample only every 10ms */
    }

    uint32_t cur_sw2 = GPIO_PinRead(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN);
    uint32_t cur_sw3 = GPIO_PinRead(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN);

    /* HIGH -> LOW (押下) を SW2/SW3 のどちらかで検出すれば前進 */
    if ((g_prev_sw2 == 1U && cur_sw2 == 0U) ||
        (g_prev_sw3 == 1U && cur_sw3 == 0U))
    {
        g_state = (uint8_t)((g_state + 1U) % 3U);
        apply_state(g_state);
    }

    g_prev_sw2 = cur_sw2;
    g_prev_sw3 = cur_sw3;
}

int main(void)
{
    BOARD_InitHardware();

    LED_RED_INIT(LOGIC_LED_OFF);
    LED_GREEN_INIT(LOGIC_LED_OFF);
    LED_BLUE_INIT(LOGIC_LED_OFF);

    gpio_pin_config_t sw_in = {kGPIO_DigitalInput, 0};
    GPIO_PinInit(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, &sw_in);
    GPIO_PinInit(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN, &sw_in);

    apply_state(g_state); /* 起動直後は RED */

    SysTick_Config(SystemCoreClock / TICK_HZ);
    while (1)
    {
        __WFI();
    }
}
