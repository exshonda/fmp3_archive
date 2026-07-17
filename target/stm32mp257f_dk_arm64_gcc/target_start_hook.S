/*
 *  ハードウェア初期化フック（STM32MP257F-DK用 / FMP3）
 *
 *  エントリ時のキャッシュ／MMU状態の正規化を行う．
 *
 *  本ボードは TF-A(FSBL) やデバッガによる再ロードなど，キャッシュ／
 *  MMU が有効な状態でエントリすることがあるため，start.S（共通部）の
 *  hardware_init_hook（weak）を上書きし，コア依存部の正規化ルーチン
 *  （core_support.S の arm64_normalize_cache_state_el1）を呼び出す．
 *
 *  tail-call（b 命令）とすることで，x30（start.S への戻りアドレス）
 *  をそのまま使ってルーチン側の ret で復帰し，スタックを使用しない．
 *  FMP3 はマルチコアだが，hardware_init_hook は各コアの start_el1 で
 *  （他コア起動前に）呼ばれ，set/way 掃引は自 PE のキャッシュのみに
 *  作用するため，各コアが自コア分を正規化する形で正しく機能する．
 *  必要性・前提条件・問題の再現方法は asp3_core の
 *  arch/arm64_gcc/docs/entry-cache-normalization.md を参照．
 */
#define TOPPERS_MACRO_ONLY
#define UINT_C(val)		(val)		/* uint_t型の定数を作るマクロ */
#define ULONG_C(val)	(val)		/* ulong_t型の定数を作るマクロ */

#include "kernel_impl.h"
#include "core_asm.inc"

	ATEXT
	AALIGN(2)
	AGLOBAL(hardware_init_hook)
ALABEL(hardware_init_hook)
	b	arm64_normalize_cache_state_el1

/*
 *  arm64_normalize_cache_state_el1 の弱定義（空）
 *
 *  パス1（cfg1_out）は libkernel.a（core_support.o）をリンクしないため，
 *  未定義参照を避ける目的で空の弱定義を置く．fmp（実行イメージ）では
 *  常にリンクされる core_support.o の強定義が優先される（cfg1_out は
 *  実行されないため空実装で問題ない）．
 */
	AWEAK(arm64_normalize_cache_state_el1)
ALABEL(arm64_normalize_cache_state_el1)
	ret
