# Polarfire SoC Kit ターゲット依存部 ユーザーズマニュアル
- 作成者: 本田晋也
- 最終更新: 2024年04月23日

# ドキュメントの位置づけ

このドキュメントは，TOPPERS/FMP3カーネルの Polarfire SoC Kit ターゲット依存部を使用するために必要な事項を説明するものである．

# Polarfire SoC Kit ターゲット依存部の概要

Polarfire SoC Kit ターゲット依存部（GNU開発環境向け）は，TOPPERS/FMP3カーネルを，Microchip社のPolarfire SoC を搭載した以下のボード上で動作させる環境を構築するためのものである．

- PolarFire SoC FPGA Icicle Kit
- PolarFire SoC Discovery Kit

また，実ターゲットシステムに代えて，SoftConsoleに付属のRenodeを用いて実行することもできる．

## 対応するターゲットシステムとターゲット略称

Polarfire SoC Kit  ターゲット依存部の動作確認は，実機を用いて行っている．ボードに関する情報は，以下のウェブサイトにある．

- PolarFire SoC FPGA Icicle Kit
	- https://www.microchip.com/en-us/development-tool/mpfs-icicle-kit-es
- PolarFire SoC Discovery Kit
	- https://www.microchip.com/en-us/development-tool/mpfs-disco-kit

ターゲット略称等は次の通り．

	ターゲット略称：polarfire_soc_kit_gcc
	システム略称：polarfire_soc_kit
	開発環境略称：gcc

## ターゲット依存部の構成

Polarfire SoC Kit ターゲット依存部（GNU開発環境向け）は，チップ依存部としてPolarfireチップ依存部（GNU開発環境向け）を，コア依存部としてRISC-Vコア依存部（GNU開発環境向け）を使用している．

	target/
		polarfire_soc_kit_gcc/	Polarfire SoC Kit ターゲット依存部

	arch/
		riscv_gcc/common/			RISC-Vコア依存部
		riscv_gcc/polarfire_soc/	Polarfire SoCチップ依存部
		riscv_gcc/doc/				RISC-V依存部に関するドキュメント
		gcc/						GCC開発環境依存部


## 開発環境と動作確認条件

開発環境として，以下からダウンロードするSoftConsoleを使用する．

	https://www.microchip.com/en-us/products/fpgas-and-plds/fpga-and-soc-design-tools/soc-fpga/softconsole


動作確認を行ったバージョンは次の通り．

- 2022.2-RISC-V-747

## メモリマップ

l2limでの実行で確認している．

## 起動処理

ブート及び各種ハードウェアの初期化はSDKを用いている．SDKの初期化が終了後，sdk_enry.c に定義されている各コアの関数をSDKから呼び出し，これらの関数からFMP3カーネルのエントリ処理にジャンプすることで，カーネルの動作をスタートさせる．

# ターゲット定義事項の規定

Polarfire SoC Kit  ターゲット依存部は，RISC-Vコア依存部とPolarfireチップ依存部を用いて実装されている．RISC-Vコア依存部およびPolarfire依存部におけるターゲット定義事項の規定については，「RISC-V依存部 ユーザーズマニュアル」を参照すること．

# ドライバ関連の情報

## タイマドライバ

高分解能タイマは，CLINTの Machine mode タイマを用いて実現している．

## シリアルインタフェースドライバ

シリアルインタフェースドライバでは，Polarfireが内蔵するUARTをサポートしている．
ボード毎に使用しているポートは次の通りである．

- PolarFire SoC FPGA Icicle Kit
	- UART1
- PolarFire SoC Discovery Kit
	- UART0

ポートは，以下の通りに設定している．

	ボーレート：115200bps
	データ：8ビット
	パリティ：なし
	ストップビット：1ビット

## システムログの低レベル出力

システムログの低レベル出力は，シリアルインタフェースドライバが用いているのと同じUARTを用い，ポーリングにより文字を出力する方法で実現している．


# システム構築手順と実行手順

## システム構築（Makefile）

Polarfire SoC Kit 用のFMP3カーネルを構築する手順は，「TOPPERS/FMP3カーネル ユーザーズマニュアル」の「３．クイックスタートガイド」の章に記述されている通りである．

configure.rb -T polarfire_soc_kit_gcc -w -S "syslog.o banner.o serial.o serial_cfg.o logtask.o mmuart.o chip_serial.o"

## システム構築（SoftConsole）

### ワークスペースの作成

fmpのトップにワークスペースフォルダを作成(エクスプローラで作成)．名前は何でも良い．

- 例) ./workspace/

SoftConsoleを実行し，上記のフォルダ以下に更にワークスペースを作成する．

- 例) ./workspace/polarfire_soc_kit

## サンプルプロジェクトのインポート

以下のフォルダをインポートする（ディフォルトのコピーを指定）

- ./target/target/polafire_soc_kit_gcc/softconsole/sample1

## ターゲットボードの指定

プロジェクトにあるMakefileでボードを指定する．

- PolarFire SoC FPGA Icicle Kit
	- BOARD = MPFS_ICICLE_KIT
- PolarFire SoC Discovery Kit
	- BOARD = MPFS_DISCOVERY_KIT

指定がない場合は，PolarFire SoC FPGA Icicle Kit が有効となる．

## ビルド

sample1のプロジェクトを選択してビルド．

## 実機実行

- PolarFire SoC FPGA Icicle Kit の設定
 	- J9をショートしてオンボードデバッガを有効に．
 	- 左右のUSBを両方PCに接続．
 
 - PolarFire SoC Discovery Kit の設定
	- USBをPCに接続．

- 実行
	- 次のランチファイルでデバッグを開始．
	- sample1.lunch 

## シミュレータ実行

次のランチファイルでシミュレータの実行とデバッグを開始．

- sample1_start-platform-and-debug.lunch 


# SDKの変更点

sdkフォルダ以下には，以下で公開されているベアメタルサンプルのスタートアップコードやドライバがある．

https://github.com/polarfire-soc/polarfire-soc-bare-metal-examples

使用しているバージョンは次の通りである．

- 2024.01

オリジナルのソースコードからの変更点は次の通りである．

- platform/mpfs_hal/startup_gcc/mss_entry.S
	- trap_vector を無効に変更．

# リファレンス

## ディレクトリ構成・ファイル構成

	target/polarfire_soc_kit_gcc/
		E_PACKAGE				簡易パッケージのファイルリスト
		MANIFEST				個別パッケージのファイルリスト
		Makefile.target			Makefileのターゲット依存部
		polarfire_soc_kit.h		ターゲットのハードウェア資源の定義
		sdk_entry.c				SDKからのエントリ
		target_asm.inc			ターゲット依存部のアセンブリ言語用マクロ定義
		target_cfg1_out.h		cfg1_out.cのリンクに必要なスタブの定義
		target_check.trb		kernel_check.trbのターゲット依存部
		target_class.trb		ターゲット依存部のクラス定義
		target_ipi.h			プロセッサ間割込みのターゲット依存部
		target_kernel.cfg		カーネル実装のコンフィギュレーションファイル
		target_kernel.h			kernel.hのターゲット依存部
		target_kernel.trb		kernel.trbのターゲット依存部
		target_kernel_impl.c	カーネル実装のターゲット依存部
		target_kernel_impl.h	カーネル実装のターゲット依存部に関する定義
		target_rename.def		ターゲット依存部の内部識別名のリネーム定義
		target_rename.h			ターゲット依存部の内部識別名のリネーム
		target_serial.cfg		シリアルドライバのコンフィギュレーションファイル
		target_serial.h			シリアルドライバのターゲット依存定義
		target_sil.h			sil.hのターゲット依存部
		target_stddef.h			t_stddef.hのターゲット依存部
		target_syssvc.h			システムサービスのターゲット依存定義
		target_test.h			テストプログラムのターゲット依存定義
		target_timer.h			タイマドライバを使用するための定義
		target_unrename.h		ターゲット依存部の内部識別名のリネーム解除
		target_user.txt			ターゲット依存部のユーザーズマニュアル

	target/polarfire_soc_kit_gcc/sdk		SDKのファイル

	target/polarfire_soc_kit_gcc/softconsole		SoftConsole用のプロジェクト

## バージョン履歴


以上

