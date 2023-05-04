/*
 *		システムサービスのターゲット依存部（IMX8MM_EVK用）
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

#include "imx8mm_evk.h"
#include "imx8mm.h"

/*
 *  起動メッセージのターゲットシステム名
 */
#ifdef TOPPERS_TZ_S
#define TARGET_NAME    "i.MX 8M Mini EVK CA53(AArch64 Secure)"
#else /* TOPPERS_TZ_S */
#define TARGET_NAME    "i.MX 8M Mini EVK CA53(AArch64 Non-Secure)"
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
 *  SIOドライバで使用するIMX8UARTに関する設定
 */
#ifdef USE_UART1
#define SIO_IMX8UART_BASE	UART1_BASE
#elif defined(USE_UART2)
#define SIO_IMX8UART_BASE	UART2_BASE
#elif defined(USE_UART3)
#define SIO_IMX8UART_BASE	UART3_BASE
#elif defined(USE_UART4)
#define SIO_IMX8UART_BASE	UART4_BASE
#endif /* USE_UART1 */

#define SIO_IMX8UART_RFDIF		IMX8UART_RFDIF
#define SIO_IMX8UART_UBIR		IMX8UART_UBIR
#define SIO_IMX8UART_UBMR		IMX8UART_UBMR

/*
 *  SIO割込みを登録するための定義
 */
#ifdef USE_UART1
#define INTNO_SIO		IRQ_UART1	/* SIO割込み番号 */
#elif defined(USE_UART2)
#define INTNO_SIO		IRQ_UART2	/* SIO割込み番号 */
#elif defined(USE_UART3)
#define INTNO_SIO		IRQ_UART3	/* SIO割込み番号 */
#elif defined(USE_UART4)
#define INTNO_SIO		IRQ_UART4	/* SIO割込み番号 */
#endif /* USE_UART1 */

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
