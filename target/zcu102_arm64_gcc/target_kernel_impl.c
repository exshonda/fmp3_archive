/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2007-2020 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: target_kernel_impl.c 216 2020-02-17 16:17:15Z ertl-honda $  
 */

/*
 *  ターゲット依存モジュール（ZCU102 Cortex-A53(AArch64)用）
 */
#include "kernel_impl.h"
#include <sil.h>

#ifdef TOPPERS_WITH_ATF
#include "atf.h"
#include "psci.h"
#endif /* TOPPERS_WITH_ATF */

/*
 *  システムログの低レベル出力のための初期化
 */
extern void	sio_initialize(intptr_t exinf);
extern void	target_fput_initialize(void);

/*
 *  EL3で行う初期化処理
 */
void
target_el3_initialize(void)
{
	chip_el3_initialize();
}

/*
 *  EL2で行う初期化処理
 */
void
target_el2_initialize(void)
{
	chip_el2_initialize();
}

/*
 *  str_ker() の前でマスタプロセッサで行う初期化
 */
void
target_mprc_initialize(void)
{
	chip_mprc_initialize();
}

/*
 *  ユーザープログラムによるMMUの初期化
 */
void user_mmu_init(void) __attribute__((weak));

/*
 *  ユーザーが定義しなかった場合のweak関数
 */
void
user_mmu_init(void) {
}

/*
 *  メモリマップの初期化（core依存部から呼び出される）
 *  同時にキャッシュも有効にする
 */
void
target_mmu_init(void)
{
	mmap_t mm;

	/*
	 *  メモリマップの設定
	 *  全領域は物理アドレス = 仮想アドレス
	 */
#ifdef TOPPERS_TZ_S
	mm.pa   = TOPPERS_MEM_BASE;
	mm.va   = mm.pa;
	mm.size = TOPPERS_MEM_SIZE;
	mm.attr	= MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ns	= MEM_NS_SECURE;
	mmu_mmap_add(&mm);
	
	mm.pa   = 0x80000000;
	mm.va   = mm.pa;
	mm.size = 0x80000000;
	mm.attr	= MEM_ATTR_DEV;
	mm.ns	= MEM_NS_SECURE;
	mmu_mmap_add(&mm);
#endif /* defined(TOPPERS_TZ_S) */

	/*
	 *  ユーザープログラムによるMMUの初期化
	 */
	user_mmu_init();
}

/*
 *  ターゲット依存の初期化
 */
void
target_initialize(PCB *p_my_pcb)
{
	/*
	 * チップ依存の初期化
	 */
	chip_initialize(p_my_pcb);

	/*
	 *  バナー表示，低レベル出力用にUARTを初期化
	 */
	if (is_mprc(p_my_pcb)) {
		sio_initialize(0);
		target_fput_initialize();
	}
}

/*
 *  ターゲット依存の終了処理
 */
void
target_exit(void)
{
	extern void	software_term_hook(void);
	void (*volatile fp)(void) = software_term_hook;

	/*
	 *  software_term_hookへのポインタを，一旦volatile指定のあるfpに代
	 *  入してから使うのは，0との比較が最適化で削除されないようにするた
	 *  めである．
	 */
	if (fp != 0) {
		(*fp)();
	}

	/*
	 *  チップ依存の終了処理
	 */
	chip_terminate();

	/*
	 *  ターゲット依存の終了処理
	 */
#if defined(TOPPERS_USE_QEMU) && !defined(TOPPERS_OMIT_QEMU_SEMIHOSTING)
	/*
	 *  QEMUを終了させる．
	 */
	if (ID_PRC(get_my_prcidx()) == TOPPERS_TMASTER_PRCID) {
		Asm("ldr x1, =0x20026\n\t"		/* ADP_Stopped_ApplicationExit */ 
			"mov x0, #0x18\n\t"			/* angel_SWIreason_ReportException */
			"hlt #0xF000");
	}
#endif /* defined(TOPPERS_USE_QEMU) && !defined(TOPPERS_OMIT_QEMU_SEMIHOSTING) */
	while (true) ;
}

/*
 *		システムログの低レベル出力（本来は別のファイルにすべき）
 */
#include "target_syssvc.h"
#include "target_serial.h"

/*
 *  低レベル出力用のSIOポート管理ブロック
 */
static SIOPCB	*p_siopcb_target_fput;

/*
 *  SIOポートの初期化
 */
void
target_fput_initialize(void)
{
	p_siopcb_target_fput = sio_opn_por(SIOPID_FPUT, 0);
}

/*
 *  SIOポートへのポーリング出力
 */
Inline void
zcu102_uart_fput(char c)
{
	/*
	 *  送信できるまでポーリング
	 */
	while (!(sio_snd_chr(p_siopcb_target_fput, c))) {
		sil_dly_nse(100);
	}
}

/*
 *  SIOポートへの文字出力
 */
void
target_fput_log(char c)
{
	if (c == '\n') {
		zcu102_uart_fput('\r');
	}
	zcu102_uart_fput(c);
}
