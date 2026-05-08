# 13_led_rgb_blend_my

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/led_blinky (cm33_core0)
- 改造ベース: 10_led_blinky_peripheral

## 改造ポイント
- RGB LED の **cross-fade**(隣り合う 2 色を同時に弱→強で入れ替え)
  - Phase 0 (1 秒): **R が弱まりつつ G が強くなる**
  - Phase 1 (1 秒): **G が弱まりつつ B が強くなる**
  - Phase 2 (1 秒): **B が弱まりつつ R が強くなる**
  - 計 3 秒で 1 サイクル、無限ループ
- pin_mux.c に追加(12 と同じ): PORT1 clock + GREEN/BLUE GPIO 出力
- **Software PWM 3ch**: SysTick 10kHz、8-bit duty、PHASE_TICKS = 10000 (1 秒)
- main は `__WFI()` で待機

## 動作確認
- ビルド:
- 書き込み:
- 期待挙動: ボード上の RGB LED が R↔G↔B↔R を 1 秒ずつ滑らかにクロスフェード
- 確認日:

## はまり点・解決メモ
- (取り込み時 / ビルド時 / 実機実行時に出た症状と対処)
