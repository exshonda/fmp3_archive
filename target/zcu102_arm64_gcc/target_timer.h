/*
 *		タイマドライバ
 * 
 *  $Id: target_timer.h 224 2020-02-24 14:47:26Z ertl-honda $
 */

#ifndef TOPPERS_TARGET_TIMER_H
#define TOPPERS_TARGET_TIMER_H


#define INTPRI_TIMER		(TMAX_INTPRI - 1)		/* 割込み優先度 */

/*
 *  チップで共通な定義
 */
#include "chip_timer.h"

#endif /* TOPPERS_TARGET_TIMER_H */
