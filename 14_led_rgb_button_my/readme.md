# 14_led_rgb_button_my

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/led_blinky (cm33_core0)
- 改造ベース: 10_led_blinky_peripheral

## 改造ポイント
- **SW2 または SW3 を押すたびに、RGB LED の点灯色が R → G → B → R ... と切替**
  - 起動直後は RED 点灯
  - SW2 / SW3 どちらでも前進(同等扱い)
- pin_mux.c に追加:
  - PORT1 clock enable
  - GREEN (PORT0_27) / BLUE (PORT1_2) GPIO 出力
  - SW2 (PORT0_23) / SW3 (PORT0_6) GPIO 入力 + 内部プルアップ
- SysTick 1kHz、10ms ごとにボタンサンプリング(= 簡易 debounce)
- 立下りエッジ検出で `g_state` を 0→1→2→0 で循環

## 動作確認
- ビルド:
- 書き込み:
- 期待挙動: 起動時に RED 点灯。SW2 か SW3 を押すと GREEN、もう一回押すと BLUE、
  さらに押すと RED に戻る
- 確認日:

## はまり点・解決メモ
- (取り込み時 / ビルド時 / 実機実行時に出た症状と対処)
