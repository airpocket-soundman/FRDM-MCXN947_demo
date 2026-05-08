# 31_onboard_touch_my

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/touch_sensing (cm33_core0)
- 改造ベース: 30_onboard_touch

## 改造ポイント
- **FreeMASTER を完全に切り、SDK 標準 debug console (LPUART4 / 115200 8N1) に
  オンボード TSI 電極の値を平文 PRINTF で流し続ける版**
- 30 (touch_sensing demo) は FreeMASTER 経由 (19200 bps バイナリプロトコル)で
  通常のシリアル端末では値が読めなかったので、それを置き換えた構成
- オンボードのタッチ電極は `El_1 = BOARD_TSI_ELECTRODE_1 = TSI ch3` の 1 個のみ。
  別売 FRDM-TOUCH ボード前提の El_2..El_11 は `nt_electrode_disable()` で無効化
- 出力フォーマット:
  ```
  signal=<u32> raw=<u32> status=<i32>
  ```
  - **signal**: NT 内部で baseline 補正済みの相対値(タッチ強度の指標)
  - **raw**: TSI ペリフェラル生カウンタ
  - **status**: `nt_electrode_get_last_status()` の戻り値(touch/release 状態 enum 値、
    そのまま整数で出力)
- nt_task() 100 周期に 1 回の頻度で出力(出力過多を抑制)
- LED フィードバックは省略(値の安定観測に集中)

## 主な変更
- `cm33_core0/main.c` 全書き換え:
  - `freemaster.h` / `freemaster_serial_lpuart.h` の include 削除
  - `init_freemaster_lpuart()`、`FMSTR_*` 全削除
  - `BOARD_InitDebugConsole()` 呼び出しを追加(LPUART4 を 115200 で初期化、PRINTF 経路)
  - main loop で `nt_electrode_get_signal/raw_signal/last_status` を周期 PRINTF
- `prj.conf` / `CMakeLists.txt` / `freemaster_cfg.h` は 30 と同じ。FreeMASTER ライブラリは
  SDK Kconfig 依存(touch middleware 側が pull する)でビルド対象に残るが、main.c から
  一切呼ばないので実行時は dead-link 状態。`prj.conf` から `fmstr` を抜こうとすると
  template の `#warning` が `-Werror` で落ちるため、削減を諦めて残置

## 動作確認
- ビルド:
- 書き込み:
- シリアル出力 (115200/8N1):
  ```
  31_onboard_touch_my
  Streaming TSI ch3 (El_1) signal/raw/status at ~100x divided rate.
  signal=... raw=... status=...
  ...
  ```
- 確認日:

## はまり点・解決メモ
- 取り込み時、SDK demo は `__repo__` ディレクトリに `mcuxsdk` の symlink を入れる。
  Windows + Git Bash の `cp -r` は symlink を解決して実体コピーするため、
  31 への複製時は **`__repo__` を必ず除外する**(個別 cp で対象を指定)
- (実機実行で気づいた件があれば追記)
