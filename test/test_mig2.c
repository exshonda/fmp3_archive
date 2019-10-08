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
 *  @(#) $Id: test_mig2.c 176 2019-09-05 05:21:03Z ertl-honda $
 */

/* 
 *		タスクの割付けプロセッサの変更のテスト(2)
 *  【テストの目的】
 *    対象タスクが，実行状態のタスクに対して，mig_tskを発行する．
 *
 *  【テスト項目】
 *    移動先CPUのレディキューでの位置により場合分けを行う．
 *
 *    (1-F) 最高優先度
 *    (1-G) 実行状態のタスクと同優先度
 *    (1-H) 実行状態のタスクより低優先度
 *    (1-I) レディキューが空のところへ移動
 */


#include <kernel.h>
#include <t_syslog.h>
#include <sil.h>
#include "syssvc/test_svc.h"
#include "kernel_cfg.h"
#include "test_mig2.h"

void
task1_1(intptr_t exinf)
{
	ER		ercd;
	ID    cpuno;

	check_point(1);//1
	ercd = mig_tsk(TASK1_1,2);
	check_ercd(ercd, E_OK);

	do{
		get_pid(&cpuno);
	}while(cpuno==1);

	check_point_prc(1, PRC2);//2
	ercd = act_tsk(TASK2_2);
	check_ercd(ercd, E_OK);

	check_point_prc(2, PRC2);//2
	ercd = act_tsk(TASK2_3);
	check_ercd(ercd, E_OK);

	check_point_prc(3, PRC2);//2
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}
  
void
task1_4(intptr_t exinf)
{
	ER		ercd;
	FLGPTN  flgptn;

	//同期フラグセット 2_2に起動を通知

	check_point(2);//1
	ercd = set_flg(FLG1,2);
	check_ercd(ercd, E_OK);

	//同期フラグ待ち 2_3の起動待ち
	do{
		ercd = pol_flg(FLG1,0x10,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);

	check_point(3);//1
	ercd = mig_tsk(TASK1_4,2);
	check_ercd(ercd, E_OK);

	check_point_prc(8, PRC2);//2
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}

void
task1_6(intptr_t exinf)
{
	ER		ercd;
	FLGPTN  flgptn;

	check_point(7);//1

	//同期フラグ待ち 1_7の移動待ち
	do{
		ercd = pol_flg(FLG1,1,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);

	check_point(8);//1

	ercd = mig_tsk(TASK1_6,2);
	check_ercd(ercd, E_OK);

	//同期フラグセット 1-7に移動を通知
	check_point_prc(10, PRC2);//2
	ercd = set_flg(FLG1,8);
	check_ercd(ercd, E_OK);

	check_point_prc(11, PRC2);//2
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}

void
task1_7(intptr_t exinf)
{
	ER		ercd;
	FLGPTN  flgptn;

	//同期フラグセット 2-3に起動を通知
	check_point(4);//1
	ercd = set_flg(FLG1,4);
	check_ercd(ercd, E_OK);

	check_point(5);//1
	ercd = mig_tsk(TASK1_7,2);
	check_ercd(ercd, E_OK);

	//同期フラグセット 1-6に移動を通知
	check_point_prc(9, PRC2);//2
	ercd = set_flg(FLG1,1);
	check_ercd(ercd, E_OK);

	//同期フラグ待ち 1_6の移動待ち
	do{
		ercd = pol_flg(FLG1,8,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);


	/*テスト終了*/
	check_finish_prc(12, PRC2);//CPU2
}

void
task1_9(intptr_t exinf)
{
	ER		ercd;

	check_point(6);//1
	ercd = act_tsk(TASK1_6);
	check_ercd(ercd, E_OK);

	/*テスト終了*/
	check_finish(9);//1
}

void
task2_2(intptr_t exinf)
{
	ER		ercd;
	FLGPTN  flgptn;

	check_point_prc(4, PRC2);

	//同期フラグ待ち 1_4の起動待ち
	do {
		ercd = pol_flg(FLG1,2,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);

	check_point_prc(5, PRC2);//2
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}

void
task2_3(intptr_t exinf)
{
	ER		ercd;
	FLGPTN flgptn;

	check_point_prc(6, PRC2);//2

	ercd = set_flg(FLG1,0x10); 

	//同期フラグ待ち1_7の起動待ち
	do{
		ercd = pol_flg(FLG1,4,TWF_ANDW,&flgptn);
	}while(ercd != E_OK);

	check_point_prc(7, PRC2);//2
	ercd = slp_tsk();
	check_ercd(ercd, E_OK);

	check_point_prc(0, PRC2);
}
