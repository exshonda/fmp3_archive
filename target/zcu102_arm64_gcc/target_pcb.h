/*
 *		pcb.hのターゲット依存部（ZCU102 Cortex-A53(AArch64)用）
 *
 *  $Id: target_pcb.h 213 2020-02-15 17:11:06Z ertl-honda $
 */

#ifndef TOPPERS_TARGET_PCB_H
#define TOPPERS_TARGET_PCB_H

/*
 *  スレッドIDレジスタにPCBへのポインタを入れる場合
 */
#define USE_THREAD_ID_PCB

/*
 *  コアで共通な定義（チップ依存部は飛ばす）
 */
#include "core_pcb.h"

#endif /* TOPPERS_TARGET_PCB_H */
