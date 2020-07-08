/* This file is generated from core_rename.def by genrename. */

#ifndef TOPPERS_CORE_RENAME_H
#define TOPPERS_CORE_RENAME_H

/*
 *  arm64.c
 */
#define icache_invalidate_all		_kernel_icache_invalidate_all
#define dcache_invalidate_all		_kernel_dcache_invalidate_all
#define dcache_clean_and_invalidate_all	_kernel_dcache_clean_and_invalidate_all
#define icache_flush				_kernel_icache_flush
#define dcache_flush				_kernel_dcache_flush
#define cache_flush					_kernel_cache_flush
#define icache_enable				_kernel_icache_enable
#define icache_disable				_kernel_icache_disable
#define dcache_enable				_kernel_dcache_enable
#define dcache_disable				_kernel_dcache_disable
#define cache_enable				_kernel_cache_enable
#define cache_disable				_kernel_cache_disable
#define tlb_invalidate_all			_kernel_tlb_invalidate_all
#define mmu_mmap_init				_kernel_mmu_mmap_init
#define mmu_mmap_add				_kernel_mmu_mmap_add
#define mmu_tt_init					_kernel_mmu_tt_init
#define mmu_init					_kernel_mmu_init

/*
 *  core_support.S
 */
#define vector_table				_kernel_vector_table
#define dispatch					_kernel_dispatch
#define dispatch_and_migrate		_kernel_dispatch_and_migrate
#define exit_and_migrate			_kernel_exit_and_migrate
#define start_dispatch				_kernel_start_dispatch
#define exit_and_dispatch			_kernel_exit_and_dispatch
#define call_exit_kernel			_kernel_call_exit_kernel
#define start_r						_kernel_start_r
#define cur_sp0_sync_handler		_kernel_cur_sp0_sync_handler
#define cur_sp0_irq_handler			_kernel_cur_sp0_irq_handler
#define cur_sp0_fiq_handler			_kernel_cur_sp0_fiq_handler
#define cur_sp0_serr_handler		_kernel_cur_sp0_serr_handler
#define cur_spx_irq_handler			_kernel_cur_spx_irq_handler
#define cur_spx_sync_handler		_kernel_cur_spx_sync_handler
#define cur_spx_fiq_handler			_kernel_cur_spx_fiq_handler
#define cur_spx_serr_handler		_kernel_cur_spx_serr_handler
#define l64_sync_handler			_kernel_l64_sync_handler
#define l64_irq_handler				_kernel_l64_irq_handler
#define l64_fiq_handler				_kernel_l64_fiq_handler
#define l64_serr_handler			_kernel_l64_serr_handler
#define l32_sync_handler			_kernel_l32_sync_handler
#define l32_irq_handler				_kernel_l32_irq_handler
#define l32_fiq_handler				_kernel_l32_fiq_handler
#define l32_serr_handler			_kernel_l32_serr_handler

/*
 *  core_kernel_impl.c
 */
#define core_mprc_initialize		_kernel_core_mprc_initialize
#define core_initialize				_kernel_core_initialize
#define core_terminate				_kernel_core_terminate
#define initialize_exception		_kernel_initialize_exception
#define xlog_sys					_kernel_xlog_sys
#define default_exc_handler			_kernel_default_exc_handler
#define default_int_handler			_kernel_default_int_handler
#define arm_fpu_initialize			_kernel_arm_fpu_initialize

/*
 *  kernel_cfg.c
 */
#define p_inh_table					_kernel_p_inh_table
#define p_intcfg_table				_kernel_p_intcfg_table
#define p_exc_table					_kernel_p_exc_table
#define idstkpt_table				_kernel_idstkpt_table

/*
 *  gic_kernel_impl.c
 */
#define gic_init					_kernel_gic_init
#define gicc_stop					_kernel_gicc_stop
#define gicd_disable_int			_kernel_gicd_disable_int
#define gicd_enable_int				_kernel_gicd_enable_int
#define gicd_clear_pending			_kernel_gicd_clear_pending
#define gicd_set_pending			_kernel_gicd_set_pending
#define gicd_probe_pending			_kernel_gicd_probe_pending
#define gicd_config					_kernel_gicd_config
#define gicd_set_priority			_kernel_gicd_set_priority
#define gicd_set_target				_kernel_gicd_set_target
#define gicd_initialize				_kernel_gicd_initialize
#define gicd_terminate				_kernel_gicd_terminate
#define initialize_interrupt		_kernel_initialize_interrupt

/*
 *  gic_support.S
 */
#define irc_begin_int				_kernel_irc_begin_int
#define irc_end_int					_kernel_irc_end_int
#define irc_get_intpri				_kernel_irc_get_intpri
#define irc_begin_exc				_kernel_irc_begin_exc
#define irc_end_exc					_kernel_irc_end_exc

/*
 *  psci_support.S
 */
#define psci_smc_cpuon				_kernel_psci_smc_cpuon


#ifdef TOPPERS_LABEL_ASM

/*
 *  arm64.c
 */
#define _icache_invalidate_all		__kernel_icache_invalidate_all
#define _dcache_invalidate_all		__kernel_dcache_invalidate_all
#define _dcache_clean_and_invalidate_all	__kernel_dcache_clean_and_invalidate_all
#define _icache_flush				__kernel_icache_flush
#define _dcache_flush				__kernel_dcache_flush
#define _cache_flush				__kernel_cache_flush
#define _icache_enable				__kernel_icache_enable
#define _icache_disable				__kernel_icache_disable
#define _dcache_enable				__kernel_dcache_enable
#define _dcache_disable				__kernel_dcache_disable
#define _cache_enable				__kernel_cache_enable
#define _cache_disable				__kernel_cache_disable
#define _tlb_invalidate_all			__kernel_tlb_invalidate_all
#define _mmu_mmap_init				__kernel_mmu_mmap_init
#define _mmu_mmap_add				__kernel_mmu_mmap_add
#define _mmu_tt_init				__kernel_mmu_tt_init
#define mmu_init					__kernel_mmu_init

/*
 *  core_support.S
 */
#define _vector_table				__kernel_vector_table
#define _dispatch					__kernel_dispatch
#define _dispatch_and_migrate		__kernel_dispatch_and_migrate
#define _exit_and_migrate			__kernel_exit_and_migrate
#define _start_dispatch				__kernel_start_dispatch
#define _exit_and_dispatch			__kernel_exit_and_dispatch
#define _call_exit_kernel			__kernel_call_exit_kernel
#define _start_r					__kernel_start_r
#define _cur_sp0_sync_handler		__kernel_cur_sp0_sync_handler
#define _cur_sp0_irq_handler		__kernel_cur_sp0_irq_handler
#define _cur_sp0_fiq_handler		__kernel_cur_sp0_fiq_handler
#define _cur_sp0_serr_handler		__kernel_cur_sp0_serr_handler
#define _cur_spx_irq_handler		__kernel_cur_spx_irq_handler
#define _cur_spx_sync_handler		__kernel_cur_spx_sync_handler
#define _cur_spx_fiq_handler		__kernel_cur_spx_fiq_handler
#define _cur_spx_serr_handler		__kernel_cur_spx_serr_handler
#define _l64_sync_handler			__kernel_l64_sync_handler
#define _l64_irq_handler			__kernel_l64_irq_handler
#define _l64_fiq_handler			__kernel_l64_fiq_handler
#define _l64_serr_handler			__kernel_l64_serr_handler
#define _l32_sync_handler			__kernel_l32_sync_handler
#define _l32_irq_handler			__kernel_l32_irq_handler
#define _l32_fiq_handler			__kernel_l32_fiq_handler
#define _l32_serr_handler			__kernel_l32_serr_handler

/*
 *  core_kernel_impl.c
 */
#define _core_mprc_initialize		__kernel_core_mprc_initialize
#define _core_initialize			__kernel_core_initialize
#define _core_terminate				__kernel_core_terminate
#define _initialize_exception		__kernel_initialize_exception
#define _xlog_sys					__kernel_xlog_sys
#define _default_exc_handler		__kernel_default_exc_handler
#define _default_int_handler		__kernel_default_int_handler
#define _arm_fpu_initialize			__kernel_arm_fpu_initialize

/*
 *  kernel_cfg.c
 */
#define _p_inh_table				__kernel_p_inh_table
#define _p_intcfg_table				__kernel_p_intcfg_table
#define _p_exc_table				__kernel_p_exc_table
#define _idstkpt_table				__kernel_idstkpt_table

/*
 *  gic_kernel_impl.c
 */
#define _gic_init					__kernel_gic_init
#define _gicc_stop					__kernel_gicc_stop
#define _gicd_disable_int			__kernel_gicd_disable_int
#define _gicd_enable_int			__kernel_gicd_enable_int
#define _gicd_clear_pending			__kernel_gicd_clear_pending
#define _gicd_set_pending			__kernel_gicd_set_pending
#define _gicd_probe_pending			__kernel_gicd_probe_pending
#define _gicd_config				__kernel_gicd_config
#define _gicd_set_priority			__kernel_gicd_set_priority
#define _gicd_set_target			__kernel_gicd_set_target
#define _gicd_initialize			__kernel_gicd_initialize
#define _gicd_terminate				__kernel_gicd_terminate
#define _initialize_interrupt		__kernel_initialize_interrupt

/*
 *  gic_support.S
 */
#define _irc_begin_int				__kernel_irc_begin_int
#define _irc_end_int				__kernel_irc_end_int
#define _irc_get_intpri				__kernel_irc_get_intpri
#define _irc_begin_exc				__kernel_irc_begin_exc
#define _irc_end_exc				__kernel_irc_end_exc

/*
 *  psci_support.S
 */
#define _psci_smc_cpuon				__kernel_psci_smc_cpuon

#endif /* TOPPERS_LABEL_ASM */

#endif /* TOPPERS_CORE_RENAME_H */
