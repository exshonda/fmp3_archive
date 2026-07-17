# CLAUDE.md — TOPPERS/FMP3 STM32MP257F-DK ターゲット依存部

このファイルは，本ターゲット依存部で作業する際の手引きである．利用者向けの詳細は同じ
ディレクトリの `target_user.md` を参照し，本ファイルは**開発者・別セッション向けの全体像と
運用規約**をまとめる．

> **【開発対象】本ツリー `fmp3_3.3_svn` が今後の開発対象**（2026-06-03 切替）．
> 移植は元々 `fmp3_3.3`（移植元・旧）で行い，StepS/StepM 完了後，arm64 共通部が
> リファクタされた `fmp3_3.3_svn` へマージした．以降の変更は本ツリー側で行うこと．
> _svn へのマージ内容は `../../arch/arm64_gcc/stm32mp2/PORTING_STM32MP2.md` 末尾
> 「fmp3_3.3_svn への適用記録」を参照．

## 目的

TOPPERS/FMP3（AArch64 版）を **STMicroelectronics STM32MP257F-DK**（Cortex-A35 ×2 /
GICv2 / LPDDR4）へ移植したもの．i.MX8MM EVK 版を基にしている．**StepS（シングルコア）と
StepM（2 コア SMP）は実機検証済み**．

## パッケージ構成と外部依存

ビルドは `fmp3_3.3` 内で**完全に自己完結**している（`obj/*/Makefile` の `SRCDIR=../..`，
`TARGETDIR` は本ディレクトリを指す．外部リポジトリのコードには依存しない）．

外部に必要なのは**ツールと FW のみ**:
- **クロスツールチェイン**: `aarch64-none-elf-gcc`（14.3 で確認．12.3/13.1 でも可）．
- **TF-A（FSBL）**: `github.com/4ms/tf-a-stm32mp25`（`v2.10-stm32mp2-baremetal`，
  `BAREMETAL_IMAGE_LOADER=1`）を git 取得・ビルド．唯一の改変は SWD 用の IWDG1 無効化（dts）．
- **OpenOCD 0.12+**（SWD デバッグ時）．設定は本依存部の `openocd/` に同梱（ST 由来 GPL を改変）．

本依存部に同梱している補助物（著作権クリーン）:
- `minimal_boot/` — SWD 運用の landing pad（"Connect using OpenOCD" 出力 → WFE 停止）．
- `openocd/` — STM32MP25x 用 OpenOCD 設定一式（`openocd/README.md` に根拠と知見）．
- `swd-debug.gdb` — gdb 自己完結スクリプト（接続 → reset → load → PC 設定 → halt）．

## ドキュメント地図

| ファイル | 内容 |
|---|---|
| `target_user.md` | 利用者マニュアル（PC 環境・ビルド・TF-A・SD 作成・SWD 実行/デバッグ・ブートシーケンス・割込み/メモリ/タイマ仕様・注意事項） |
| `../../arch/arm64_gcc/stm32mp2/PORTING_STM32MP2.md` | 移植の設計・経緯の記録 |
| `openocd/README.md` | OpenOCD 設定の構成・根拠・本開発で得た知見・トラブルシュート |
| `../../utils/gdb_os_aware/` | OS Awareness（gdb Python: `stask`/`atask`/`sem`/`dtq`/…/`intr`/`spn`）。README.md / CLAUDE.md あり（Step2） |
| `target_os_awareness.py` | OS Awareness のターゲット依存部（GIC 割込み許可/禁止状態）。chip_os_awareness→core_os_awareness と連鎖（本ボードは追加なし） |
| `CLAUDE.md`（本ファイル） | 開発者向けの全体像と運用規約 |

## ビルドと実行（クイックリファレンス）

```bash
# ビルドディレクトリを作って構成 → ビルド（<FMP3> は fmp3_3.3 のパス）
mkdir <OBJ> && cd <OBJ>
ruby <FMP3>/configure.rb -T stm32mp257f_dk_arm64_gcc -w \
     -S 'syslog.o banner.o serial.o serial_cfg.o logtask.o stm32usart.o chip_serial.o'
# 生成された Makefile の PRC_NUM を 1（シングル）または 2（SMP）に設定（必須．既定 4 は不可）
make                       # -> fmp (ELF), main.bin

# 実機実行・デバッグ（<OBJ> で．Makefile.target がターゲットを提供）
make swd-run               # OpenOCD だけでロード＆実行（gdb 不要）
make swd-debug             # OpenOCD 自動起動＋gdb でデバッグ（gdb 終了で OpenOCD も終了）
make openocd               # 別端末用: OpenOCD だけ起動
make gdb                   # 別端末用: gdb だけ起動（OpenOCD 起動済みのこと）
make console               # UART コンソール(USART2)を開く（picocom/minicom/cu 自動選択．TTY/BAUD 上書き可）
```
- 同梱のビルド済み構成 `<FMP3>/obj/obj_stm32mp2`（`PRC_NUM=2`）でそのまま `make` も可．
- フル版アプリは SWD ロードが前提（実行アドレス `TEXT_START=0x88001000`）．

## OS Awareness（Step2, 実機検証済み）

gdb から FMP3 のカーネル状態を可視化する OS-awareness は **`../../utils/gdb_os_aware/`** へ
移動した（`os_awareness.py` + `README.md` + `CLAUDE.md`）。詳細はそちらを参照。要点のみ:

- **`aarch64-none-elf-gdb` は Python 非対応**なので **`gdb-multiarch`** を使う。
- ビルドディレクトリで **`make osdebug`**（本依存部の `Makefile.target` が提供。OpenOCD 自動起動
  + gdb-multiarch で `os_awareness.py` を読込）→ gdb 内で `continue`→Ctrl-C→`stask`/`atask`/`sem`。
- コマンド: `stask`(タスク静的) / `atask`(タスク動的＋レディキュー＋待ちキュー＋スタック使用量) /
  `sem`/`dtq`/`pdq`/`flg`/`mtx`/`mpf`(同期・通信オブジェクト) / `cyc`/`alm`(時間イベント) /
  `intr`(割込み要求ライン＋GIC 許可/禁止状態＋ハンドラ/ISR 関数名) / `spn`(スピンロック)。
  割込みの許可/禁止状態とハンドラ表(`_kernel_p_inh_table`)の読み出しはターゲット依存部
  `target_os_awareness.py`（→chip→core）が提供。ATT_ISR のラッパは kernel_cfg.c 解析で
  実 ISR 名(exinf)に解決。
- **SMP(PRC_NUM=2) は両コア halt** で一貫スナップショット（FMP3 実行後に
  `monitor stm32mp25x.a35_1 arp_examine; arp_halt`）。a35_0 のみ halt だと PRC2 のキューが不整合。

## 移植のキーポイント（変更を加える前に把握すること）

- **動作モード**: EL3 で起動し `start.S` がセキュア EL1 へドロップ（`TOPPERS_TZ_S`）．
  `chip_el3_initialize` で SCR/CPUECTLR(SMPEN)/**CPTR_EL3 のトラップ解除**を行う
  （TF-A が CPTR_EL3 トラップを残すため．未解除だと `mrs cpacr_el1` で EC=0x18 例外）．
- **GIC は GICv2(GIC-400)**．セキュア(Group0)割込みを **IRQ で配送**する（`gicc_init` TZ_S は
  `GICC_CTLR=ENABLEGRP0`，**FIQEN を立てない**）．FIQ 配送のままだとハンドラが GIC ack 前に
  FIQ を再許可して同一割込みが暴走再入する（StepS のタイマ FIQ ライブロックの原因だった）．
- **タイマ**: Secure Physical Timer（INTID 29）．周波数は `CNTFRQ_EL0` を実行時読み出し（≈64MHz）．
- **マルチコア(StepM)**: BL31/PSCI が無いため Core1 は `chip_mprc_initialize` で
  `CA35SYSCFG.VBAR_CR`(0x48802084) + `RCC.C1P1RSTCSETR`(0x44200408) により **EL3 直接起動**する．
- **メモリ**: TEXT=`0x88001000`（SWD．先頭ページは FSBL/landing pad が使うため 2 ページ目以降）/
  `0x88000000`（SD 直接ブート）．DATA=`0x90000000`（`.data` 初期値は ROM に置き `start.S` がコピー）．
- **共通部の改変（_svn）**: `arch/arm64_gcc/common/` は上書きせず差分のみ．`arm64.h`/`arm64_tool.h`
  の CPUECTLR・`enable_smp` ガードに A35 追加，`gic_kernel_impl.c` の GICv2 FIQEN ドロップは
  **`GIC_NO_FIQ_IN_SECURE` マクロで stm32mp2 限定**（`Makefile.chip` で `-D` 定義．zynqmp 系に影響なし）．
  プロセッサID取得は _svn ではターゲット依存部へ移設（`target_sil.h`/`target_kernel_impl.h`/
  `target_asm.inc`）．`SMP_CACHE_BYTES`・`.bss..cacheline_aligned` も追加．詳細は PORTING ドキュメント．
- **MMU 設定は静的テーブル方式（2026-06-03〜）**: `USE_ARM64_MMU_CONFIG_TABLE`（`Makefile.target`
  で定義）により，`target_mmu_init()` ではなく weak の **`arm64_memory_area[]`** テーブル
  （`target_kernel_impl.c`）で MMU を設定する（arm 依存部の `arm_memory_area[]` と同等の方式．
  `user_mmu_init()` フックは廃止）．**全 6 arm64 ターゲットを移行済み**（他 5 ターゲットは
  ビルド確認のみ・実機検証は本ターゲットのみ）．共通部のマクロ未定義時の従来経路
  （`target_mmu_init()` 呼び出し）はツリー外ターゲット互換のため残存．
  経緯は `baremetal/.steering/20260603-arm64-mmu-static-config/` と PORTING ドキュメント参照．

## ボード/TF-A の注意

- 移植先は **DK（LPDDR4）**．TF-A は `DTB_FILE_NAME=stm32mp257f-dk.dtb STM32MP_LPDDR4_TYPE=1`，
  FIP の DDR FW は `lpddr4_pmu_train.bin`．EV1（DDR4）とはここが異なる（SoC は同一 MP257F）．
- **SWD デバッグ用 FSBL は IWDG1 を無効化**（`fdts/stm32mp257f-dk.dts` の `&iwdg1` を
  `status="disabled"`）．製品では戻すこと（watchdog 無し FSBL は出荷不可）．

## 現状と次のタスク候補

- **完了・実機検証済み（`fmp3_3.3` 上）**: StepS（シングルコア）/ StepM（2 コア SMP．
  `Processor 1/2 start`，prc1/prc2 でのタスク実行）．
- **_svn 上も SWD 実機検証済み（2026-06-03．PRC_NUM=2, gcc 14.3, Entry=0x88001000）**:
  `make swd-run` でバナー・`Processor 1/2 start`・両コアのタスク交互実行を確認
  （MMU 静的テーブル化の前後で UART 出力一致．ログは
  `baremetal/.steering/20260603-arm64-mmu-static-config/`）．
- **ランタイムテスト完了（2026-06-05）: 機能 36 件＋MP 固有 10 件中 45/46 PASS**
  （DK 実機 SMP，`test/testexec.rb`＋`<FMP3>/TEST-EXEC/`）．cpuexc は全件 PASS．
  テストで `SIL_DLY_TIM1/2` を 70/44→**12/10** に修正（dlynse 実測，ASP3 と同値），
  swd-run の halt レース対策を適用．**mtrans2 のみ既知の制限**（sus/rsm 連射による
  ディスパッチ IPI 飽和で PRC1 のタスクコンテキストがスタベーション．
  `TEST_DELAY_TIME_NSE=1000` なら PASS）．詳細は PORTING_STM32MP2.md
  「ランタイムテストの実施」．
- **未検証**: SD 直接ブート（`target_user.md` 6 章に手順のみ）．SWD では（fmp3_3.3 で）検証済み．
- **候補**: SD 直接ブートの実機検証 / CM33・CM0+ コアの活用 / 各ペリフェラル（I2C・SAI・DMA・USB 等）/
  FPU コンテキストや高分解能タイマの扱い．

## 運用規約

- `target_user.md` は利用者マニュアル．句読点は TOPPERS スタイルの **「，」「．」**（「、」「。」は使わない）．
- 設計に影響する変更は `PORTING_STM32MP2.md`，OpenOCD 設定の知見は `openocd/README.md` に反映する．
- 品質チェック: 対象 `<OBJ>` で `make` が警告なく通ること，実機または SWD で UART 出力を確認すること．
- 新規ファイルを同梱したら `MANIFEST` に追記する．
