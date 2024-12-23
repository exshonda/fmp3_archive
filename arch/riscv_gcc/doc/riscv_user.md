# RISC-V依存部 ユーザーズマニュアル
- 対応バージョン: Release 3.3.0
- 最終更新: 2023年4月18日

このドキュメントは，TOPPERS/FMP3カーネルのRISC-V依存部を使用するために必要な事項を説明するものである．

# 本マニュアルの位置づけ

TOPPERS/FMP3カーネルのRISC-V依存部について説明したものである．

# RISC-V依存部の概要

## RISC-Vコア依存部

FMP3カーネルのRISC-Vコア依存部は，RV32I/RV64I/RV32E準拠したプロセッサコアを持つターゲットシステム（チップ）に共通に使用できる部分である．

現時点では，GNU開発環境のみに対応している．

## ターゲット定義事項の規定

### 割込み処理に関する規定

RISC-Vコア依存部では，割込み番号と割込みハンドラ番号を，次のように付与している．

複数のプロセッサに接続された割込み要求ラインに対しては，割込み番号の上位16ビットを0とし，下位16ビットで，その中のどの割込み要求ラインであるかを識別する．また，1つのプロセッサのみに接続された割込み要求ラインに対しては，割込み番号の上位16ビットを，接続されたプロセッサのID番号とし，下位16ビットで，その中のどの割込み要求ラインであるかを識別する．

割込みハンドラはプロセッサ毎に登録することから，同じ割込み要求に対応する割込みハンドラであっても，プロセッサ毎に異なる割込みハンドラ番号を付与する必要がある．そこで，割込みハンドラ番号の上位16ビットを，その割込みハンドラを実行するプロセッサのID番号とし，下位16ビットで，その中でどの割込みハンドラであるかを識別する．

### PLIC依存部における割込み処理に関する規定

PLIC依存部では，割込みの禁止（dis_int），割込みの許可（ena_int），割込み要求のクリア（clr_int），割込み要求のチェック（prb_int）では，以下の例外を除いて，複数のプロセッサに接続された割込み要求ラインと，サービスコールを呼び出したプロセッサのみに接続された割込み要求ラインのみを操作対象にできる．この制約に合致せずにこれらのサービスコールを呼び出した場合，E_PARエラーとなる．

## CPU例外処理に関する規定

異なるプロセッサで発生するCPU例外は異なるCPU例外であると扱い，CPU例外ハンドラはプロセッサ毎に登録することから，同じ種類のCPU例外であっても，プロセッサ毎に異なるCPU例外ハンドラ番号を付与する必要がある．そこで，ARMコア依存部では，CPU例外ハンドラ番号の上位16ビットを，そのCPU例外ハンドラを実行するプロセッサのID番号とし，下位16ビットで，以下の中のどのCPU例外ハンドラであるかを識別する．

	EXCNO_MISALIGNED_FETCH  0
	EXCNO_FAULT_FETCH       1
	EXCNO_IINST             2
	EXCNO_BREAKPOINT        3
	EXCNO_MISALIGNED_LOAD   4
	EXCNO_FAULT_LOAD        5
	EXCNO_MISALIGNED_STORE  6
	EXCNO_FAULT_STORE       7
	EXCNO_USER_ECALL        8
	EXCNO_SUPERVISOR_ECALL  9
	EXCNO_HYPERVISOR_ECALL  10
	EXCNO_MACHINE_ECALL     11
	EXCNO_FETCH_PAGE_FAULT  12
	EXCNO_LOAD_PAGE_FAULT   13
	EXCNO_STORE_PAGE_FAULT  15

## FPUサポートに関する規定

RISC-Vコア依存部は，すべての処理単位でFPUを使用できるようにする方法のみサポートする．タスク切換えや割込み
ハンドラ／CPU例外ハンドラの出入口でFPUレジスタを保存／復帰する．

# ベクタテーブル

ベクターテーブル方式を用いる場合は，チップ依存部でベクターテーブルを用意して，mtvec に登録すること．

# 割込み・例外エントリ

割込み・例外エントリについて次のマクロを用意する．

- USE_RISCV_DIRECT_TRAP ダイレクトモードを有効に

ダイレクトモードを有効にする．チップ依存部では，mtvec にcore_int_entryを登録すること．このモードでは，core_int_entry において，scratch を使用するため，チップ依存部ではこのレジスタを使用しないようにすること．

上記マクロを定義しない場合は，ベクタテーブルの例外とカーネル管理の割込みのエントリに対して以下の関数を登録すること．

- core_int_entry : 例外入口処理
- core_int_entry : カーネル管理外割込み入口処理

# リファレンス

## ディレクトリ構成・ファイル構成

	risc-v_gcc/
		E_PACKAGE				簡易パッケージのファイルリスト
		MANIFEST				個別パッケージのファイルリスト

	- riscv_gcc/common/
		Makefile.core			Makefileのコア依存部
		riscv.h					コアのハードウェア資源の定義
		riscv_insn.h			コア独自の命令の実行
		core_asm.inc			アセンブラ記述のためのマクロ
		core_cfg1_out.h			cfg1_out.cのリンクに必要なスタブの定義
		core_check.trb			kernel_check.trbのコア依存部
		core_kernel.h			kernel.hのコア依存部
		core_kernel.trb			kernel.trbのコア依存部
		core_kernel_impl.c		カーネル実装のコア依存部関連の定義
		core_kernel_impl.h		カーネル実装のコア依存部
		core_offset.trb			genoffset.trbのコア依存部
		core_rename.def			コア依存部の内部識別名のリネーム定義
		core_rename.h			コア依存部の内部識別名のリネーム
		core_sil.h				sil.hのコア依存部
		core_stddef.h			t_stddef.hのコア依存部
		core_support.S			カーネル実装のコア依存部（アセンブリ言語で記述した部分）
 		core_sym.def			kernel_sym.defのコア依存部
		core_syssvc.h			システムサービスのコア依存定義
		core_test.h				テストプログラムのコア依存定義
		core_unrename.h			コア依存部の内部識別名のリネーム解除
		msi_ipi.cfg				MSI用のプロセッサ間割込みのコンフィギュレーションファイル
		msi_ipi.h				MSI用のプロセッサ間割込みに関する定義
		mtimer.c				Mタイマ用のタイマドライバ
		mtimer.h				Mタイマ用のタイマドライバ関連の定義
		plic_kernel.trb			パス2の生成スクリプトのGIC依存部
		plic_kernel_impl.c		カーネル実装のGIC依存部関連の定義
		plic_kernel_impl.h		カーネル実装のGIC依存部
		start.S					カーネル用のスタートアップモジュール（RISC-V用）

	- riscv_gcc/doc/
		- riscv_user.md			RISC-V依存部 ユーザーズマニュアル
		- riscv_design.md		RISC-Vコア依存部 設計メモ
		- riscv_memo.md			RISC-Vのアーキテクチャに関するメモ
		- plic_design.md		PLIC依存部 設計メモ
		- plic_memo.md			PLICのアーキテクチャに関するメモ
		- u54mc_memo.md			U54-MCのアーキテクチャに関するメモ

# バージョン履歴

## 2024/12/23
- common/core_sil.h
  - TOPPERS_sil_loc_spn(void) において，ロックの取得を失敗した場合，
    無条件に割り込みを許可するのではなく，TOPPERS_sil_loc_spn(void)
    呼び出し時の状態にするよう変更．

## 2024/09/09
- common/start.S
  - sbssの初期化を追加．
- common/core_support.S
  - core_exc_entry_4 において，割込み優先度マスク, 例外ネストカウント保存領域分を戻す処理の追加忘れを修正．
- common/core_test.h
  - PREPARE_RETURN_CPUEXC_IINST の定義を追加．

以上
