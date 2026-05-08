# FRDM-MCXN947_demo

NXP **FRDM-MCXN947** 評価ボード向けに、MCUXpresso SDK のサンプルを取り込み、改造したデモを **1 サンプル = 1 ディレクトリ** で並べる作業用リポジトリ。

開発の経緯・つまずき・実機の動作ログは別レポの開発ログにまとめる:
[maker_contest_2026 / FRDM-MCXN947 開発ログ](https://github.com/airpocket-soundman/maker_contest_2026/blob/main/_board_logs/frdm-mcxn947.md)

---

## 前提

| 必要なもの | バージョン目安 | 備考 |
| --- | --- | --- |
| Windows 10/11 | — | PowerShell 5.1+ または PowerShell 7+ |
| VS Code | 最新 | |
| MCUXpresso for VS Code 拡張 | 最新 | NXP 公式 |
| MCUXpresso Installer 経由で導入 | — | Arm GNU Toolchain / LinkServer / MCUXpresso Config Tools 等 |
| MCUXpresso SDK (west レイアウト) | v06 系 (26.x) | このレポとは **別ディレクトリ** に展開しておく |

SDK の取得手順は [開発ログのセットアップ章](https://github.com/airpocket-soundman/maker_contest_2026/blob/main/_board_logs/frdm-mcxn947.md#開発環境セットアップ) を参照。

---

## 初回セットアップ (どのマシンでも 1 回だけ)

このレポの全 `.vscode` / CMake 設定は **環境変数 `MCUXSDK_DIR`** を読む形に統一されている。よって本レポを clone した後、初回だけ以下を実行する。

```powershell
# このレポのルートで
pwsh -File scripts/setup.ps1
```

スクリプトは:

1. `MCUXSDK_DIR` が既に設定済みならそれを使う
2. なければ `~/Documents/mcuxsdk` などの候補を自動探索
3. それでも見つからなければパスを尋ねる
4. west root として妥当か検証 (`.west/`, `manifests/`, `mcuxsdk/cmake/toolchain/armgcc.cmake` の存在をチェック)
5. ユーザースコープの環境変数 `MCUXSDK_DIR` に保存 (再起動後も有効)

明示的にパスを渡すこともできる:

```powershell
pwsh -File scripts/setup.ps1 -SdkDir "C:\Users\me\Documents\mcuxsdk"
```

設定後 **VS Code を再起動** すれば、`.vscode/settings.json` の `${env:MCUXSDK_DIR}` が解決される。

---

## ディレクトリ構成

```
FRDM-MCXN947_demo/
├─ README.md                  このファイル
├─ CLAUDE.md                  AI 補助・新規貢献者向けの規約 (サンプル取り込み手順 / 命名 / ハマり点)
├─ .gitignore
├─ .vscode/
│   └─ settings.json          コミット対象。${env:MCUXSDK_DIR} ベースで全マシン共通
├─ scripts/
│   └─ setup.ps1              初回 1 回だけ実行 (環境変数の設定)
└─ <NN>_<sample>/             サンプル 1 件 = ディレクトリ 1 つ
    ├─ CMakeLists.txt / CMakePresets.json / mcux_include.json
    ├─ <sample>.c (.cpp)
    ├─ Kconfig / prj.conf
    ├─ .vscode/                 mcuxpresso-tools.json / settings / launch / tasks
    ├─ frdmmcxn947_cm33_core0/  ボード固有(board, clock_config, hardware_init, pin_mux など)
    └─ README.md                由来 SDK パス / 動作確認 / 改造ポイント / はまり点
```

`<NN>` は 2 桁のプレフィックス。原本と改造版を **隣り合わせで diff しやすく** するために連番で並べる:

| プレフィックス例 | 中身 |
| --- | --- |
| `00_hello_world` | SDK 由来そのままの **原本** (Freestanding application で取り込み、無編集) |
| `01_hello_world_my` | 上記の改造版 (同じく Freestanding。編集して挙動を変える) |
| `10_led_blinky_peripheral` | 別カテゴリの原本 (Freestanding) |
| `11_led_blinky_peripheral_my` | その改造版 |

> **App type は原本も改造版も `Freestanding application` で統一**。SDK 側のパス・バージョン依存を最小化するため。詳細な理由・命名規則・取り込み後の必須調整ファイルなどは [CLAUDE.md](./CLAUDE.md) を参照。

---

## ローカル依存を排する設計の要点

このレポを **どのマシンでも clone 直後に動かせる** ようにするための約束事:

1. **SDK の絶対パスは環境変数 `MCUXSDK_DIR` 経由でのみ参照する。** ハードコードされた `D:/GitHub/...` や `C:/Users/<name>/...` を `.vscode/`, `CMakeLists.txt`, スクリプトに書かない。
2. **MCUXpresso 拡張が機械固有パスを書き戻すファイル** (各サンプルの `<NN>_<sample>/.vscode/mcuxpresso-tools.json` と `<NN>_<sample>/mcux_include.json`) は **両方とも `.gitignore` 済みの per-machine 扱い**。CMake configure 用の env / cache var の正本は `<NN>_<sample>/CMakePresets.json` (env var sanitize 済み inline 定義) と [.vscode/settings.json](.vscode/settings.json) の `cmake.configureArgs`。新規取り込み時の対応手順は [CLAUDE.md](./CLAUDE.md#3-nn_samplemcux_includejson-の中身を-cmakepresetsjson-に-inline-化-取り込み直後必須) を参照。
3. **ツールチェーン (`ARMGCC_DIR`) は MCUXpresso Installer の標準パス** (`~/.mcuxpressotools/...`) に揃える。別場所にインストールしている人は `MCUXSDK_DIR` と同じ仕組みで env var (例 `ARMGCC_DIR`) を別途設定する想定。
4. **per-machine の上書きが必要になった場合は `.vscode/settings.local.json`** に書く (`.gitignore` 済み)。VS Code は両方をマージするので、共通設定はそのまま、個別事情だけローカルに逃がせる。

---

## 動作確認の流れ (要約)

1. SDK / Installer / VS Code 拡張は導入済み、`pwsh -File scripts/setup.ps1` 実行済み (環境変数 OK) を前提
2. VS Code でこのフォルダを開く
3. 対象サンプルの `<NN>_<sample>/CMakePresets.json` を CMake Tools が検出 → **CMake: Select Configure Preset** で `debug` を選択
4. **CMake: Configure** → 末尾に `-DSdkRootDirPath=<MCUXSDK_DIR>/mcuxsdk ...` が出ているか、`-- Build files have been written to:` が出ているかを確認
5. **CMake: Build** で `<sample>/debug/` に `.elf` / `.bin` / `.hex` が生成
6. MCUXpresso 拡張の **Flash** アクションで MCU-Link 経由 SWD 書き込み (USB Type-C は MCU-Link 側 J17 ポート)
7. シリアルは MCU-Link VCOM を **115200 / 8N1** で開く

詳細・ハマり点・サンプルごとの差分は [CLAUDE.md](./CLAUDE.md) と各 `<sample>/README.md` を参照。
