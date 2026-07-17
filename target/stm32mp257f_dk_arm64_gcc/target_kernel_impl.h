/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2020 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 *
 *  上記著作権者は，以下の(1)～(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 *
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 *
 */

/*
 *		カーネルのターゲット依存部に関する定義（STM32MP257F-DK用）
 *
 *  カーネルのターゲット依存部のヘッダファイル．kernel_impl.hのターゲッ
 *  ト依存部の位置付けとなる．
 */

#ifndef TOPPERS_TARGET_KERNEL_IMPL_H
#define TOPPERS_TARGET_KERNEL_IMPL_H

/*
 *  ターゲット依存部のハードウェア資源の定義
 */
#include "stm32mp257f_dk.h"

/*
 *  スレッドIDレジスタにPCBへのポインタを入れる場合
 */
#define USE_THREAD_ID_PCB

/*
 *  微少時間待ちのための定義（本来はSILのターゲット依存部）
 *
 *  test_dlynse の "for fitting parameters" 出力（DK 実機）から逆算した値．
 *  初回コスト 12ns・ループ毎 10ns（旧値 70/44 は移植元由来で実機より大きく，
 *  遅延不足で全測定 NG となっていた．ASP3 側と同一の実測値）．
 */
#define SIL_DLY_TIM1	12
#define SIL_DLY_TIM2	10

/*
 *  ipi_hanlderのバイパス処理を使用するか
 */
#define USE_BYPASS_IPI_DISPATCH_HANDER

#ifndef TOPPERS_MACRO_ONLY

/*
 *  プロセッサIDに関する関数
 *
 *  ＜arm64 共通部のリファクタ（プロセッサID取得をターゲット依存部へ移動）に対応＞
 *  チップ依存部(chip_kernel_impl.h→gic_kernel_impl.h)がこれらを使用するため，
 *  チップ依存部の include より前で定義する．以下の別ファイルと整合させること:
 *    target_asm.inc : my_prcidx()
 *    target_sil.h   : sil_get_pid()
 *  STM32MP257F-DK は Cortex-A35 ×2．MPIDR_EL1 の AFF0(bit[7:0])がコア番号(0/1)．
 */

/* プロセッサINDEX（0オリジン）の取得（C言語版） */
Inline uint32_t
get_my_prcidx(void)
{
	uint64_t mpidr;

	Asm("mrs %0, mpidr_el1":"=r"(mpidr));
	return (uint32_t)mpidr & 0x000000ff;
}

/* プロセッサIDから MPIDR(Multiprocessor Affinity Register)値への変換 */
Inline uint64_t
conv_prcid_to_mpidr(ID prcid)
{
	return (prcid - 1);
}

/* プロセッサIDから GICD のターゲットへの変換 */
Inline uint32_t
conv_prcid_to_gicdtarget(ID prcid)
{
	return (uint32_t)(1U << (prcid - 1));
}

#endif /* TOPPERS_MACRO_ONLY */

/*
 *  チップ依存部（STM32MP2用）
 */
#include "chip_kernel_impl.h"

#ifndef TOPPERS_MACRO_ONLY

/*
 *  ターゲットシステム依存の初期化（マスタのみ）
 */
extern void	target_mprc_initialize(void);

/*
 *  ターゲットシステム依存の初期化
 */
extern void	target_initialize(PCB *p_my_pcb);

/*
 *  EL3で行う初期化処理
 */
extern void target_el3_initialize(void);

/*
 *  EL2で行う初期化処理
 */
extern void target_el2_initialize(void);

/*
 *  ターゲットシステムの終了
 *
 *  システムを終了する時に使う．
 */
extern void	target_exit(void) NoReturn;

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_TARGET_KERNEL_IMPL_H */
