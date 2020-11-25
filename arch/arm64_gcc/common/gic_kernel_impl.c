/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2019 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: gic_kernel_impl.c 258 2020-09-15 07:48:14Z ertl-honda $
 */

/*
 *		カーネルの割込みコントローラ依存部（GIC用）
 */

#include "kernel_impl.h"
#include "interrupt.h"
#include <sil.h>
#include "arm64.h"

/* 前方参照 */
static void gicc_init(void);
static void gic_sgi_ppi_init(void);

/*
 *  GICv2,3,4 で共通の関数
 */
/*
 *  Distributor 終了
 */
void
gicd_terminate(void)
{
	uint32_t ctlr_val;

	/* 現在値を確認 */
	ctlr_val = sil_rew_mem((void *)GICD_CTLR);

	/* ARE ビットは保持し、残りはクリア */
	ctlr_val &= GICD_CTLR_ARE_MASK;
	
	/* Distributor を無効に */
	sil_wrw_mem((void *)(GICD_CTLR), ctlr_val);
}

/*
 *  Distributor 初期化共通処理
 */
static void
gicd_initialize_common(void)
{
	int32_t i;
	uint32_t ctlr_val;

	/* 現在値を確認 */
	ctlr_val = sil_rew_mem((void *)GICD_CTLR);

	/* ARE ビットは保持し、残りはクリア */
	ctlr_val &= GICD_CTLR_ARE_MASK;

	/* Distributor を無効に */
	sil_wrw_mem((void *)(GICD_CTLR), ctlr_val);

#ifdef TOPPERS_TZ_S
	/* 割込みを全てグループ1(IRQ)に */
	for(i = TMIN_GLOBAL_INTNO/32; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_IGROUPRn + (uintptr_t)(4 * i)), 0xffffffff);
		sil_wrw_mem((void *)(GICD_IGRPMODRn + (uintptr_t)(4 * i)), 0x00000000);
	}
#endif /* TOPPERS_TZ_S */

	/* 割込みを全て禁止 */
	for(i = TMIN_GLOBAL_INTNO/32; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_ICENABLERn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* ペンディングをクリア */
	for(i = TMIN_GLOBAL_INTNO/32; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_ICPENDRn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* 優先度最低に設定  */
	for(i = TMIN_GLOBAL_INTNO/4; i < GIC_TNUM_INT/4; i++){
		sil_wrw_mem((void *)(GICD_IPRIORITYRn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* ターゲット初期化（全てCPU0へ） */
	for(i = TMIN_GLOBAL_INTNO/4; i < GIC_TNUM_INT/4; i++){
		sil_wrw_mem((void *)(GICD_ITARGETSRn + (uintptr_t)(4 * i)), 0x01010101);
	}

	/* モード初期化(1-N Level) */
	for(i = TMIN_GLOBAL_INTNO/16; i < GIC_TNUM_INT/16; i++){
		sil_wrw_mem((void *)(GICD_ICFGRn + (uintptr_t)(4 * i)), 0x55555555);
	}
}

#if TOPPERS_GIC_VER == 2
/*
 *  GIC初期化（GICv2）
 */
void
gic_init(void)
{
	/*
	 *  SGIとPPIの 初期化
	 */
	gic_sgi_ppi_init();

	/*
	 *  GIC CPUインタフェース初期化
	 */
	gicc_init();
}

/*
 *  CPU Interface の初期化（GICv2）
 */
static void
gicc_init(void)
{
	/* CPUインタフェースを無効に */
	sil_wrw_mem((void *)GICC_CTLR, 0);

	/* 最低優先度に設定 */
	gicc_set_priority(INT_IPM(TIPM_ENAALL));

	/* 割込み優先度の全ビット有効に */
	gicc_set_bp(0);

	/* ペンディングしている可能性があるので，EOI によりクリア */
	sil_wrw_mem((void *)GICC_EOIR, sil_rew_mem((void *)GICC_IAR));

	/* CPUインタフェースを有効に */
#ifdef TOPPERS_TZ_S
	sil_wrw_mem((void *)GICC_CTLR, (GICC_CTLR_FIQEN|GICC_CTLR_ENABLEGRP0));
#else  /* !TOPPERS_TZ_S */
	sil_wrw_mem((void *)GICC_CTLR, GICC_CTLR_ENABLE);
#endif /* TOPPERS_TZ_S */
}

/*
 *  CPU Interface の終了（GICv2）
 */
void
gicc_stop(void)
{
	sil_wrw_mem((void *)(GICC_CTLR), 0);
}

/*
 *  Distoributor 関連
 */

/*
 *  Distributor 初期化（GICv2）
 */
void
gicd_initialize(void)
{
	/*
	 *  共通処理を呼び出す
	 */
	gicd_initialize_common();

	/* Distibutor を有効に */
	sil_wrw_mem((void *)(GICD_CTLR), GICD_CTLR_ENABLE);
}

/*
 *  割込み禁止（GICv2）
 */
void
gicd_disable_int(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t offset_bit   = id % 32;

	sil_wrw_mem((void *)(GICD_ICENABLERn + offset_addr), (1 << offset_bit));
}

/*
 *  割込み許可（GICv2）
 */
void
gicd_enable_int(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t offset_bit  = id % 32;

	sil_wrw_mem((void *)(GICD_ISENABLERn + offset_addr), (1 << offset_bit));
}

/*
 *  割込みペンディングクリア（GICv2）
 */
void
gicd_clear_pending(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t offset_bit  = id % 32;

	sil_wrw_mem((void *)(GICD_ICPENDRn + offset_addr), (1 << offset_bit));
}

/*
 *  割込みペンディングセット（GICv2）
 */
void
gicd_set_pending(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t offset_bit  = id % 32;

	sil_wrw_mem((void *)(GICD_ISPENDRn + offset_addr), (1 << offset_bit));
}

/*
 *  割込み要求のチェック（GICv2）
 */
bool_t
gicd_probe_pending(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t offset_bit  = id % 32;
	uint32_t state;

	state = sil_rew_mem((void *)(GICD_ISPENDRn + offset_addr));

	return ((state & (1 << offset_bit)) == (1 << offset_bit));
}

/*
 *  割込みコンフィギュレーション設定（GICv2）
 */
void
gicd_config(uint8_t id,  bool_t is_edge, bool_t is_1_n)
{
	uintptr_t offset_addr;
	uint16_t offset_bit;
	uint32_t cfgr_reg_val;
	uint8_t  config;
	SIL_PRE_LOC;

	if (is_edge) {
		config = GICD_ICFGRn_EDGE;
	}
	else {
		config = GICD_ICFGRn_LEVEL;
	}

	if (is_1_n) {
		config |= GICD_ICFGRn_1_N;
	}
	else {
		config |= GICD_ICFGRn_N_N;
	}

	offset_addr = (id / 16) * 4;
	offset_bit  = (id % 16) * 2;

	SIL_LOC_SPN();

	cfgr_reg_val  = sil_rew_mem((void *)(GICD_ICFGRn + offset_addr));
	cfgr_reg_val &= ~(0x03U << offset_bit);
	cfgr_reg_val |= (0x03U & config) << offset_bit;
	sil_wrw_mem((void *)(GICD_ICFGRn + offset_addr), cfgr_reg_val);

#ifdef TOPPERS_TZ_S
	offset_addr = (id / 32) * 4;
	offset_bit  = id % 32;
	sil_wrw_mem((void *)(GICD_IGROUPRn + offset_addr),
				sil_rew_mem((void *)(GICD_IGROUPRn+ offset_addr)) & ~(1 << offset_bit));
#endif /* TOPPERS_TZ_S */

	SIL_UNL_SPN();
}

/*
 *  割込み優先度のセット（GICv2）
 *  内部表現で渡す．
 */
void
gicd_set_priority(INTNO intno, uint_t pri)
{
	uintptr_t offset_addr = (intno / 4) * 4;
	uint16_t shift  = ((intno % 4) * 8);
	uint32_t pr_reg_val;
	SIL_PRE_LOC;

	SIL_LOC_SPN();
	pr_reg_val  = sil_rew_mem((void *)(GICD_IPRIORITYRn + offset_addr));
	pr_reg_val &= ~(0xffU << shift);
	pr_reg_val |= (pri << shift);
	sil_wrw_mem((void *)(GICD_IPRIORITYRn + offset_addr), pr_reg_val);
	SIL_UNL_SPN();
}

/*
 *  SGIとPPIの 初期化（GICv2）
 */
static void
gic_sgi_ppi_init(void)
{
	int32_t i;

	/* 割込みを全てグループ1(IRQ)に */
	sil_wrw_mem((void *)(GICD_IGROUPRn + (uintptr_t)(4 * 0)), 0xffffffff);

	/* 割込みを全て禁止 */
	sil_wrw_mem((void *)(GICD_ICENABLERn + (uintptr_t)(4 * 0)), 0xffffffff);

	/* ペンディングをクリア */
	sil_wrw_mem((void *)(GICD_ICPENDRn + (uintptr_t)(4 * 0)), 0xffff0000);

	/* 優先度最低に設定  */
	for(i = 0; i < TMIN_GLOBAL_INTNO/4; i++){
		sil_wrw_mem((void *)(GICD_IPRIORITYRn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* モード初期化(1-N Level) */
	sil_wrw_mem((void *)(GICD_ICFGRn + (uintptr_t)(4 * 1)), 0x55555555);
}

/*
 *  GIC割込みターゲットの設定（GICv2）
 *  cpusはターゲットとするCPUのビットパターンで指定
 */
void
gicd_set_target(uint8_t id, ID iprcid, uint8_t cpus)
{
	uintptr_t	offset_addr = (id / 4) * 4;
	uint32_t	shift  = (id % 4) * 8;
	uint32_t itr_reg_val;
	SIL_PRE_LOC;

	SIL_LOC_SPN();
	itr_reg_val  = sil_rew_mem((void *)(GICD_ITARGETSRn + offset_addr));
	itr_reg_val &= ~(0xf << shift);
	itr_reg_val |= (cpus << shift);
	sil_wrw_mem((void *)(GICD_ITARGETSRn + offset_addr), itr_reg_val);
	SIL_UNL_SPN();
}
#elif (TOPPERS_GIC_VER == 3) || (TOPPERS_GIC_VER == 4)

/*
 *  GICv3,4向けの関数
 */

uintptr_t	gicr_base[TNUM_PRCID];
static bool_t	gicr_init(void);

/*
 *  GIC初期化（GICv3,4）
 */
void
gic_init(void)
{
	/*
	 *  GIC Redistributor初期化
	 */
	if (!gicr_init()) {
		target_exit();
	}

	/*
	 *  SGIとPPIの 初期化
	 */
	gic_sgi_ppi_init();

	/*
	 *  GIC CPUインタフェース初期化
	 */
	gicc_init();
}

/*
 *  CPU Interface の初期化（GICv3,4）
 */
static void
gicc_init(void)
{
	volatile uint32_t   reg32_val;

	/* Enable System Register */
	ICC_SRE_EL1_WRITE(1);

#ifdef TOPPERS_TZ_S
	/* Group0,1割込を無効に */
	ICC_IGRPEN0_EL1_WRITE(0);
	ICC_IGRPEN1_EL1_WRITE(0);

	/* 最低優先度に設定 */
	gicc_set_priority(INT_IPM(TIPM_ENAALL));

	/* 割込み優先度の全ビット有効に */
	gicc_set_bp(0);

	/* ペンディングしている可能性があるので，EOI によりクリア */
	ICC_IAR0_EL1_READ(reg32_val);
	ICC_EOIR0_EL1_WRITE(reg32_val);

	/* Group0,1割込を有効に */
	ICC_IGRPEN0_EL1_WRITE(1);
	ICC_IGRPEN1_EL1_WRITE(1);

	/* Disable IRQ,FIQ bypass */
	ICC_SRE_EL1_WRITE(3);
#else  /* !TOPPERS_TZ_S */
	/* Group1割込を無効に */
	ICC_IGRPEN1_EL1_WRITE(0);

	/* 最低優先度に設定 */
	gicc_set_priority(INT_IPM(TIPM_ENAALL));

	/* 割込み優先度の全ビット有効に */
	gicc_set_bp(0);

	/* ペンディングしている可能性があるので，EOI によりクリア */
	ICC_IAR1_EL1_READ(reg32_val);
	ICC_EOIR1_EL1_WRITE(reg32_val);

	/* Group1割込を有効に */
	ICC_IGRPEN1_EL1_WRITE(1);

	/* Disable IRQ,FIQ bypass */
	ICC_SRE_EL1_WRITE(7);
#endif /* TOPPERS_TZ_S */
}

/*
 *  CPU Interface の終了（GICv3,4）
 */
void
gicc_stop(void)
{
#ifdef TOPPERS_TZ_S
	ICC_IGRPEN0_EL1_WRITE(0);
	ICC_IGRPEN1_EL1_WRITE(0);
#else  /* !TOPPERS_TZ_S */
	ICC_IGRPEN1_EL1_WRITE(0);
#endif /* TOPPERS_TZ_S */
}

/*
 *  Redistoributor初期化（GICv3,4）
 */
static bool_t
gicr_init(void)
{
	volatile uint32_t   reg32_val;
	uint_t  cnt;
	uint_t	prc_id = get_my_prcidx();

	gicr_base[prc_id] = GICR_BASE + (GICR_SIZE * prc_id);

	/* Redistoributor起動 */
	reg32_val = *((volatile uint32_t *)GICR_WAKER(prc_id));
	reg32_val &= ~GICR_ProcSleep;
	*((volatile uint32_t *)GICR_WAKER(prc_id)) = reg32_val;

	/* 起動を待つ */
	for( cnt = 0 ; cnt < 1000 ; cnt++ ) {
		reg32_val = *((volatile uint32_t *)GICR_WAKER(prc_id));
		if ((reg32_val & GICR_ChildrenAsleep) == 0) {
			return true;
		}
		sil_dly_nse(1000000);
	}

	return false;
}

/*
 *  Distoributor 関連
 */

/*
 *  Distributor 初期化（GICv3,4）
 */
void
gicd_initialize(void) {
	int32_t i;

	/* 共通処理を呼び出す */
	gicd_initialize_common();

	/* SPIの設定値をクリア */
#ifdef TOPPERS_TZ_S
	/* 全てNonSecure Group1に */
	for(i = 1; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_IGROUPRn + (uintptr_t)(4 * i)),  0xffffffff);
		sil_wrw_mem((void *)(GICD_IGRPMODRn + (uintptr_t)(4 * i)), 0x00000000);
	}
#endif /* TOPPERS_TZ_S */

	/* 割込みを全て禁止 */
	for(i = 1; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_ICENABLERn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* ペンディングをクリア */
	for(i = 1; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_ICPENDRn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* 優先度最低に設定  */
	for(i = 1; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_IPRIORITYRn + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* ターゲット初期化（全てCPU0へ） */
	/*   SPIs */
	for(i = 1; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_ITARGETSRn + (uintptr_t)(4 * i)), 0x01010101);
	}

	/* モード初期化(1-N Level) */
	for(i = 1; i < GIC_TNUM_INT/32; i++){
		sil_wrw_mem((void *)(GICD_ICFGRn + (uintptr_t)(4 * i)), 0x55555555);
	}

	/* Distibutor を有効に */
#ifdef TOPPERS_TZ_S
	sil_wrw_mem((void *)(GICD_CTLR), sil_rew_mem((void *)(GICD_CTLR)) | GICD_CTLR_ARE_S);
	sil_wrw_mem((void *)(GICD_CTLR), sil_rew_mem((void *)(GICD_CTLR)) | GICD_CTLR_ENABLEGRP1S);
#else  /* !TOPPERS_TZ_S */
	sil_wrw_mem((void *)(GICD_CTLR), sil_rew_mem((void *)(GICD_CTLR)) | GICD_CTLR_ARE_NS);
	sil_wrw_mem((void *)(GICD_CTLR), sil_rew_mem((void *)(GICD_CTLR)) | GICD_CTLR_ENABLEGRP1NS);
#endif /* TOPPERS_TZ_S */
}

/*
 *  割込み禁止（GICv3,4）
 */
void
gicd_disable_int(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t shift   = id % 32;
	uint_t	prc_id = get_my_prcidx();

	if (id < 32) {
		sil_wrw_mem((void *)GICR_ICENABLER0(prc_id), (1 << shift));
	}
	else {
		sil_wrw_mem((void *)(GICD_ICENABLERn + offset_addr), (1 << shift));
	}
}

/*
 *  割込み許可（GICv3,4）
 */
void
gicd_enable_int(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t shift  = id % 32;
	uint_t	prc_id = get_my_prcidx();

	if (id < 32) {
		sil_wrw_mem((void *)GICR_ISENABLER0(prc_id), (1 << shift));
	}
	else {
		sil_wrw_mem((void *)(GICD_ISENABLERn + offset_addr), (1 << shift));
	}
}

/*
 *  割込みペンディングクリア（GICv3,4）
 */
void
gicd_clear_pending(uint8_t id)
{
	uint16_t shift  = id % 32;
	uint_t	prc_id = get_my_prcidx();

	if (id < 32) {
		sil_wrw_mem((void *)GICR_ICPENDR0(prc_id), (1 << shift));
	}
	else {
#ifdef TOPPERS_TZ_S
		sil_wrw_mem((void *)GICD_CLRSPI_SR, id);
#else  /* !TOPPERS_TZ_S */
		sil_wrw_mem((void *)GICD_CLRSPI_NSR, id);
#endif /* TOPPERS_TZ_S */
	}
}

/*
 *  割込みペンディングセット（GICv3,4）
 */
void
gicd_set_pending(uint8_t id)
{
	uint16_t	shift  = id % 32;
	uint_t		prc_id = get_my_prcidx();

	if (id < 32) {        
		sil_wrw_mem((void *)GICR_ISPENDR0(prc_id), (1 << shift));
	}
    else {
#ifdef TOPPERS_TZ_S
		sil_wrw_mem((void *)GICD_SETSPI_SR, id);
#else  /* !TOPPERS_TZ_S */
		sil_wrw_mem((void *)GICD_SETSPI_NSR, id);
#endif /* TOPPERS_TZ_S */
	}
}

/*
 *  割込み要求のチェック（GICv3,4）
 */
bool_t
gicd_probe_pending(uint8_t id)
{
	uintptr_t offset_addr = (id / 32) * 4;
	uint16_t shift  = id % 32;
	uint32_t state;
	uint_t	prc_id = get_my_prcidx();

	if (id < 32) {
		state = sil_rew_mem((void *)GICR_ISPENDR0(prc_id));
	}
	else {
		state = sil_rew_mem((void *)(GICD_ISPENDRn + offset_addr));
	}

	return ((state & (1 << shift)) == (1 << shift));
}

/*
 *  割込みコンフィギュレーション設定（GICv3,4）
 */
void
gicd_config(uint8_t id,  bool_t is_edge, bool_t is_1_n)
{
	uintptr_t offset_addr;
	uint16_t shift;
	uint32_t cfgr_reg_val;
	uint8_t  config;
	uint_t	prc_id = get_my_prcidx();
	SIL_PRE_LOC;

	if (is_edge) {
		config = GICD_ICFGRn_EDGE;
	}
	else {
		config = GICD_ICFGRn_LEVEL;
	}

	if (is_1_n) {
		config |= GICD_ICFGRn_1_N;
	}
	else {
		config |= GICD_ICFGRn_N_N;
	}

	offset_addr = (id / 16) * 4;
	shift  = (id % 16) * 2;

	SIL_LOC_SPN();

	if (id < 32) {
		cfgr_reg_val  = sil_rew_mem((void *)(GICR_ICFGR0(prc_id) + offset_addr));
		cfgr_reg_val &= ~(0x03U << shift);
		cfgr_reg_val |= (0x03U & config) << shift;
		sil_wrw_mem((void *)(GICR_ICFGR0(prc_id) + offset_addr), cfgr_reg_val);
#ifdef TOPPERS_TZ_S
		/* 割込をSecure Group1に設定 */
		shift  = id % 32;
		sil_wrw_mem((void *)GICR_IGROUPR0(prc_id),
					sil_rew_mem((void *)GICR_IGROUPR0(prc_id)) & ~(1U << shift));
		sil_wrw_mem((void *)GICR_IGRPMODR0(prc_id),
					sil_rew_mem((void *)GICR_IGRPMODR0(prc_id)) | (1U << shift));
#endif /* TOPPERS_TZ_S */
	}
	else {
		cfgr_reg_val  = sil_rew_mem((void *)(GICD_ICFGRn + offset_addr));
		cfgr_reg_val &= ~(0x03U << shift);
		cfgr_reg_val |= (0x03U & config) << shift;
		sil_wrw_mem((void *)(GICD_ICFGRn + offset_addr), cfgr_reg_val);
#ifdef TOPPERS_TZ_S
		/* 割込をSecure Group1に設定 */
		offset_addr = (id / 32) * 4;
		shift  = id % 32;
		sil_wrw_mem((void *)(GICD_IGROUPRn + offset_addr),
					sil_rew_mem((void *)(GICD_IGROUPRn + offset_addr)) & ~(1U << shift));
		sil_wrw_mem((void *)(GICD_IGRPMODRn + offset_addr),
					sil_rew_mem((void *)(GICD_IGRPMODRn + offset_addr)) | (1U << shift));
#endif /* TOPPERS_TZ_S */
	}

	SIL_UNL_SPN();
}

/*
 *  割込み優先度のセット（GICv3,4）
 *  内部表現で渡す．
 */
void
gicd_set_priority(INTNO intno, uint_t pri)
{
	uintptr_t offset_addr = (intno / 4) * 4;
	uint16_t shift  = ((intno % 4) * 8);
	uint32_t pr_reg_val;
	uint_t	prc_id = get_my_prcidx();
	SIL_PRE_LOC;

	SIL_LOC_SPN();
	if (intno < GIC_INTNO_SPI0) {
		pr_reg_val  = sil_rew_mem((void *)(GICR_IPRIORITYRn(prc_id) + offset_addr));
		pr_reg_val &= ~(0xffU << shift);
		pr_reg_val |= (pri << shift);
		sil_wrw_mem((void *)(GICR_IPRIORITYRn(prc_id) + offset_addr), pr_reg_val);
	}
	else {
		pr_reg_val  = sil_rew_mem((void *)(GICD_IPRIORITYRn + offset_addr));
		pr_reg_val &= ~(0xffU << shift);
		pr_reg_val |= (pri << shift);
		sil_wrw_mem((void *)(GICD_IPRIORITYRn + offset_addr), pr_reg_val);
	}
	SIL_UNL_SPN();
}

/*
 *  SGIとPPIの 初期化（GICv3,4）
 */
static void
gic_sgi_ppi_init(void)
{
	int_t i;
	uint_t	prc_id = get_my_prcidx();

#ifdef TOPPERS_TZ_S
	/* SGIとPPIを全て Non-Secure Group 1 に */
	sil_wrw_mem((void *)GICR_IGROUPR0(prc_id), 0xffffffff);
	sil_wrw_mem((void *)GICR_IGRPMODR0(prc_id), 0x00000000);
#endif /* TOPPERS_TZ_S */

	/* 割込みを全て禁止 */
	sil_wrw_mem((void *)GICR_ICENABLER0(prc_id), 0xffffffff);

	/* ペンディングをクリア */
	sil_wrw_mem((void *)GICR_ICPENDR0(prc_id), 0xffff0000);

	/* 優先度最低に設定  */
	for(i = 0; i < TMIN_GLOBAL_INTNO/4; i++){
		sil_wrw_mem((void *)(GICR_IPRIORITYRn(prc_id) + (uintptr_t)(4 * i)), 0xffffffff);
	}

	/* モード初期化(1-N Level) */
	/*   SGIs */
	sil_wrw_mem((void *)GICR_ICFGR0(prc_id), 0x55555555);
	/*   PPIs */
	sil_wrw_mem((void *)GICR_ICFGR1(prc_id), 0x55555555);
}

/*
 *  GIC割込みターゲットの設定（GICv3,4）
 *  cpusはターゲットとするCPUのビットパターンで指定
 */
void
gicd_set_target(uint8_t id, ID iprcid, uint8_t cpus)
{
	uintptr_t offset_addr = id * 8;
	uint64_t itr_reg_val;
	SIL_PRE_LOC;

	SIL_LOC_SPN();
	itr_reg_val  = sil_rew_mem((void *)(GICD_IROUTERn + offset_addr));
	itr_reg_val |= iprcid - 1;
	sil_wrw_mem((void *)(GICD_IROUTERn + offset_addr), itr_reg_val);
	SIL_UNL_SPN();
}

/*
 *  割込みグループの設定（セキュリティ拡張）
 */
Inline void
gicd_config_group(INTNO intno, uint_t group)
{
	uint_t		shift = intno % 32;
	uint32_t	reg;

	reg = sil_rew_mem((void *)(GICD_IGROUPRn + (uintptr_t)((intno / 32) * 4)));
	reg &= ~(0x01U << shift);
	reg |= (group << shift);
	sil_wrw_mem((void *)(GICD_IGROUPRn + (uintptr_t)((intno / 32) * 4)), reg);
}

/*
 *  割込みグループモディファイアの設定（セキュリティ拡張）
 */
Inline void
gicd_config_group_modifier(INTNO intno, uint_t group)
{
	uint_t		shift = intno % 32;
	uint32_t	reg;

	reg = sil_rew_mem((void *)(GICD_IGRPMODRn + (uintptr_t)((intno / 32) * 4)));
	reg &= ~(0x01U << shift);
	reg |= (group << shift);
	sil_wrw_mem((void *)(GICD_IGRPMODRn + (uintptr_t)((intno / 32) * 4)), reg);
}

#endif /* (TOPPERS_GIC_VER == 3) || (TOPPERS_GIC_VER == 4) */

#ifndef OMIT_GIC_INITIALIZE_INTERRUPT

/*
 *  割込み要求ラインの属性の設定
 *
 *  FMP3カーネルでの利用を想定して，パラメータエラーはアサーションでチェッ
 *  クしている．
 */
Inline void
config_int(PCB *p_my_pcb, INTNO intno, ATR intatr, PRI intpri, ID iprcid, uint_t affinity)
{
	assert(VALID_INTNO(p_my_pcb->prcid, intno));
	assert(TMIN_INTPRI <= intpri && intpri <= TMAX_INTPRI);

	/*
	 *  割込みを禁止
	 *
	 *  割込みを受け付けたまま，レベルトリガ／エッジトリガの設定や，割
	 *  込み優先度の設定を行うのは危険なため，割込み属性にかかわらず，
	 *  一旦マスクする．
	 */
	disable_int(intno);

	/*
	 *  割込みをコンフィギュレーション
	 */
	if ((intatr & TA_EDGE) != 0U) {
		gicd_config(INTNO_MASK(intno), GICD_ICFGRn_EDGE, true);
		clear_int(intno);
	}
	else {
		gicd_config(INTNO_MASK(intno), GICD_ICFGRn_LEVEL, true);
	}

	/*
	 *  割込み優先度とターゲットプロセッサを設定
	 */
	gicd_set_priority(INTNO_MASK(intno), INT_IPM(intpri));
	if (INTNO_MASK(intno) >= TMIN_GLOBAL_INTNO) {
		gicd_set_target(INTNO_MASK(intno), iprcid, affinity);
	}

	/*
	 * 割込みを許可
	 */
	if ((intatr & TA_ENAINT) != 0U) {
		enable_int(intno);
	}
}

/*
 *  割込み管理機能の初期化
 */
void
initialize_interrupt(PCB *p_my_pcb)
{
	uint_t			i;
	const INTINIB	*p_intinib;

	for (i = 0; i < tnum_cfg_intno; i++) {
		p_intinib = &(intinib_table[i]);
		if (p_intinib->iprcid == p_my_pcb->prcid) {
			config_int(p_my_pcb, p_intinib->intno, p_intinib->intatr,
							p_intinib->intpri, p_intinib->iprcid, p_intinib->affinity);
		}
	}
}

#endif /* OMIT_GIC_INITIALIZE_INTERRUPT */
