# 12_led_rgb_sequence_my

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/led_blinky (cm33_core0)
- 改造ベース: 10_led_blinky_peripheral

## 改造ポイント
- RGB LED を **1 秒ずつ R → G → B → R ...** と順次点灯(同時に点くのは 1 色のみ)
- pin_mux.c に追加:
  - PORT1 clock enable
  - GREEN (PORT0_27) と BLUE (PORT1_2) を GPIO 出力に
- 全 LED が active LOW(SDK の `LED_xxx_ON/OFF` マクロ使用)
- SysTick 1kHz、1000 tick ごとに `g_state` を 0→1→2→0 で循環

## 動作確認
- ビルド:
- 書き込み:
- 期待挙動: ボード上の **RED → GREEN → BLUE** LED が 1 秒ずつ順番に点灯、同時に
  点くのは 1 色のみ
- 確認日:

## はまり点・解決メモ
- (取り込み時 / ビルド時 / 実機実行時に出た症状と対処)
