# 11_led_pwm_breath_my

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/led_blinky (cm33_core0)
- 改造ベース: 10_led_blinky_peripheral

## 改造ポイント
- RED LED (PORT0_10, active LOW) を **1秒で明 → 1秒で暗** の三角波で呼吸
- **Software PWM** (SysTick 10kHz, 8-bit duty)
  - PWM 周期 = 256 tick (25.6ms, ~39Hz refresh、目視ちらつかない)
  - duty 0..255 を 1 秒で fade (1 step ≒ 39 tick = 3.9ms)
- pin_mux.c は変更なし(SDK 既存の RED 出力設定をそのまま使用)
- main は `__WFI()` で待機、ロジックは `SysTick_Handler` 内に集約

## 動作確認
- ビルド:
- 書き込み:
- 期待挙動: ボード上の **RED LED** だけが、1 秒かけて 0% → 100%、もう 1 秒かけて
  100% → 0% を繰り返す(2 秒で 1 周期)
- 確認日:

## はまり点・解決メモ
- (取り込み時 / ビルド時 / 実機実行時に出た症状と対処)
