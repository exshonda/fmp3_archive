/*
 *  TOPPERS/FMP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Flexible MultiProcessor Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2005-2020 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: core_timer.h 252 2020-07-14 06:36:21Z ertl-honda $
 */

/*
 *  タイマドライバ（ARM64用）
 */

#ifndef TOPPERS_CORE_TIMER_H
#define TOPPERS_CORE_TIMER_H

#include "kernel/kernel_impl.h"
#include <sil.h>

/*
 *  高分解能タイマ割込みハンドラ登録のための定数
 */
#ifdef TOPPERS_TZ_S
#define INHNO_TIMER			GIC_IRQNO_SPHYTIMER
#define INTNO_TIMER			GIC_IRQNO_SPHYTIMER
#else /* TOPPERS_TZ_S */
#define INHNO_TIMER			GIC_IRQNO_NSPHYTIMER
#define INTNO_TIMER			GIC_IRQNO_NSPHYTIMER
#endif /* TOPPERS_TZ_S */

#define INHNO_TIMER_PRC1	(0x00010000 | INHNO_TIMER)	/* ハンドラ番号 */
#define INHNO_TIMER_PRC2	(0x00020000 | INHNO_TIMER)	/* ハンドラ番号 */
#define INHNO_TIMER_PRC3	(0x00030000 | INHNO_TIMER)	/* ハンドラ番号 */
#define INHNO_TIMER_PRC4	(0x00040000 | INHNO_TIMER)	/* ハンドラ番号 */
#define INTNO_TIMER_PRC1	(0x00010000 | INTNO_TIMER)	/* 割込み番号 */
#define INTNO_TIMER_PRC2	(0x00020000 | INTNO_TIMER)	/* 割込み番号 */
#define INTNO_TIMER_PRC3	(0x00030000 | INTNO_TIMER)	/* 割込み番号 */
#define INTNO_TIMER_PRC4	(0x00040000 | INTNO_TIMER)	/* 割込み番号 */

#ifndef INTPRI_TIMER
#define INTPRI_TIMER		(TMAX_INTPRI - 1)				/* 割込み優先度 */
#endif /* INTPRI_TIMER */

#ifndef INTATR_TIMER
#define INTATR_TIMER		TA_NULL							/* 割込み属性 */
#endif /* INTATR_TIMER */

#ifndef TOPPERS_MACRO_ONLY

/*
 *  タイマ値の内部表現の型
 */
typedef uint64_t CLOCK;

/*
 *  タイマのクロックを保持する変数
 *  単位はMHz
 *  target_initialize() で初期化される
 */
extern uint32_t timer_clock;

/*
 *  タイマの設定値
 */
extern CLOCK timer_cval[];

/*
 *  タイマの起動処理
 *
 *  タイマを初期化し，周期的なタイマ割込み要求を発生させる．
 */
extern void target_hrt_initialize(intptr_t exinf);

/*
 *  タイマの停止処理
 *
 *  タイマの動作を停止させる．
 */
extern void target_hrt_terminate(intptr_t exinf);

#ifdef _RUN_TTSP3_
/*------------------------------------------------------------------*/
/*  TTSP3ビルド時 _RUN_TTSP3_ を定義して全体をビルドする必要がある  */
/*  ttsp3/library/FMP/target/imx8mm_evk_arm64_gcc/ttsp_target.sh の */
/*  CONFIG_OPT で定義している                                       */
/*------------------------------------------------------------------*/
extern volatile bool_t stop_tick;
extern volatile HRTCNT temp_gtc;
extern volatile HRTCNT stop_time;
extern volatile bool_t tick_int;
extern void ttsp_int_raise(INTNO intno);
#endif  /*  _RUN_TTSP3_ */

/*
 *  ジェネリックタイマの現在値の読出し
 */
Inline CLOCK
target_timer_get_count(void)
{
	CLOCK cnt;
#ifdef _RUN_TTSP3_
	/*  ttsp3実行用 */
	if(stop_tick == true)
	  /*  ソフトカウンタを返す    */
	  return (CLOCK)stop_time;
#ifdef TOPPERS_TZ_S
	CNTPCT_EL0_READ(cnt);
#else /* !TOPPERS_TZ_S */
	CNTPCT_EL0_READ(cnt);
#endif /* TOPPERS_TZ_S */
	return cnt;
#else /*  _RUN_TTSP3_ */
    /*  現在のカウンタ値を返す  */
#ifdef TOPPERS_TZ_S
	CNTPCT_EL0_READ(cnt);
#else /* !TOPPERS_TZ_S */
	CNTPCT_EL0_READ(cnt);
#endif /* TOPPERS_TZ_S */
	return cnt;
#endif /*  _RUN_TTSP3_ */
}

/*
 *  タイマのコンパレータ値の設定
 */
Inline void
target_timer_set_cval(uint64_t cvr)
{
	/* 割込みをディスエーブル */
#ifdef TOPPERS_TZ_S
	CNTPS_CTL_EL1_WRITE((uint32_t)CNTPS_CTL_IMASK_BIT);
#else /* !TOPPERS_TZ_S */
	CNTP_CTL_EL0_WRITE((uint32_t)CNTP_CTL_IMASK_BIT);
#endif /* TOPPERS_TZ_S */

	/* コンパレータ値を設定 */
#ifdef TOPPERS_TZ_S
	CNTPS_CVAL_EL1_WRITE(cvr);
#else /* !TOPPERS_TZ_S */
	CNTP_CVAL_EL0_WRITE(cvr);
#endif /* TOPPERS_TZ_S */

#ifdef _RUN_TTSP3_
	/*  ttsp3実行用     */
	/*  割り込みが完了している場合のみ  */
	if(stop_tick == false)
#endif /*  _RUN_TTSP3_ */

	/* 割込みをイネーブル */
#ifdef TOPPERS_TZ_S
	CNTPS_CTL_EL1_WRITE((uint32_t)CNTPS_CTL_ENABLE_BIT);
#else /* !TOPPERS_TZ_S */
	CNTP_CTL_EL0_WRITE((uint32_t)CNTP_CTL_ENABLE_BIT);
#endif /* TOPPERS_TZ_S */
}

/*
 *  タイマ割込み要求のチェック
 */
Inline bool_t
target_timer_probe_int(void)
{
#if 0
	uint32_t ctl;

#if defined(TOPPERS_TZ_S)
	CNTPS_CTL_EL1_READ(ctl);
#else
	CNTP_CTL_EL0_READ(ctl);
#endif

	return ((ctl & CNTP_CTL_ISTATUS_BIT) == CNTP_CTL_ISTATUS_BIT);
#else
	/*
	 * Generic Timerはタイムアウト後はCNTP_CVALを更新するまで，CNTP_TVALの値は0のまま
	 * であるため， target_timer_probe_int()がtrueを返すとget_utm()に不整合が発生するため，
	 * 常にfalseを返すこととする． 
	 */
	return false;
#endif /* 0 */
}

/*
 *  高分解能タイマ
 */
/*
 *  高分解能タイマ割込みハンドラ
 */
extern void	target_hrt_handler(void);


/*
 *  高分解能タイマの起動処理
 */
extern void	target_hrt_initialize(intptr_t exinf);

/*
 *  高分解能タイマの停止処理
 */
extern void	target_hrt_terminate(intptr_t exinf);

/*
 *  高分解能タイマの現在のカウント値の読出し
 */
Inline HRTCNT
target_hrt_get_current(void)
{
	/*
	 *  タイマのカウント値を読み出し，
	 *  timer_clockで除した値を返す．
	 */
	return((HRTCNT)(target_timer_get_count() / timer_clock));

}

/*
 *  高分解能タイマへの割込みタイミングの設定
 *
 *  高分解能タイマを，hrtcntで指定した値カウントアップしたら割込みを発
 *  生させるように設定する．
 */
Inline void
target_hrt_set_event(ID prcid, HRTCNT hrtcnt)
{
#ifdef  _RUN_TTSP3_
	/*  ttsp3実行用     */
	tick_int = false;
#endif /*  _RUN_TTSP3_ */
	/*
	 *  コンパレータ値を，(現在のカウント値＋hrtcnt×timer_clock)
	 *  に設定し，コンパレータと割込みをイネーブルする．
	 */
	target_timer_set_cval(target_timer_get_count()
						  + (((uint64_t) hrtcnt) * timer_clock));
}

/*
 *  高分解能タイマへの割込みタイミングのクリア
 */
Inline void
target_hrt_clear_event(ID prcid)
{
	uint32_t ctl;

#ifdef TOPPERS_TZ_S
	CNTPS_CTL_EL1_READ(ctl);
	CNTPS_CTL_EL1_WRITE(ctl & ~((uint32_t)CNTPS_CTL_ENABLE_BIT));
#else /* !TOPPERS_TZ_S */
	CNTP_CTL_EL0_READ(ctl);
	CNTP_CTL_EL0_WRITE(ctl & ~((uint32_t)CNTP_CTL_ENABLE_BIT));
#endif /* TOPPERS_TZ_S */
}

/*
 *  高分解能タイマ割込みの要求
 */
Inline void
target_hrt_raise_event(ID prcid)
{
#ifdef  _RUN_TTSP3_
	/*  ttsp3実行用     */
	ena_int(INTNO_TIMER_PRC1);
	stop_time = temp_gtc;
	ttsp_int_raise(INTNO_TIMER);
#else   /*  _RUN_TTSP3_ */
	target_hrt_set_event(prcid, 1U);
#endif  /*  _RUN_TTSP3_ */
}

/*
 *  割込みタイミングに指定する最大値
 */
#if !defined(TCYC_HRTCNT) || (TCYC_HRTCNT > 4002000002U)
#define HRTCNT_BOUND		4000000002U
#else
#define HRTCNT_BOUND		(TCYC_HRTCNT - 2000000U)
#endif

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_CORE_TIMER_H */
