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
 *  @(#) $Id: target_kernel_impl.c 246 2020-07-09 06:31:08Z ertl-honda $  
 */

/*
 *  ターゲット依存モジュール（IMX8MM_EVK用）
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
static void	sio_port_init(void);

#ifdef SYSMON
#ifdef TOPPERS_TZ_S
/* ATFのTrusted OSベクタテーブル */
extern atfsmc_vectors_t atf_vector_table;
#endif /* TOPPERS_TZ_S */
#endif /* SYSMON */

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
#ifdef TOPPERS_32BIT_ABOVE_ADDR
/* Cortex-A53で利用可能な全てのメモリとレジスタをMMUに設定する場合に定義する。
 * ATFなどの同時動作するソフトウェアを利用する場合、定義しない方が良い */
//#define INITMMU_ALL_MEM

void
target_mmu_init(void)
{
	mmap_t mm;

	/*
	 *  メモリマップの設定
	 *  全領域は物理アドレス = 仮想アドレス
	 */
#ifndef INITMMU_ALL_MEM
	/*
     *  使用領域のみ設定
     */
#ifdef TOPPERS_TZ_S
	mm.ns   = MEM_NS_SECURE;
#else  /* TOPPERS_TZ_S */
	mm.ns   = MEM_NS_NONSECURE;
#endif /* TOPPERS_TZ_S */

	/* Registers */
	mm.pa   = 0x0018000000;
	mm.va   = mm.pa;
	mm.size = 0x0028000000;
	mm.attr = MEM_ATTR_SO;    /* Device-nGnRnE */
	mm.ap   = MEM_AP_RW_EL1;  /* Read/write from higher EL0 */
	mmu_mmap_add(&mm);

	/* TOPPERS/FMPが動作するメモリ */
	mm.pa   = TOPPERS_MEM_ADDR;
	mm.va   = mm.pa;
	mm.size = TOPPERS_MEM_SIZE;
	mm.attr = MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ap   = MEM_AP_RW_EL1;  /* Read/write from higher EL0 */
	mmu_mmap_add(&mm);

#else  /* INITMMU_ALL_MEM */
	/*
     *  全領域設定
     */
#ifdef TOPPERS_TZ_S
	mm.ns	= MEM_NS_SECURE;
#else  /* TOPPERS_TZ_S */
	mm.ns   = MEM_NS_NONSECURE;
#endif /* TOPPERS_TZ_S */

	/* Internal RAM */
	mm.pa   = 0x0000100000;
	mm.va   = mm.pa;
	mm.size = 0x0000A00000;
	mm.attr = MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ap   = MEM_AP_RW_EL1;  /* Read/write from higher EL0 */
	mmu_mmap_add(&mm);

	/* QSPI Flash memory */
	mm.pa   = 0x0008000000;
	mm.va   = mm.pa;
	mm.size = 0x0010000000;
	mm.attr = MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ap   = MEM_AP_RO_EL1;  /* Read only from higher EL0 */
	mmu_mmap_add(&mm);

	/* Registers */
	mm.pa   = 0x0018000000;
	mm.va   = mm.pa;
	mm.size = 0x0028000000;
	mm.attr = MEM_ATTR_SO;    /* Device-nGnRnE */
	mm.ap   = MEM_AP_RW_EL1;  /* Read/write from higher EL0 */
	mmu_mmap_add(&mm);

	/* DDR */
	mm.pa   = 0x0040000000;
	mm.va   = mm.pa;
	mm.size = 0x0080000000;
	mm.attr = MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ap   = MEM_AP_RW_EL1;  /* Read/write from higher EL0 */
	mmu_mmap_add(&mm);
#endif /* INITMMU_ALL_MEM */

	/*
	 *  ユーザープログラムによるMMUの初期化
	 */
	user_mmu_init();
}
#else	/* TOPPERS_32BIT_ABOVE_ADDR */
void
target_mmu_init(void)
{
	mmap_t mm;

	/*
	 *  メモリマップの設定
	 *  全領域は物理アドレス = 仮想アドレス
	 */
#ifdef TOPPERS_TZ_S
	/*
     *  使用領域のみ設定 (Secure OS)
     */
	/* TOPPERS/FMPが動作するメモリ */
	mm.pa   = TOPPERS_MEM_ADDR;
	mm.va   = mm.pa;
	mm.size = TOPPERS_MEM_SIZE;
	mm.attr	= MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ns	= MEM_NS_SECURE;
	mmu_mmap_add(&mm);

	/* Registers */
	mm.pa   = 0x0008000000;
	mm.va   = mm.pa;
	mm.size = 0x0038000000;
	mm.attr	= MEM_ATTR_SO;
	mm.ns	= MEM_NS_SECURE;
	mmu_mmap_add(&mm);

	mm.ns   = MEM_NS_NONSECURE;
	mmu_mmap_add(&mm);
#else  /* TOPPERS_TZ_S */
	/*
     *  使用領域のみ設定 (NonSecure OS)
     */
	/* TOPPERS/FMPが動作するメモリ */
	mm.pa   = TOPPERS_MEM_ADDR;
	mm.va   = mm.pa;
	mm.size = TOPPERS_MEM_SIZE;
	mm.attr	= MEM_ATTR_NML_C; /* Normal, Outer and Inner Write-Back No-transient */
	mm.ns   = MEM_NS_NONSECURE;
	mmu_mmap_add(&mm);

	/* Registers */
	mm.pa   = 0x0008000000;
	mm.va   = mm.pa;
	mm.size = 0x0038000000;
	mm.attr	= MEM_ATTR_SO;
	mm.ns   = MEM_NS_NONSECURE;
	mmu_mmap_add(&mm);
#endif /* TOPPERS_TZ_S */

	/*
	 *  ユーザープログラムによるMMUの初期化
	 */
	user_mmu_init();
}
#endif	/* TOPPERS_32BIT_ABOVE_ADDR */

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
		sio_port_init();
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
imx8mm_evk_uart_fput(char c)
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
		imx8mm_evk_uart_fput('\r');
	}
	imx8mm_evk_uart_fput(c);
}

/*
 *  UART用のポートの初期化
 */
static void
sio_port_init(void)
{
#ifdef USE_UART3
	/*
	 *  UART3 IOMUX設定
	 */
	/* UART3_RXD */
	/*   PAD_UART3_RXD -> GPIO5_IO26 */
	sil_wrw_mem((void*)IOMUXC_SW_MUX_CTL_PAD_UART3_RXD, IOMUXC_MUX_MODE_ALT0);
	/*   PAD ECSPI1_SCLK -> UART3_RX */
	sil_wrw_mem((void*)IOMUXC_SW_MUX_CTL_PAD_ECSPI1_SCLK, IOMUXC_MUX_MODE_ALT1);
	sil_wrw_mem((void*)IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK, 0x00000140);
    /* UART3_TXD */
	/*   PAD_UART3_TXD -> GPIO5_IO27 */
	sil_wrw_mem((void*)IOMUXC_SW_MUX_CTL_PAD_UART3_TXD, IOMUXC_MUX_MODE_ALT0);
	/*   PAD ECSPI1_MOSI -> UART3_TX */
	sil_wrw_mem((void*)IOMUXC_SW_MUX_CTL_PAD_ECSPI1_MOSI, IOMUXC_MUX_MODE_ALT1);
	sil_wrw_mem((void*)IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI, 0x00000140);
#endif /* USE_UART3 */

#ifdef USE_UART4
	/*
	 * アクセス許可
	 */ 
	sil_wrw_mem((void*)(RDC_PDAP70), 0xff);
	/*
	 *  UART4 IOMUX設定
	 */
	/* UART4_RXD */
	/*   PAD_UART4_RXD -> UART4_RX */
	sil_wrw_mem((void*)IOMUXC_SW_MUX_CTL_PAD_UART4_RXD, IOMUXC_MUX_MODE_ALT0);
	sil_wrw_mem((void*)IOMUXC_UART4_RXD_SELECT_INPUT, 2);

	/* UART4_TXD */
	/*   PAD_UART4_TXD -> UART4_RX */
	sil_wrw_mem((void*)IOMUXC_SW_MUX_CTL_PAD_UART4_TXD, IOMUXC_MUX_MODE_ALT0);
#endif /* USE_UART4 */
}
