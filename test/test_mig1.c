/*
 *  TOPPERS Software
 *      Toyohashi Open Platform for Embedded Real-Time Systems
 * 
 *  Copyright (C) 2007-2018 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: test_mig1.c 176 2019-09-05 05:21:03Z ertl-honda $
 */

/* 
 *		タスクの割付けプロセッサの変更のテスト(1)
 *  【テストの目的】
 *    対象タスクが，休止状態，実行可能状態のタスクに対して，mig_tskを発行する．
 *
 *  【テスト項目】
 *    (1-A)対象タスクが，休止状態
 *    実行可能状態のタスクはさらに，移動先CPUのレディキューでの位置により場合分けを行う．
 *
 *    (1-B) 最高優先度
 *    (1-C) 実行状態のタスクと同優先度
 *    (1-D) 実行状態のタスクより低優先度
 *    (1-E) レディキューが空のところへ移動
 */

#include <kernel.h>
#include <t_syslog.h>
#include <sil.h>
#include "syssvc/test_svc.h"
#include "kernel_cfg.h"
#include "test_mig1.h"

void
task1_1(intptr_t exinf)
{
	ER		ercd;
	FLGPTN  flgptn;

	check_point(1);
	ercd = mig_tsk(TASK1_5,2);
	check_ercd(ercd, E_OK);

	check_point(2);
	ercd = mig_tsk(TASK1_3,2);
	check_ercd(ercd, E_OK);

	//1-3run待ち
	do{
		ercd = pol_flg(FLG1,8,TWF_ANDW,&flgptn);
	} while(ercd != E_OK);

	check_point(3);
	ercd = mig_tsk(TASK1_4,2);
	check_ercd(ercd, E_OK);
  
	check_point(4);
	ercd = mig_tsk(TASK1_7,2);
	check_ercd(ercd, E_OK);

	//同期フラグセット 1_3に移動完了を通知
	check_point(5);
	ercd = set_flg(FLG1,1);
	check_ercd(ercd, E_OK);

	//同期フラグセット待ち 1_7の起動待ち
	do{
		ercd = pol_flg(FLG1,2,TWF_ANDW,&flgptn);
	} while(ercd != E_OK);

	check_point(6);
	ercd = mig_tsk(TASK1_6,2);
	check_ercd(ercd, E_OK);

	check_point(7);
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point(0);
}

void
task1_3(intptr_t exinf)
{
	ER		ercd;
	FLGPTN flgptn;

	check_point_prc(1, PRC2);

	//1-1に移動を通知
	ercd = set_flg(FLG1,8);
	check_ercd(ercd, E_OK);

	do{
		ercd = pol_flg(FLG1,1,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);
  
	check_point_prc(2, PRC2);
	ercd = act_tsk(TASK1_5);
	check_ercd(ercd, E_OK);

	check_point_prc(3, PRC2);
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}

void
task1_4(intptr_t exinf)
{
	ER		ercd;

	check_point_prc(4, PRC2);
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point(0);
}

void
task1_5(intptr_t exinf)
{
	ER		ercd;

	check_point_prc(5, PRC2);
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}

void
task1_6(intptr_t exinf)
{
	ER		ercd;

	check_point_prc(7, PRC2);
	ercd = set_flg(FLG1,4);
	check_ercd(ercd, E_OK);

	check_point_prc(8, PRC2);
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}

void
task1_7(intptr_t exinf)
{
	ER		ercd;
	FLGPTN  flgptn;

	check_point_prc(6, PRC2);
	//同期フラグセット 1_1に起動通知
	ercd = set_flg(FLG1,2);
	check_ercd(ercd, E_OK);

	//同期フラグセット待ち 1_6の起動待ち
	do{
		ercd = pol_flg(FLG1,4,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);
  
	check_point_prc(9, PRC2);
	//同期フラグセット
	ercd = set_flg(FLG1,0x10);
	check_ercd(ercd, E_OK);

	check_point_prc(10, PRC2);
	//同期フラグセット
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);
    
	check_point(0);
}

void
task1_8(intptr_t exinf)
{
	ER		ercd;
	FLGPTN flgptn;
  
	check_point(8);
  
	do{
		ercd = pol_flg(FLG1,0x10,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);

	check_point(9);
	ercd = mig_tsk(TASK1_9,2);
	check_ercd(ercd, E_OK);

	/*
	 *  テスト終了
	 */

	check_finish(10);//CPU1

}

void
task1_9(intptr_t exinf)
{
	/*
	 *  テスト終了
	 */
    check_finish_prc(11, PRC2);//CPU2
}
