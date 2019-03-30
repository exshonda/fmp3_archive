/*
 *		pcb.hのターゲット依存部（ZYBO用）
 *
 *  $Id: target_pcb.h 135 2019-01-28 14:31:50Z ertl-honda $
 */

#ifndef TOPPERS_TARGET_PCB_H
#define TOPPERS_TARGET_PCB_H

/*
 *  スレッドIDレジスタに自プロセッサPCBへのポインタを格納する
 */
#define USE_THREAD_ID_PCB

/*
 *  コアで共通な定義（チップ依存部は飛ばす）
 */
#include "core_pcb.h"

#endif /* TOPPERS_TARGET_CCB_H */
