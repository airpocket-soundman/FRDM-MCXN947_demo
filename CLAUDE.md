# CLAUDE.md — FRDM-MCXN947_demo 規約

このリポジトリで作業する **AI アシスタント / 新規貢献者** 向けのルール集。
リポジトリ全体の趣旨は [README.md](./README.md) を、開発の経緯・実機ログは
[maker_contest_2026 の開発ログ](https://github.com/airpocket-soundman/maker_contest_2026/blob/main/_board_logs/frdm-mcxn947.md) を参照。

---

## このレポの目的を一言で

**MCUXpresso SDK の FRDM-MCXN947 サンプルを取り込み、原本と改造版を並べて比較・改良する。** 改造版は他人が clone してそのままビルドできる状態に保つ (= ローカル絶対パス禁止)。

---

## ローカル依存を排する 4 つの約束

1. **SDK パスは環境変数 `MCUXSDK_DIR` 経由で参照する。** ハードコードされた `D:/GitHub/...` や `C:/Users/<name>/...` を絶対に書かない。
2. **VS Code / CMake の正本は [.vscode/settings.json](.vscode/settings.json) の `cmake.configureArgs`** (env var で書かれている)。これがあらゆる preset レイヤより優先されるよう構成済み。
3. **MCUXpresso 拡張が機械固有パスで上書きするファイル** (`<NN>_<sample>/.vscode/mcuxpresso-tools.json` の `sdk.path`、`<NN>_<sample>/mcux_include.json` の `environment` ブロック等) はコミット時に env var プレースホルダ化する。詳細は [取り込み後に必ずやる "ローカル化" 作業](#取り込み後に必ずやる-ローカル化-作業) を参照。
4. **machine-local な上書きが必要なら `.vscode/settings.local.json`** に書く (`.gitignore` 済み)。共通設定 (`settings.json`) には触らない。

---

## サンプル取り込み方針 (App type の選択)

MCUXpresso for VS Code の **Import Example from Repository** には 2 種類あるが、**このレポでは原本・改造版とも `Freestanding application` を使う**。

| App type | 中身 | このレポでの使いどころ |
| --- | --- | --- |
| **`Freestanding application`** ✅ | SDK の必要ファイル(`board/`, `app/`, リンカスクリプト等)を指定先に **コピー** して自己完結化 | **原本も改造版も全部これ。** デフォルト |
| `Repository application` | `mcuxsdk/examples/...` 配下にプロジェクトを作り SDK ソースを **参照する** (コピーしない) | 通常使わない。一時的な疎通確認のみ |

**Freestanding 統一の理由**:

1. **配布性**: SDK 側のパス・バージョン依存が最小化される。clone した相手の SDK が違うバージョンでも、コピー済みのソースは固定で動く
2. **改造容易性**: `board/hardware_init.c` 等を直接編集できる(Repository は SDK 側の実体を参照するため編集が無視される)
3. **diff の読みやすさ**: 原本 (`00_*`) と改造版 (`01_*_my`) の diff が「サンプル間の純粋な差分」になり、SDK ツリーの構造変化が混ざらない
4. **再現性**: SDK 側で `git pull` しても、このレポ内のサンプルの挙動は変わらない

**Repository application は使わない**:

- 配布した相手に SDK パスを再現させる必要が出る
- SDK 更新で同じプロジェクトの挙動が変わる
- 編集が build に反映されないため改造で詰む

> RAM 実行 / Flash 書き込みは App type ではなく **Build Configuration** (リンカスクリプト `*_ram.ld` / `*_flash.ld`) で決まる。App type の話とは独立。

---

## ディレクトリ命名規約

```
<NN>_<sample_name>[_my]/
```

- **`<NN>`**: 2 桁の連番。同カテゴリの原本と改造版を **隣り合わせ** に置くために、`00`/`01`、`10`/`11` のようにペアで詰めて使う
- **`<sample_name>`**: SDK 由来の例題名 (例 `hello_world`, `led_blinky_peripheral`, `tflm_label_image`)。`_cm33_core0` のようなコア指定は省略してよい
- **末尾 `_my`**: 改造版のみ付ける。原本には付けない

例:

| ディレクトリ | App type | 由来 SDK パス |
| --- | --- | --- |
| `00_hello_world` | Freestanding | `examples/demo_apps/hello_world` |
| `01_hello_world_my` | Freestanding | (上の改造版) |
| `10_led_blinky_peripheral` | Freestanding | `examples/demo_apps/led_blinky_peripheral` |
| `11_led_blinky_peripheral_my` | Freestanding | (上の改造版) |
| `20_tflm_label_image` | Freestanding | `examples/eiq_examples/tflm_label_image` |

---

## 各サンプルディレクトリの中身

MCUXpresso for VS Code の Import Example (Freestanding) が生成する物理レイアウト (SDK v06 / 26.6.0):

```
<NN>_<sample>/
├─ CMakeLists.txt              アプリ部の CMake 設定
├─ CMakePresets.json           debug / release プリセット (mcux_include.json を include)
├─ mcux_include.json           ARMGCC_DIR / SdkRootDirPath 等の env 定義 (env var 化済み)
├─ Kconfig                     アプリ Kconfig
├─ prj.conf                    アプリ Kconfig 値 (必須。無いと Kconfig 段階で死ぬ)
├─ example.yml                 SDK メタデータ
├─ <sample>.c                  アプリソース (NPU 系は main.cpp など別の場合あり)
├─ readme.md                   SDK 由来の README (元のまま)
├─ .vscode/
│  ├─ mcuxpresso-tools.json     projectType=sdk-v2-freestanding (Freestanding 取り込み時の既定)
│  ├─ settings.json             サンプル単位の VS Code 設定
│  ├─ launch.json               デバッグ設定 (mcuxpresso-debug)
│  ├─ tasks.json                CMake build / configure / clean タスク
│  └─ c_cpp_properties.json     IntelliSense 設定
└─ frdmmcxn947_cm33_core0/      ボード固有部 (SDK の _boards/frdmmcxn947/<category>/<example>/ 由来)
   ├─ board_files.cmake         このフォルダ配下の C/H を build ターゲットに足す
   ├─ <sample>.mex              MCUXpresso Config Tools のプロジェクトファイル (pin mux 等)
   ├─ Kconfig.trace
   ├─ prj.conf                  board 側 Kconfig 値
   ├─ cm33_core0/               Core 専用ファイル (app.h / hardware_init.c など)
   ├─ frdmmcxn947/              ボード共通 (board.[ch], clock_config.[ch], 各種 *_config.h)
   └─ <sample>/                 ペリフェラル / pin_mux 等のサンプル固有ファイル
```

各サンプルには別途 `README.md` を置き、由来 SDK パス・動作確認結果・改造ポイント・はまり点を記録 (CLAUDE.md 末尾の [テンプレート](#各サンプルの-readmemd-に書くこと) を使う)。

---

## 取り込み後に必ずやる "ローカル化" 作業

MCUXpresso for VS Code の Import Example (Freestanding) 直後、**そのままだとローカル絶対パスを含んでいる**。**コミット前に必ず以下を実施**。

> **SDK 26.6.0 + Freestanding import 時点の状況** (2026-05 確認):
> - 項目 1 (`mcuxpresso-tools.json` の `projectType`) → 自動で `sdk-v2-freestanding` になる。確認のみで OK
> - 項目 2 (`<NN>_<sample>/prj.conf`) → 標準サンプル (`hello_world`, `led_blinky_peripheral`, `tflm_label_image`) では自動生成される。古い SDK や一部サンプルで欠ける場合のみ集約が必要
> - 項目 3 (`mcux_include.json` のローカル絶対パス) → **必ず手動 sanitize が必要**。下記参照

### 1. `<NN>_<sample>/.vscode/mcuxpresso-tools.json` の `projectType` を確認

PROJECTS パネルでバッジが「**MCUXpresso SDK 26.x.x**」と「**CMake**」のどちらで表示されるかは `projectType` で決まる。

| 状態 | `projectType` | `sdk` ブロック |
| --- | --- | --- |
| ❌ CMake バッジ (拡張から見て一般 CMake プロジェクト) | `cmake-freestanding` | 無し |
| ✅ MCUXpresso SDK バッジ | `sdk-v2-freestanding` | path / version / boardId / deviceId / coreId を記入 |

正しい例 (env var 化):

```jsonc
{
  "version": "25.3",
  "toolchainPath": "${userHome}/.mcuxpressotools/arm-gnu-toolchain-14.2.rel1-mingw-w64-x86_64-arm-none-eabi",
  "linkedProjects": [],
  "trustZoneType": "none",
  "multicoreType": "none",
  "projectType": "sdk-v2-freestanding",
  "sdk": {
    "path": "${env:MCUXSDK_DIR}",
    "version": "26.6.0",
    "boardId": "frdmmcxn947",
    "deviceId": "MCXN947",
    "coreId": "cm33_core0"
  },
  "projectName": "<NN>_<sample>"
}
```

> `${env:MCUXSDK_DIR}` を MCUXpresso 拡張がそのまま受け付けない場合がある。その時は **`mcuxpresso-tools.json` を `.gitignore` に入れて per-machine 扱い** にし、CMake 引数オーバーライド ([.vscode/settings.json](.vscode/settings.json)) を正本とする。

### 2. `<NN>_<sample>/prj.conf` の存在を確認 (無ければ作る)

SDK の一部サンプルは app 側に `prj.conf` が **存在しない**。その状態でビルドすると Kconfig 段階で

```
FileNotFoundError: '<APP_DIR>/prj.conf'
```

で死ぬ (SDK の `cmake/extension/kconfig.cmake` のフリースタンディング経路が `${APP_DIR}/prj.conf` の存在を必須としているため)。

対処は **board 側 prj.conf の中身を app 側にコピー / 集約** して作る。例 (led_blinky):

```ini
# 由来: examples/_boards/frdmmcxn947/demo_apps/led_blinky/cm33_core0/prj.conf
CONFIG_MCUX_COMPONENT_driver.reset=y
CONFIG_MCUX_PRJSEG_module.board.pinmux_project_folder=y
```

> SDK 26.6.0 の Freestanding import では `00_hello_world`, `10_led_blinky_peripheral`, `20_tflm_label_image` のいずれも app-level `prj.conf` が自動生成されることを確認済み。古い SDK や別系統のサンプルで欠ける場合のみこの集約が必要。

### 3. `<NN>_<sample>/mcux_include.json` のローカル絶対パス sanitize **(コミット前必須)**

Freestanding import 直後、`<sample>/mcux_include.json` の `debug-env` / `release-env` 内に **ローカル絶対パスがハードコード** される (例: `C:/Users/<your_user>/.mcuxpressotools/...`、`c:/Users/<your_user>/Documents/mcuxsdk`)。これをそのままコミットすると、他人 / 他マシンで build できなくなる。

**Sanitize 内容** (CMake preset の env var 構文 `$env{...}` を使う):

| Before (絶対パス) | After (env var) |
| --- | --- |
| `C:/Users/<you>/.mcuxpressotools` | `$env{USERPROFILE}/.mcuxpressotools` |
| `c:/Users/<you>/Documents/mcuxsdk` | `$env{MCUXSDK_DIR}` |

PowerShell でまとめてやる場合 (取り込み直後・コミット前に実行):

```powershell
# レポルートで
Get-ChildItem -Path . -Filter mcux_include.json -Recurse | ForEach-Object {
    $c = Get-Content $_.FullName -Raw
    # 自分のユーザ名で置き換える (USERPROFILE は env var 展開で動的に解決される)
    $c = $c -replace [regex]::Escape("$env:USERPROFILE\.mcuxpressotools".Replace('\','/')), '$env{USERPROFILE}/.mcuxpressotools'
    $c = $c -replace [regex]::Escape($env:MCUXSDK_DIR), '$env{MCUXSDK_DIR}'
    [System.IO.File]::WriteAllText($_.FullName, $c, (New-Object System.Text.UTF8Encoding $false))
    Write-Host "Sanitized: $($_.FullName)"
}
```

**保険として** [.vscode/settings.json](.vscode/settings.json) にワークスペース全体の `cmake.configureArgs` で同じ変数を `-D` 上書きしている。万が一 `mcux_include.json` の値が古い / 壊れた状態 (拡張が `SdkRootDirPath` を空文字に正規化することがある) でも、CMake は同じ `-DVAR=...` が複数ある場合 **最後の値** を採用するため、ワークスペース設定で救える。

**ビルドが通らない時のチェックリスト**:

1. `pwsh -File scripts/setup.ps1` を実行して `MCUXSDK_DIR` を設定済みか / VS Code を再起動済みか
2. `<sample>/debug/` (古い CMakeCache) を削除して再 Configure (キャッシュが効いて修正が反映されない件)
3. configure ログの末尾が `-DSdkRootDirPath=<MCUXSDK_DIR>/mcuxsdk -DCMAKE_TOOLCHAIN_FILE=<MCUXSDK_DIR>/mcuxsdk/cmake/toolchain/armgcc.cmake` で締められていて `-- Build files have been written to:` が出れば正常

---

## 各サンプルの README.md に書くこと

新しいサンプルディレクトリには必ず `README.md` を置き、最低限以下を記録する:

```markdown
# <NN>_<sample_name>

## 由来
- SDK バージョン: 26.6.0
- App type: Freestanding
- 由来パス: examples/demo_apps/hello_world (cm33_core0)
- 改造ベース: なし / 00_hello_world

## 改造ポイント
- (Freestanding 版のみ。差分の意図を箇条書き)

## 動作確認
- ビルド: OK / NG
- 書き込み: OK / NG
- シリアル出力 (115200/8N1):
  ```
  hello world.
  ```
- 確認日:

## はまり点・解決メモ
- (取り込み時 / ビルド時 / 実機実行時に出た症状と対処)
```

---

## やってはいけないこと

- **コミット前に絶対パスをチェック**: `D:/`, `C:/Users/`, `/home/` といった文字列が `.vscode/`, `<NN>_<sample>/`, `frdmmcxn947_cm33_core0/` 配下に紛れていないか確認(特に `mcux_include.json`)
- **`.vscode/settings.local.json` をコミットしない** (`.gitignore` 済み)
- **MCUXpresso 拡張が `mcuxpresso-tools.json` を書き戻した直後にそのままコミットしない**: 必ず `${env:MCUXSDK_DIR}` 化するか、`.gitignore` で除外する判断をする
- **Repository application を使わない**: 編集が build に反映されない / SDK パス依存が残る。原本も改造版も Freestanding で取り込む
- **`debug/`, `release/`, `flash_debug/`, `*.elf`, `*.bin`, `*.hex` をコミットしない** (`.gitignore` 済み)
