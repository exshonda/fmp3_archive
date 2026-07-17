/*
 *		システムサービスのターゲット依存部（STM32MP257F-DK用）
 *
 *  システムサービスのターゲット依存部のヘッダファイル．システムサービ
 *  スのターゲット依存の設定は，できる限りコンポーネント記述ファイルで
 *  記述し，このファイルに記述するものは最小限とする．
 * 
 *  $Id: target_syssvc.h 352 2023-05-05 04:42:48Z ertl-honda $
 */

#ifndef TOPPERS_TARGET_SYSSVC_H
#define TOPPERS_TARGET_SYSSVC_H

#ifdef TOPPERS_OMIT_TECS

#include "stm32mp257f_dk.h"
#include "stm32mp2.h"

/*
 *  起動メッセージのターゲットシステム名
 */
#ifdef TOPPERS_TZ_S
#define TARGET_NAME    "STM32MP257F-DK CA35(AArch64 Secure)"
#else /* TOPPERS_TZ_S */
#define TARGET_NAME    "STM32MP257F-DK CA35(AArch64 Non-Secure)"
#endif /* TOPPERS_TZ_S */

/*
 *  シリアルインタフェースドライバを実行するクラスの定義
 */
#define CLS_SERIAL		CLS_PRC1

/*
 *  システムログの低レベル出力のための文字出力
 *
 *  ターゲット依存の方法で，文字cを表示/出力/保存する．
 */
extern void target_fput_log(char c);

/*
 *  サポートするシリアルポートの数
 */
#define TNUM_PORT		1

/*
 *  SIOドライバで使用するSTM32 USARTに関する設定
 *    USE_UART6 指定時は GPIO ヘッダの USART6，既定は ST-LINK の USART2．
 */
#ifdef USE_UART6
#define SIO_STM32USART_BASE	USART6_BASE
#else
#define SIO_STM32USART_BASE	USART2_BASE
#endif /* USE_UART6 */

/*
 *  SIO割込みを登録するための定義
 */
#ifdef USE_UART6
#define INTNO_SIO		IRQ_USART6	/* SIO割込み番号 */
#else
#define INTNO_SIO		IRQ_USART2	/* SIO割込み番号 */
#endif /* USE_UART6 */

#define ISRPRI_SIO		1			/* SIO ISR優先度 */
#define INTPRI_SIO		(-2)		/* SIO割込み優先度 */
#define INTATR_SIO		TA_NULL		/* SIO割込み属性 */

/*
 *  低レベル出力で使用するSIOポート
 */
#define SIOPID_FPUT		1
/*
 *  ログタスクのスタックサイズ
 */
#define LOGTASK_STACK_SIZE	4096

#endif /* TOPPERS_OMIT_TECS */

/*
 *  コアで共通な定義（チップ依存部は飛ばす）
 */
#include "core_syssvc.h"

#endif /* TOPPERS_TARGET_SYSSVC_H */
