/*
 *		sil.hのターゲット依存部（STM32MP257F-DK用）
 *
 *  このヘッダファイルは，sil.hからインクルードされる．他のファイルから
 *  直接インクルードすることはない．このファイルをインクルードする前に，
 *  t_stddef.hがインクルードされるので，それに依存してもよい．
 * 
 *  $Id: target_sil.h 246 2020-07-09 06:31:08Z ertl-honda $
 */

#ifndef TOPPERS_TARGET_SIL_H
#define TOPPERS_TARGET_SIL_H

/*
 *  プロセッサのエンディアン
 */
#define SIL_ENDIAN_LITTLE

#ifndef TOPPERS_MACRO_ONLY

/*
 *  プロセッサIDの取得
 *
 *  ＜arm64 共通部のリファクタ（sil_get_pid をターゲット依存部へ移動）に対応＞
 *  STM32MP257F-DK は Cortex-A35 ×2．MPIDR_EL1 の AFF0(bit[7:0])がコア番号(0/1)．
 */
Inline void
sil_get_pid(ID *p_prcid)
{
	uint64_t	reg;
	uint32_t	index;

	Asm("mrs %0, mpidr_el1":"=r"(reg));
	index = (uint32_t)reg & 0x000000ff;
	*p_prcid = (ID)index + 1;
}

#endif /* TOPPERS_MACRO_ONLY */

/*
 *  コアで共通な定義（チップ依存部は飛ばす）
 */
#include "core_sil.h"

#endif /* TOPPERS_TARGET_SIL_H */
