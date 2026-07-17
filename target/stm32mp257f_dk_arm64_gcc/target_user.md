# TOPPERS/FMP3 STM32MP257F-DK(AArch64) ターゲット依存部 利用ガイド

STMicroelectronics **STM32MP257F-DK** 向け TOPPERS/FMP3 ターゲット依存部の解説と利用方法．
i.MX8MM EVK 版（`target/imx8mm_evk_arm64_gcc/target_user.txt`）を基に，STM32MP257F-DK 用へ
全面改訂したもの．設計・移植の詳細は `arch/arm64_gcc/stm32mp2/PORTING_STM32MP2.md` を参照．

---

## 概要

`stm32mp257f_dk_arm64_gcc` 依存部は，ST 社の **STM32MP257F-DK**（Cortex-A35 の AArch64 モード）
をサポートする．サポートするプロセッサ数は 1～2 個で，`PRC_NUM` マクロで選択する．動作モードは
**セキュア EL1**（`TOPPERS_TZ_S`）．

- ターゲット略称: `stm32mp257f_dk_arm64_gcc`
  （SYS=`stm32mp257f_dk`, CHIP=`stm32mp2`, CORE=`arm64`, TOOL=`gcc`）

### ボード
動作確認を行ったボード:
- STMicroelectronics **STM32MP257F-DK**（MB1605, SoC: STM32MP257F, Cortex-A35 ×2 / GICv2 /
  LPDDR4）

### コンパイラ
動作確認を行った GCC（Arm GNU Toolchain, ベアメタル `aarch64-none-elf`）:
- `aarch64-none-elf-gcc` **14.3.Rel1**（12.3 / 13.1 でも動作確認）
- https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

### デバッグ環境
- **OpenOCD 0.12.0 + aarch64-none-elf-gdb**（ST-LINK V3 経由の SWD）で動作確認．
- 上流（i.MX8MM 版）の Partner-Jet2(ICE) は使用しない．

### 実行環境
- TF-A の **FSBL(BL2)** が PLL/LPDDR4 等を初期化し，本カーネルを EL3 で起動する
  （BL31/OP-TEE/U-Boot は使用しない，`BAREMETAL_IMAGE_LOADER=1` のフォーク）．
- 本カーネルの `start.S` が EL3→セキュア EL1 へドロップして動作する．

---

## プロセッサ数

`TNUM_PRCID` マクロ（ビルドディレクトリの `Makefile` の `PRC_NUM`）で **1～2** の範囲で設定する．
`PRC_NUM=2` で Core1(a35_1) を起動して SMP 動作する．

> 注意: ターゲット依存部の既定（`PRC_NUM` 未設定時）は 4 だが，本ボードのコアは 2 個までである．
> 必ず `PRC_NUM=1` または `2` を明示すること（後述のビルド手順参照）．

---

## カーネルのコンフィギュレーション

> 以下はカーネル共通の機能であり，本ターゲットでも同様に選択可能．

### カーネルタイマ方式
グローバルタイマ方式・ローカルタイマ方式の両方をサポート．

### スピンロック方式
ネイティブ方式・エミュレーション方式の両方をサポート．ネイティブ方式の最大数は 256 個．

---

## カーネルの使用リソース

プロセッサ毎にカーネルは以下のリソースを使用する．

- **タイマ**: コア内蔵の **Secure Physical Timer**（CNTPS, INTID 29 / PPI13）を使用．
  カウンタ周波数は CNTFRQ_EL0（実機 ≈ 64 MHz）を実行時に読み出す．
- **UART**: コンソールに **USART2**（ST-LINK 仮想 COM, 0x400E0000, IRQ147）を使用する．
  USART2 は TF-A により 115200 8N1 で初期化済みであることを前提とする（本ガイドで検証済みは
  USART2 のみ．USART6 については「注意事項」を参照）．
- **プロセッサ間割込み**: 0 番・1 番のプロセッサ間割込み（GIC SGI）を使用．
- **メモリ**: DDR を使用．
  - TEXT : `0x88001000`（SWD 実行用の既定）／ `0x88000000`（SD 直接ブート用，後述）
  - DATA : `0x90000000`（`.data` の初期値は ROM 側に置き `start.S` が RAM へコピー）

### 割込みコントローラ（GIC）
STM32MP2 の GIC は **GICv2(GIC-400)** モード（`TOPPERS_GIC_VER=2`）．
GICD=`0x4AC10000`, GICC=`0x4AC20000`．セキュア(Group0)割込みは **IRQ で配送**する
（`gicc_init` で `GICC_CTLR=ENABLEGRP0`，FIQEN を立てない）．これによりカーネルの
CPU ロック（IRQ マスク）モデルと整合する．

---

## メモリマップとスタック

### メモリ配置
| 領域 | アドレス | 備考 |
|---|---|---|
| TEXT（コード/rodata, ROM） | `0x88001000`（SWD 既定）／`0x88000000`（SD 直接ブート） | `Makefile.target` の `TEXT_START_ADDRESS`．`.data` の初期値も ROM 側に置き `start.S` が RAM へコピーする |
| DATA / BSS / スタック / ヒープ | `0x90000000`– | `DATA_START_ADDRESS`．`.bss` 内に `.stack*` を含む |
| 利用可能メモリサイズ | `0x4000_0000`（1GB） | `TOPPERS_MEM_SIZE` |

FSBL は bm-fw を `0x88000000` にロードし同アドレスへ entry する．SWD 運用では先頭ページ
（`0x88000000`–`0x88001000`）を避けるため TEXT を `0x88001000` にする（4・5 章および
「STM32MP257F-DK のブートシーケンス」節を参照）．

### MMU の設定

MMU の設定（メモリマップ）は，`target_kernel_impl.c` の静的テーブル
**`arm64_memory_area[]`**（エントリ数 `arm64_tnum_memory_area`）で定義する
（`USE_ARM64_MMU_CONFIG_TABLE` 指定時．arm 依存部の `arm_memory_area[]` と同等の方式）．
既定では以下の 2 領域を物理アドレス＝仮想アドレスでマップする．

| 領域 | アドレス | サイズ | 属性 |
|---|---|---|---|
| ペリフェラル（USART2/RCC/GIC 等） | `0x40000000` | `0x10000000` | Device-nGnRnE |
| DDR | `0x80000000` | `0x40000000` | Normal, Write-Back |

テーブルは weak 定義であり，アプリケーション側で同名のテーブルを定義することで
全体を差し替えることができる（旧方式の `user_mmu_init()` フックは廃止した）．

### スタック
| スタック | サイズ | 定義 |
|---|---|---|
| 非タスクコンテキスト用（プロセッサ毎, ISTK） | 既定 8KB（`DEFAULT_ISTKSZ=0x2000`） | `arch/arm64_gcc/stm32mp2/chip_kernel_impl.h`．`start.S` が各 EL で `my_istkpt` により設定 |
| アイドル用 | 768B（`DEFAULT_IDSTKSZ=0x0300`） | 同上 |
| タスクスタック | コンフィギュレーションで個別指定 | `CRE_TSK` 等．`.bss` の `.stack*` に配置 |

> AArch64 セキュア EL1 で動作し，例外・割込みは非タスクコンテキストスタック(ISTK)上で処理する
> （上流 i.MX 版と同様．FIQ/IRQ 専用スタックは分離しない）．

---

## 割込み管理

本ターゲットの割込みは GICv2(GIC-400) で管理する．利用者は TOPPERS 標準の静的 API
（`ATT_ISR`/`DEF_INH`/`CFG_INT`）で割込みサービスルーチン・割込みハンドラを登録する．

### 割込み番号（INTNO）の体系
AArch64/GIC の INTID をそのまま割込み番号に用いる．
- SGI（ソフトウェア生成割込み, プロセッサ間割込みに使用）: `0`–`15`
- PPI（プロセッサ固有割込み）: `16`–`31`
- SPI（共有ペリフェラル割込み）: `32`–

マルチコア（`PRC_NUM=2`）では，割込み先プロセッサを INTNO の上位 16bit で指定する
（PRC1 宛なら `(0x00010000 | INTNO)`，PRC2 宛なら `(0x00020000 | INTNO)`）．SPI を特定コアへ
固定する場合に用いる．SGI/PPI は各コアにローカルである．

主要な割込み番号:
| 用途 | INTID | 種別 |
|---|---|---|
| Secure Physical Timer（カーネルタイマ） | 29 | PPI |
| USART2（コンソール, SIO） | 147 | SPI |
| USART6（未検証） | 168 | SPI |
| プロセッサ間割込み（ディスパッチ要求等） | 0, 1 | SGI |

### 割込み優先度
`GIC_PRI_LEVEL=32`（優先度 32 段, GIC-400 の上位 5bit）．カーネルが扱う割込み優先度の範囲は
- `TMIN_INTPRI = -15`（最高位）
- `TMAX_INTPRI = -1`（最低位）

で，値が小さいほど高優先度である．カーネルタイマは `INTPRI_TIMER = TMAX_INTPRI - 1`（= -2）を
使用する．利用者割込みは原則この範囲内で設定する．

### ハンドラ登録の例（静的 API）
```c
/* 割込みサービスルーチン(ISR)を割り付ける例 */
ATT_ISR({ TA_NULL, EXINF, INTNO, isr_func, ISRPRI });
CFG_INT(INTNO, { TA_ENAINT | TA_EDGE, INTPRI });   /* 属性・優先度の設定 */

/* 割込みハンドラ(INH)を直接定義する例 */
DEF_INH(INHNO, { TA_NULL, inh_func });
CFG_INT(INTNO, { TA_ENAINT, INTPRI });
```
`INTNO` には上表の値（マルチコア時は PRCID を OR したもの）を指定する．実行時の個別許可・禁止は
`ena_int`/`dis_int` で行う．記述の詳細は TOPPERS/FMP3 のユーザーズマニュアル（静的 API）に従う．

---

## タイマの仕様

- カーネルタイマには **Secure Physical Timer**（`CNTPS`, INTID 29 / PPI）を使用する．
- カウンタ周波数は固定値を仮定せず，実行時に **`CNTFRQ_EL0`** を読み出して用いる（実機 ≈ 64MHz）．
- 割込み優先度は `INTPRI_TIMER = TMAX_INTPRI - 1`（「割込み管理」節参照）．
- 高分解能タイマ（`TCYC_HRTCNT`）は**定義しない**（`target_kernel.h`）．システム時刻はティック
  単位で管理する．グローバルタイマ方式・ローカルタイマ方式の双方を選択できる
  （「カーネルのコンフィギュレーション」節参照）．

---

## システムログ機能

システムログはグローバル方式で使用する．低レベル出力およびログタスクは
コンソール（USART2）の SIO ポートを使用する．

---

## ブートの流れについて

TF-A の FSBL(BL2) が LPDDR4 を初期化し，FIP のベアメタルペイロード（本カーネル）を
DDR にロードして **EL3 セキュア**で起動する（PSCI/BL31 は無い）．

本カーネルの `start.S` は EL3 から開始し，EL3 初期化後にセキュア EL1 へドロップする．
サブコア(Core1)は PSCI ではなく **EL3 直接起動**（`CA35SYSCFG.VBAR_CR` + `RCC.C1P1RSTCSETR`）で
立ち上げる．

---

## 実行方法

本ガイドは FMP3 利用者向けである．以下の構成を前提とする:
- 本 FMP3（`<FMP3>`）— 本パッケージ．
- TF-A（FSBL）— **git から取得してビルドする**（3 章）．
- **SWD（OpenOCD + GDB）での実行・デバッグ（5 章）が本ガイドで動作確認済みの方法**．準備として
  FSBL と landing pad を書いた SD（4 章）を用意する．landing pad（`minimal_boot/`）も OpenOCD 設定
  （`openocd/`）も**本依存部に同梱**しており，TF-A（git）以外の外部リポジトリは不要．
- **SD 直接ブート（6 章）は FMP3 + TF-A のみで完結するが，現状未検証**．

ドキュメント中のパス表記:
- `<FMP3>` : 本 FMP3 カーネルを展開したディレクトリ．
- `<OBJ>`  : 後述の手順で作成するビルドディレクトリ．
- `<TFA>`  : 3 章で取得・ビルドする TF-A のディレクトリ．

### 1. PC（ホスト）環境の構築

#### 1.1 動作確認環境
- OS: **Ubuntu 24.04.4 LTS（Noble Numbat）** / kernel 6.17 系で動作確認．
- 必要ツール: クロスコンパイラ，Ruby（コンフィギュレータ用），（SWD デバッグ時のみ）OpenOCD・GDB．

#### 1.2 クロスコンパイラ（aarch64-none-elf-gcc）
```bash
# Arm GNU Toolchain（aarch64-none-elf）を展開し PATH を通す
export PATH=/usr/local/tools/arm-gnu-toolchain-14.3.rel1-x86_64-aarch64-none-elf/bin:$PATH
aarch64-none-elf-gcc --version
```

#### 1.3 コンフィギュレータ実行用 Ruby
```bash
sudo apt-get install -y ruby     # 未導入なら
ruby --version                   # 2.x / 3.x
```

#### 1.4 （SWD デバッグ時のみ）OpenOCD + GDB
```bash
sudo apt-get install -y openocd  # 0.12.0 で動作確認
openocd --version
aarch64-none-elf-gdb --version   # クロスツールチェイン同梱
sudo apt-get install -y gdb-multiarch # OS-awareness 用 15.1 で動作確認
gdb-multiarch --version 
```

#### 1.5 ボード接続とパーミッション設定
- STM32MP257F-DK の **ST-LINK(USB)** をホストに接続する．シリアルコンソール（USART2,
  115200 8N1, `/dev/ttyACM0` 等）と SWD（OpenOCD）の両方がこの経路．

**(a) シリアルコンソール(ttyACM)を非 root でアクセス**
```bash
sudo usermod -aG dialout "$USER"     # 再ログインで有効
ls -l /dev/ttyACM0
```

**(b) （SWD 時のみ）ST-LINK を非 root でアクセスする udev ルール**
ST-LINK の VID（`0483`，STMicroelectronics）を一括許可する 1 行のルールを作成する
（製品 ID を列挙せず VID だけで V2/V3 の全バリアントを網羅）:
```bash
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", MODE="0660", GROUP="plugdev", TAG+="uaccess"' \
  | sudo tee /etc/udev/rules.d/60-openocd.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo usermod -aG plugdev "$USER"     # 再ログインで有効
```
> 開発機では VID だけの一括許可で十分．製品 ID を絞りたい場合は OpenOCD 同梱の
> `contrib/60-openocd.rules` を使う．`lsusb` に `STMicroelectronics STLINK-V3`（0483:3753）が
> 見えること．

### 2. FMP3 のビルド

サンプルアプリ `sample1` を例に説明する．

**(1) ビルドディレクトリの生成**
```bash
mkdir <OBJ>          # 例: obj_sample1
cd <OBJ>
ruby <FMP3>/configure.rb -T stm32mp257f_dk_arm64_gcc -w \
     -S 'syslog.o banner.o serial.o serial_cfg.o logtask.o stm32usart.o chip_serial.o'
```

**(2) プロセッサ数の設定**
生成された `<OBJ>/Makefile` の `PRC_NUM` を設定する（**必須**．未設定だと既定の 4 になり動作しない）:
```make
PRC_NUM = 1      # シングルコア（または 2 = マルチコア SMP）
```

**(3) ビルド**
```bash
cd <OBJ>
make             # -> fmp (ELF), main.bin を生成
```

> 補足: 本パッケージにはビルド済みディレクトリ `<FMP3>/obj/obj_stm32mp2`（`PRC_NUM=2`）が
> 同梱されており，そのまま `make` してもよい．

**メモリ配置の注意**: `Makefile.target` の `TEXT_START_ADDRESS` / `DATA_START_ADDRESS` で決まる．
- SWD 実行用（既定）: `TEXT=0x88001000` / `DATA=0x90000000`
  （先頭ページ 0x88000000–0x88001000 は FSBL が使うため 2 ページ目以降に置く）．
- SD 直接ブート用: FSBL は FIP ペイロードを **0x88000000 にロードし 0x88000000 へ entry** する．
  SD ブートする場合は `Makefile.target` の `TEXT_START_ADDRESS=0x0088000000` に変更して再ビルドする．

### 3. TF-A（FSBL）の取得とビルド

本カーネルは TF-A の FSBL(BL2) から EL3 で起動する．ベアメタルイメージを直接ロードする
`BAREMETAL_IMAGE_LOADER` 対応の TF-A フォークを **git から取得**する．

**(1) 取得**
```bash
git clone -b v2.10-stm32mp2-baremetal https://github.com/4ms/tf-a-stm32mp25.git
cd tf-a-stm32mp25          # 以降 <TFA>
```

**(2)（SWD デバッグ時のみ）IWDG1 の無効化**
DK は IWDG1（独立ウォッチドッグ, 32 秒）が有効で，SWD でブレーク停止すると約 32 秒で
リセットされデバッグできない．SWD デバッグする場合は `fdts/stm32mp257f-dk.dts` の `&iwdg1` を
無効化する:
```dts
&iwdg1 {
	timeout-sec = <32>;
	status = "disabled";   /* 開発用．製品では "okay" に戻すこと */
};
```
> ⚠️ この変更を入れた FSBL は watchdog 無しのため製品出荷不可．SD ブートのみで使う場合は不要．
>
> 補足: 「アプリ側で IWDG1 をリフレッシュし続ける」案は確実ではない．`IWDG1_KR`(0x44010000) への
> 書き込みには APB バスクロック(CK_BUS_IWDG1)を先に有効化する必要があり（TF-A の
> `stm32_iwdg_refresh` がそうしている），引き渡し後はクロックがゲートされるため空振りする．
> よって SWD デバッグでは **FSBL での無効化が確実**である．

**(3) ビルド**（DK は **LPDDR4**．DDR 初期化 FW は `lpddr4_pmu_train.bin`）
```bash
make PLAT=stm32mp2 DTB_FILE_NAME=stm32mp257f-dk.dtb \
     STM32MP_SDMMC=1 STM32MP_LPDDR4_TYPE=1 BAREMETAL_IMAGE_LOADER=1 \
     CROSS_COMPILE=aarch64-none-elf- LOG_LEVEL=40 dtbs fsbl
make PLAT=stm32mp2 BAREMETAL_IMAGE_LOADER=1 fiptool
# 出力: build/stm32mp2/release/tf-a-stm32mp257f-dk.stm32 / tools/fiptool/fiptool
```

### 4. 起動用 SD カードの作成（SWD デバッグ用）

5 章の SWD デバッグの前提として，**FSBL** と **"landing pad" アプリ**を書いた SD を用意する．
landing pad は DDR 初期化済み・キャッシュ OFF で停止する最小アプリで，OpenOCD はこの状態に
FMP3 をロードする．landing pad は本ターゲット依存部に **`minimal_boot/`** として同梱しており
（USART2 に "Connect using OpenOCD" を出力して停止する），外部リポジトリは不要である．

**(1) landing pad（minimal_boot）のビルド**
```bash
cd <FMP3>/target/stm32mp257f_dk_arm64_gcc/minimal_boot
make                       # -> build/main.bin（"Connect using OpenOCD" 版, 約72B）
```

**(2) パーティション作成**（GUID/開始セクタが規定値でないと BOOTROM/TF-A が拒否する）

> ⚠️ **デバイスの特定（最重要・誤ると別ディスクを破壊）**: まず `lsblk` で対象を確認し，
> **容量・型番で必ず確認**する．USB カードリーダは通常 `/dev/sdX`（パーティションは `sdX1`/`sdX2`…），
> 内蔵 SD スロットは `/dev/mmcblkN`（パーティションは `mmcblkNp1`/`p2`…）．以降の `/dev/sdX` は
> 自分のデバイスに置換し（パーティション番号は含めない），`mmcblk` の場合はパーティション名を
> `mmcblkNp1` のように読み替えること．

SD を `/dev/sdX` として:
```bash
sudo sgdisk -Z /dev/sdX
sudo sgdisk -go /dev/sdX
sudo sgdisk --resize-table=128 -a 1 \
  -n 1:34:545      -c 1:fsbla1    -t 1:8DA63339-0007-60C0-C436-083AC8230908 \
  -n 2:546:1057    -c 2:fsbla2    -t 2:8DA63339-0007-60C0-C436-083AC8230908 \
  -n 3:1058:1569   -c 3:metadata1 -t 3:8A7A84A0-8387-40F6-AB41-A8B9A5A60D23 \
  -n 4:1570:2081   -c 4:metadata2 -t 4:8A7A84A0-8387-40F6-AB41-A8B9A5A60D23 \
  -n 5:2082:10273  -c 5:fip-a     -t 5:19D5DF83-11B0-457B-BE2C-7559C13142A5 \
  -n 6:10274:18465 -c 6:fip-b     -t 6:19D5DF83-11B0-457B-BE2C-7559C13142A5 \
  /dev/sdX
```

**(3) FSBL を P1・P2 に書き込み**（`sdX1`→`mmcblkNp1` 等，自分のデバイスに置換）:
```bash
cd <TFA>
FSBL=build/stm32mp2/release/tf-a-stm32mp257f-dk.stm32
sudo dd if=$FSBL of=/dev/sdX1 bs=1M conv=fsync
sudo dd if=$FSBL of=/dev/sdX2 bs=1M conv=fsync
```
> SWD デバッグでは 3 章の **IWDG1 を無効化した FSBL** を使うこと（ブレーク停止でリセットされない）．

**(4) FIP（DDR 初期化 FW + landing pad）を作成し P5 に書き込み**:
```bash
cd <TFA>
tools/fiptool/fiptool --verbose create \
  --ddr-fw drivers/st/ddr/phy/firmware/bin/stm32mp2/lpddr4_pmu_train.bin \
  --bm-fw  <FMP3>/target/stm32mp257f_dk_arm64_gcc/minimal_boot/build/main.bin \
  build/stm32mp2/release/fip.bin
sudo dd if=build/stm32mp2/release/fip.bin of=/dev/sdX5 bs=1M conv=fsync
sync
```
SD をボードに挿してリセットし，コンソール（115200 8N1, `/dev/ttyACM0`）に
**"Connect using OpenOCD"** が出てハングすれば準備完了．以降は SD を差し替えずに 5 章で
FMP3 を SWD ロードする．

### 5. SWD（OpenOCD + GDB）による実行・デバッグ

> 本ガイドで**動作確認した方法**．SD 再書き込み無しで高速に反復できる．前提:
> - 4 章の SD（FSBL + landing pad）をボードに挿し，"Connect using OpenOCD" でハングさせておく．
> - **OpenOCD 用 STM32MP25x ボード設定は本ターゲット依存部に `openocd/` として同梱**している
>   （upstream OpenOCD 0.12 には STM32MP25x が未収録のため）．外部リポジトリは不要．
>   `interface/stlink-dap.cfg`・`target/swj-dp.tcl`・`mem_helper.tcl` は OpenOCD 同梱版を使用する
>   （OpenOCD 0.12 以降が必要）．

FMP3 は SWD 実行用に **`TEXT_START=0x88001000`（既定）** でビルドしておくこと．

**(0) make による実行（推奨）**
ビルドディレクトリ(`<OBJ>`)で，OpenOCD と gdb を起動できる make ターゲットを用意している
（`<OBJ>` で `make` 済みであること）:

| コマンド | 動作 | OpenOCD |
|---|---|---|
| `make swd-run`   | **gdb を使わず** OpenOCD だけでロード＆実行（reset→load→PC設定→resume→shutdown） | 完了で自動終了 |
| `make swd-debug` | OpenOCD を自動起動し gdb（`swd-debug.gdb`）でロード＆デバッグ（1 端末で完結） | **gdb 終了で自動終了** |
| `make openocd`   | OpenOCD だけを前面起動（別端末の `make gdb` と対で使う） | Ctrl-C で終了 |
| `make gdb`       | gdb だけを起動しロード＆デバッグ（OpenOCD は別端末で起動済みのこと） | （触らない） |
| `make console`   | UART コンソール(USART2)を開く（出力確認・キー入力用．別端末で使う） | （触らない） |
| `make osdebug`   | OpenOCD 自動起動＋`gdb-multiarch` で OS-awareness（タスク/同期オブジェクト等を可視化）を読み込む | **gdb 終了で自動終了** |

```bash
cd <OBJ>
make console        # 端末C: シリアルコンソールを開いておく（出力を見る）
make swd-run        # 単に動かすだけ（最も手軽．gdb 不要）
make swd-debug      # 1 端末でソースデバッグ（break 設定後 continue．そのまま走らせるなら continue）

# 2 端末に分けたい場合（OpenOCD を起動しっぱなしにして gdb を繰り返したいとき）
make openocd        # 端末1: OpenOCD を起動して放置
make gdb            # 端末2: gdb を起動（再ビルド後はこれを再実行するだけ）
```
- `make console` は `picocom`／`minicom`／`cu` を自動選択して開く（115200 8N1）．ポートやボーレートは
  `make console TTY=/dev/ttyACM1 BAUD=115200` のように上書きできる．アプリの UART 出力確認と
  キー入力（割込みテスト等）に使う．
- 用途は **動かすだけなら `swd-run`，デバッグするなら `swd-debug`** の 2 択でよい．`swd-debug` で
  起動して `continue`（または `monitor resume`）すれば「ロードして実行」と同じ状態になる．
- `make swd-debug` ＝ `make openocd` + `make gdb` を 1 端末にまとめたもの．OpenOCD をバックグラウンドで
  起動し（ログは `<OBJ>/openocd-bg.log`），gdb が終了すると `trap` で OpenOCD も停止する．
- `make openocd` / `make gdb` は **別端末に分ける**運用で，OpenOCD を起動したまま `make gdb` を
  繰り返せる（反復が速い）．
- 上書き可能な変数: `ELF`（既定 `fmp`），`ENTRY_PC`（既定は **ELF のエントリポイント
  から自動取得**．本ターゲットの固定リンカスクリプトは `ENTRY(start)` を指定して
  おり `TEXT_START_ADDRESS` と一致するが，ELF を正とする），`EN_CA35_1`（既定 `0`）．
- **`make swd-run` は OpenOCD 単体でロード＆実行が完結する**（gdb 不要）．`resume` 後に `shutdown` で
  OpenOCD を抜けるが，CPU はそのまま実行を継続する．
- **`make osdebug`** は OS-awareness（FMP3 のタスク・同期/通信オブジェクト・割込み等を gdb から
  可視化する `stask`/`atask`/`sem`/… コマンド）を読み込んで起動する．`gdb-multiarch`（Python 対応）が
  必要．詳細は `../../utils/gdb_os_aware/README.md` を参照．

以下 (1)〜(2) は上記を手動で行う場合の手順（仕組みの説明）である．`make openocd` が (1)，
`make gdb` が (2) に対応する．

**(1) OpenOCD 起動**（同梱の `openocd/` で実行．SMP では a35_1 を examine させない
ため `EN_CA35_1 0` を付ける．シングルコアでも付けてよい）:
```bash
cd <FMP3>/target/stm32mp257f_dk_arm64_gcc/openocd
openocd -c "set EN_CA35_1 0" -f openocd.cfg
# Listening on port 3333 (a35_0), 4444 (telnet), 6666 (tcl)
```

**(2) ロード＆デバッグ（gdb だけで完結）**
同梱の gdb スクリプト `swd-debug.gdb`（依存部ルート直下，自己完結型）を使う（telnet 不要）:
```bash
aarch64-none-elf-gdb <OBJ>/fmp \
  -x <FMP3>/target/stm32mp257f_dk_arm64_gcc/swd-debug.gdb
```
`swd-debug.gdb` は `:3333` に接続し，
**reset run（DDR 初期化・landing pad で停止）→ a35_0 を examine/halt → load → エントリへ PC 設定**
までを行い，**halt のまま待機**する．`break` 設定後に `continue` でソースデバッグできる
（そのまま走らせるだけなら `continue` または `monitor resume`）．実行後も gdb プロンプトに留まる．
gdb を使わずに単に動かすだけなら `make swd-run`（OpenOCD 単体）で足りる．

手動で同等のことを行う場合（gdb のコマンド）:
```
(gdb) target extended-remote localhost:3333
(gdb) monitor reset run
(gdb) shell sleep 3                                  # DDR 初期化 + landing pad 起動待ち
(gdb) monitor stm32mp25x.a35_0 arp_examine           # 起動後に examine（reset 直後は早すぎる）
(gdb) monitor stm32mp25x.a35_0 arp_halt
(gdb) load
(gdb) monitor reg pc 0x88001000                      # ELF の Entry に合わせる
(gdb) monitor resume
```
- エントリは `aarch64-none-elf-readelf -h fmp | grep Entry` で確認（ビルドにより変わる）．
- `Failed to write memory at 0x80000fe4` の警告は，ベアメタル FSBL に ST デバッグ待ち合わせ機構が
  無いことによるもので**無害**（同梱 OpenOCD 設定の reset イベントで catch 済み．これにより
  gdb-attach 拒否を回避し gdb だけで完結する）．
- **ソースデバッグ**する場合は `swd-debug.gdb`（resume せず halt 状態で待機）を使い，
  `break`／`continue` で進める．
- （参考）telnet コマンドポート(4444)から `reset run; arp_examine; arp_halt; load_image; reg pc;
  resume` を直接送る運用も可能だが，通常は上記 gdb だけで足りる．

**(3) 期待される出力（sample1）**:
```
TOPPERS/FMP3 Kernel Release 3.3.x for STM32MP257F-DK CA35(AArch64 Secure) ...
Processor 1 start.
（PRC_NUM=2 なら）Processor 2 start.
Sample program starts (exinf = 1).
TASK1_1 is running on prc1 (001) .   |
（PRC_NUM=2 なら）TASK2_1 is running on prc2 (001) .   |
  ...（カウンタが増加し続ける）
```

### 6. 起動用 SD カードの作成（SD 直接ブート）（未検証）

> ⚠️ この章は手順のみ記載で**現状未検証**である（動作確認は 5 章の SWD で実施）．
> FMP3 を SD から直接起動する．landing pad（`minimal_boot`）は不要．

`main.bin` は 2 章で **`TEXT_START_ADDRESS=0x0088000000` でビルドしたもの**を使う．

**(1) パーティション作成・FSBL 書き込み**: 4 章 (2)(3) と同じ
（SD 直接ブートでは IWDG1 は有効でもよいが，その場合アプリ側で 32 秒以内のリフレッシュが必要）．

**(2) FIP（DDR 初期化 FW + FMP3 の main.bin）を作成し P5 に書き込み**:
```bash
cd <TFA>
tools/fiptool/fiptool --verbose create \
  --ddr-fw drivers/st/ddr/phy/firmware/bin/stm32mp2/lpddr4_pmu_train.bin \
  --bm-fw  <OBJ>/main.bin \
  build/stm32mp2/release/fip.bin
sudo dd if=build/stm32mp2/release/fip.bin of=/dev/sdX5 bs=1M conv=fsync
sync
```

#### 起動方法

1. SD をボードに挿し，ブートスイッチを SD ブートに設定する．
2. ST-LINK の USB をホストへ接続（USART2 経路）．
3. コンソールを開く（115200 8N1）:
   ```bash
   picocom -b 115200 /dev/ttyACM0      # または minicom -b 115200 -D /dev/ttyACM0
   ```
4. リセット．TF-A ログに続き，FMP3 のバナーとサンプル出力（期待値は 5 章 (3) と同じ）が
   出れば成功．

---

## STM32MP257F-DK のブートシーケンス

電源投入／リセットから本カーネル（または landing pad）が走り出すまでの流れ．TF-A の FSBL(BL2)
が DDR を初期化し，FIP のベアメタルペイロードを **EL3 セキュア**で起動する（BL31/OP-TEE/U-Boot は
使用しない `BAREMETAL_IMAGE_LOADER` 構成）．

```
電源ON / Reset
  └─ BOOTROM（SoC 内蔵）
       ・ブートピンに従い SD を選択，SD の P1/P2(fsbl) から FSBL を読み込む
  └─ BL2 = TF-A FSBL（セキュア EL3）
       ① FIP から DDR 初期化 FW(lpddr4_pmu_train, image id=26)を SYSRAM にロード
       ② プラットフォーム設定 → LPDDR4 を初期化（実機: 1200MHz, 4096MB）
       ③ FIP から bm-fw(image id=27)を DDR の 0x88000000 にロード
            ・bm-fw = SD ブート時は FMP3 / SWD 運用時は landing pad(minimal_boot)
       ④ "Booting BL31", Entry=0x88000000, SPSR=0x3cd(EL3h, DAIF マスク) で bm-fw へ分岐
  └─ bm-fw（セキュア EL3 で開始）
       ・landing pad: USART2 に "Connect using OpenOCD" を出力し WFE で停止
                      （MMU/キャッシュ OFF のまま → OpenOCD ロードがコヒーレント）
       ・FMP3:        start.S が EL3 初期化(SCR/CPTR/CPUECTLR)→ セキュア EL1 へドロップ
                      → MMU/キャッシュ有効化 → カーネル起動 → タスク実行
```

実機 BL2 ログ例（抜粋）:
```
NOTICE:  BL2: v2.10-stm32mp2-r2.0(release)
INFO:    BL2: Loading image id 26 ...          ← DDR 初期化 FW
INFO:    RAM: LPDDR4 1x32Gbits 1x32bits 1200MHz
INFO:    Memory size = 0x100000000 (4096 MB)
INFO:    BL2: Loading image id 27 at address 0x88000000   ← bm-fw
NOTICE:  BL2: Booting BL31
INFO:    Entry point address = 0x88000000
INFO:    SPSR = 0x3cd
```

> ポイント: FSBL は bm-fw を **0x88000000 にロードし同アドレスへ entry** する．
> SD 直接ブートは `TEXT_START=0x88000000`，SWD は先頭ページを避け `0x88001000`（4 章・5 章参照）．

---

## SWD デバッグの詳細（OpenOCD と gdb の役割分担）

SWD デバッグは **OpenOCD** と **gdb** の 2 段構成で動く．OpenOCD がハードウェア（ST-LINK→
CoreSight デバッグ）を操作し，gdb は OpenOCD の gdb サーバ越しにソースレベルで操作する．

### OpenOCD が行うこと
- **ST-LINK(SWD/dapdirect)** で SoC の CoreSight **DAP** に接続する．
- DAP 上の **AP / コア**を定義する（同梱 `target/stm32mp25x.cfg`）:
  - `ap0`(APB), `axi`(AXI, ap-num 4), `ap2`(CM0+用), `ap8`(CM33用)
  - `a35_0`(dbgbase 0x80210000), `a35_1`(0x80310000) … **`target smp a35_0 a35_1`** で SMP グループ
  - `m33`, `m0p`
- **リセット制御**（`reset_config srst_only`）と，リセット後のコアの **examine（デバッグ接続確立）／
  halt／resume**，**メモリ R/W**，レジスタ R/W，ブレークポイント設定を担う．
- 各コアに **gdb サーバ**を立てる（examine 順にポート自動割当）:
  - **a35_0(+a35_1, SMP) → :3333**, m33 → :3334, m0p → :3335
  - ほかに **telnet :4444**, **tcl :6666**．
- **本構成の改変点**（ベアメタル FSBL 対応）:
  - reset イベントの ST デバッグ依存処理を無効化／`catch` 化し，reset 後に a35_0 を確実に examine
    （→ gdb-attach 拒否を回避）．詳細は `openocd/README.md`．
  - **`EN_CA35_1 0`** で a35_1 を examine しない（SMP 時に SoC 側 `start_cpu1` が Core1 を起動
    できるようにする．OpenOCD が a35_1 を halt 保持すると Core1 が走らないため）．

### gdb が行うこと
- OpenOCD の gdb サーバ(:3333)へ **RSP 接続**（`target extended-remote localhost:3333`）．
- **`monitor <cmd>`** で OpenOCD へコマンドを委譲（gdb 内から実行）:
  `reset run`（FSBL 再実行→DDR 初期化→landing pad で停止），`arp_examine`／`arp_halt`（a35_0 を
  デバッグ接続・停止），`reg pc <addr>`（PC 設定），`resume`（実行）．
- **`load`** で ELF の各セクションをターゲットメモリ（LMA）へ転送．
- **ソースレベルデバッグ**（`break`/`step`/`continue`/`print`/レジスタ・メモリ参照）．

### 典型シーケンス（gdb 完結．5 章 (2) と同じ）
```
gdb: target extended-remote :3333     → OpenOCD(a35_0)に接続
gdb: monitor reset run                → OpenOCD: SRST→FSBL→DDR初期化→landing pad停止
gdb: (sleep)                          → DDR初期化+landing pad起動を待つ
gdb: monitor arp_examine / arp_halt   → OpenOCD: a35_0をデバッグ接続して停止
gdb: load                             → OpenOCD経由でELFをDDRへ転送
gdb: monitor reg pc 0x88001000        → OpenOCD: PCをエントリへ
gdb: monitor resume                   → OpenOCD: a35_0実行 → FMP3起動
```
> a35_0 の `arp_examine` は **reset 直後では早すぎて失敗**するため，`reset run` 後に少し待って
> から行う（CA35 が起動しきってから）．この点は telnet 運用でも同じ．

### gdb でのレジスタ参照・ブレークポイントの注意
- gdb から参照できるシステムレジスタは OpenOCD のターゲット記述に含まれるものに限られる．
  - 参照可: `$ESR_EL1` / `$ELR_EL1` / `$SPSR_EL1` など EL1 の主要レジスタ．
  - 参照不可（`void` になる）: `$FAR_EL1` / `$CNTFRQ_EL0` / `$*_EL3` 等．これらは `monitor` 経由
    （OpenOCD のメモリ・レジスタ読み出し）で確認する．
- **ハードウェアブレークポイント**は例外処理中（`PSTATE.D=1`, デバッグ例外マスク）には発火しない．
  `daifclr` でデバッグ例外が解除された区間でのみ有効である．
- HW ブレークポイント資源（BRP）が枯渇・残留した場合，OpenOCD 再起動や `reset run` では CPU
  デバッグレジスタが残ることがある．確実に消すにはボードを**電源 OFF/ON** する．

---

## 注意事項・既知の事項

- **コンソール UART(USART2)** は TF-A が 115200 8N1 で初期化済み．本ドライバ
  (`stm32usart.c`)は BRR 等を再設定せず TE/RE/UE を有効化するのみ．
- **USART6 について**: `Makefile.target` の `-DUSE_UART6` で選択は可能（**コンパイルは通る**）が，
  **実機未検証**である．TF-A はコンソールとして USART2 のみを初期化するため，USART6 を使うには
  クロック供給・ピンマルチプレクサ・ボーレートの設定を別途追加する必要がある（現状の依存部は
  これらを行わない）．したがって現状で動作確認済みのコンソールは **USART2 のみ**．
- **割込み配送**: セキュア(Group0)割込みは GICv2 で **IRQ 配送**（`gicc_init` の `FIQEN=0`）．
  FIQ 配送のままだと，ハンドラが GIC ack 前に FIQ を再許可して同一割込みが暴走再入する．
- **マルチコア起動**: BL31/PSCI が無いため Core1 は `chip_mprc_initialize`（マスタのみ・EL1）で
  `CA35SYSCFG.VBAR_CR`(0x48802084) + `RCC.C1P1RSTCSETR`(0x44200408) により EL3 直接起動する．
- **共通部の改変**: 本移植では `arch/arm64_gcc/common/` にも最小限の変更がある
  （`-DTOPPERS_CORTEX_A35` 対応，GICv2 の `FIQEN=0` 化）．詳細は PORTING ドキュメント参照．
- **SD ブートと SWD で TEXT アドレスが異なる**: SD ブートは `TEXT_START=0x88000000`，
  SWD は `0x88001000`．用途に応じて `Makefile.target` を設定して再ビルドすること．
- **同梱 OpenOCD 設定の改変**: `openocd/target/stm32mp25x.cfg`（GPL-2.0-or-later, ST 由来）は
  本ターゲット向けに改変している（`_handshake_with_wrapper` を no-op 化し，reset/examine イベントの
  各手順を `catch` で包む）．これによりベアメタル FSBL でも reset 後に a35_0 が確実に examine され，
  **gdb だけで（telnet 無しで）reset→load→実行が完結**する．改変箇所はファイル内にコメントで明示．
- **`minimal_boot`（同梱）の位置づけ**: 本ターゲット依存部に同梱した landing pad
  （`minimal_boot/`，USART2 に "Connect using OpenOCD" を出力して WFE 停止．MMU/キャッシュは
  触らないため OpenOCD ロードがコヒーレント）．SD 直接ブート（6 章）では**使用しない**．
  SWD（4・5 章）で，DDR 初期化済み・キャッシュ OFF の土台としてのみ利用する．

---

## 同梱物とライセンス

本ターゲット依存部には，外部リポジトリ依存を減らすため以下を同梱している．

| 同梱物 | 内容 | ライセンス |
|---|---|---|
| `minimal_boot/` | SWD 運用の landing pad（新規作成）．USART2 に "Connect using OpenOCD" を出力し WFE 停止．MMU/キャッシュは触らない | TOPPERS ライセンス（新規作成） |
| `openocd/`（`openocd.cfg`, `board/stm32mp25x_dk.cfg`, `target/stm32mp25x.cfg`） | STM32MP25x 用 OpenOCD 設定（upstream 0.12 未収録のため同梱）．ベアメタル FSBL + gdb 完結向けに改変 | GPL-2.0-or-later（STMicroelectronics 由来を改変） |
| `swd-debug.gdb` | gdb だけで reset→load→ソースデバッグを行う自己完結スクリプト | TOPPERS ライセンス（新規作成） |

外部から取得するもの:
- **TF-A**（FSBL）: `github.com/4ms/tf-a-stm32mp25`（3 章）．
- OpenOCD 同梱の `interface/stlink-dap.cfg` / `target/swj-dp.tcl` / `mem_helper.tcl`（OpenOCD 0.12 以降）．

> OpenOCD 設定の改変点と根拠は `openocd/README.md` に，移植全体の設計は
> `arch/arm64_gcc/stm32mp2/PORTING_STM32MP2.md` に記載している．

---

## 変更履歴

- 2026/07/17
  - `stm32mp257f_dk.ld` から `.bss..cacheline_aligned` セクションの配置指定を削除．
    arm64 コア依存部でロック変数をキャッシュライン長の配列として確保する方式に変更し，
    型自身がサイズとアラインメントを保証するようになったため，リンカスクリプトによる
    セクションの集約が不要になった（詳細は `arch/arm64_gcc/doc/arm64_user.txt` の
    2026/7/17 の項を参照）．

- 2026/06/03
  - STM32MP257F-DK ターゲット依存部の初版．i.MX8MM EVK 版を基に移植．
  - GICv2(GIC-400) 対応，STM32 USART ドライバ，CPTR_EL3 トラップ解除，
    タイマ FIQ ライブロック修正（GICv2 FIQEN=0），Core1 の EL3 直接起動（SMP）．
  - シングルコア／2 コア SMP を実機で動作確認．
  - メモリマップとスタック・割込み管理・タイマの仕様・同梱物とライセンスの各節を追加．
    SWD デバッグの詳細に gdb のレジスタ参照／ブレークポイントの注意を追記．

