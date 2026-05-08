/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * Modifications copyright 2026 (FRDM-MCXN947_demo)
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "board.h"
#include "app.h"

/*
 * 01_hello_world_my : 00_hello_world ベースに、オンボード 2 ボタンの
 * 押下を検出して UART (LPUART4 / 115200 8N1) へ区別メッセージを送る。
 *
 *   SW2 (PORT0_23) -> "SW2 pushed."
 *   SW3 (PORT0_6)  -> "SW3 pushed."
 *
 * ボタンは内部プルアップ + 押下で LOW (active LOW)。
 * 立下りエッジを検出して 1 押下につき 1 メッセージ。
 * Debounce は 10ms 周期ポーリングで吸収。
 */

static void delay_ms_busy(uint32_t ms)
{
    /* Core ~150MHz 想定の超ざっくり busy-wait。tick 厳密性は不要な用途。 */
    for (uint32_t i = 0; i < ms; i++)
    {
        for (volatile uint32_t j = 0; j < 15000U; j++)
        {
            __NOP();
        }
    }
}

int main(void)
{
    BOARD_InitHardware();

    PRINTF("MCUX SDK version: %s\r\n", MCUXSDK_VERSION_FULL_STR);
    PRINTF("Press SW2 or SW3 (active LOW, internal pull-up).\r\n");

    /* SW2 / SW3 を GPIO 入力モードに(PORT mux は pin_mux.c で設定済み)。 */
    gpio_pin_config_t sw_in = {kGPIO_DigitalInput, 0};
    GPIO_PinInit(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN, &sw_in);
    GPIO_PinInit(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN, &sw_in);

    /* 解放状態 = HIGH(=1)を初期値に */
    uint32_t prev_sw2 = 1U;
    uint32_t prev_sw3 = 1U;

    while (1)
    {
        uint32_t cur_sw2 = GPIO_PinRead(BOARD_SW2_GPIO, BOARD_SW2_GPIO_PIN);
        uint32_t cur_sw3 = GPIO_PinRead(BOARD_SW3_GPIO, BOARD_SW3_GPIO_PIN);

        /* HIGH -> LOW (押し下げ) を検出して 1 度だけ送信 */
        if (prev_sw2 == 1U && cur_sw2 == 0U)
        {
            PRINTF("SW2 pushed.\r\n");
        }
        if (prev_sw3 == 1U && cur_sw3 == 0U)
        {
            PRINTF("SW3 pushed.\r\n");
        }

        prev_sw2 = cur_sw2;
        prev_sw3 = cur_sw3;

        delay_ms_busy(10U);
    }
}
