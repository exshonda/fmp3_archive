/*
 *		t_stddef.hのターゲット依存部（ZCU102 Cortex-A53(AArch64)用）
 *
 *  このヘッダファイルは，t_stddef.hの先頭でインクルードされる．他のファ
 *  イルからは直接インクルードすることはない．他のヘッダファイルに先立っ
 *  て処理されるため，他のヘッダファイルに依存してはならない．
 * 
 *  $Id: target_stddef.h 213 2020-02-15 17:11:06Z ertl-honda $
 */

#ifndef TOPPERS_TARGET_STDDEF_H
#define TOPPERS_TARGET_STDDEF_H

/*
 *  ターゲットを識別するためのマクロの定義
 */
#define TOPPERS_ZCU102_ARM64					/* システム略称 */

/*
 *  チッブ依存で共通な定義
 */
#include "chip_stddef.h"

/*
 *  アサーションの失敗時の実行中断処理
 */
#ifndef TOPPERS_MACRO_ONLY

Inline void
TOPPERS_assert_abort(void)
{

#if defined(TOPPERS_USE_QEMU) && !defined(TOPPERS_OMIT_QEMU_SEMIHOSTING)
	/*
	 *  デバッグコンソールへ文字列を出力．
	 */
	Asm("mov x1, %0\n\t"
		"mov x0, #4\n\t"
		"hlt #0xF000" : : "r"("Abort!\n"));

	/*
	 *  QEMUを終了させる．
	 */
	Asm("ldr x1, =0x20026\n\t"		/* ADP_Stopped_ApplicationExit */ 
		"mov x0, #0x18\n\t"			/* angel_SWIreason_ReportException */
		"hlt #0xF000");
#endif
	while (1) ;					/* trueの定義前なので，1と記述する */
}

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_TARGET_STDDEF_H */
