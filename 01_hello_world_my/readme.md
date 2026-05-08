# 01_hello_world_my

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/hello_world (cm33_core0)
- 改造ベース: 00_hello_world

## 改造ポイント
- オンボード 2 ボタンの押下を検出して UART に区別メッセージ送信
  - **SW2** (PORT0_23) → `SW2 pushed.\r\n`
  - **SW3** (PORT0_6) → `SW3 pushed.\r\n`
- ボタンは内部プルアップ + active LOW(押下で 0 を読む)。`pin_mux.c` の
  `BOARD_InitPins()` 末尾で `PORT_SetPinConfig` を 2 ピン分追加(mux=Alt0=GPIO,
  pull-up, input buffer enable)
- main は `GPIO_PinRead` で 10ms 周期ポーリング、立下りエッジで `PRINTF`
- Debounce は 10ms ポーリング自体で吸収(機械接点のチャタリングは数 ms)

## 動作確認
- ビルド:
- 書き込み:
- シリアル出力 (115200/8N1):
  ```
  MCUX SDK version: ...
  Press SW2 or SW3 (active LOW, internal pull-up).
  SW2 pushed.
  SW3 pushed.
  ```
- 確認日:

## はまり点・解決メモ
- (取り込み時 / ビルド時 / 実機実行時に出た症状と対処)
