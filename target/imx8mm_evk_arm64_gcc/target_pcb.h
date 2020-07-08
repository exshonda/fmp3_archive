/*
 *		pcb.hのターゲット依存部（IMX8M_EVK用）
 *
 *  $Id: target_pcb.h 246 2020-07-09 06:31:08Z ertl-honda $
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
