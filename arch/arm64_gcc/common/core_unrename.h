/* This file is generated from core_rename.def by genrename. */

/* This file is included only when core_rename.h has been included. */
#ifdef TOPPERS_CORE_RENAME_H
#undef TOPPERS_CORE_RENAME_H

/*
 *  arm64.c
 */
#undef icache_invalidate_all
#undef dcache_invalidate_all
#undef dcache_clean_and_invalidate_all
#undef icache_flush
#undef dcache_flush
#undef cache_flush
#undef icache_enable
#undef icache_disable
#undef dcache_enable
#undef dcache_disable
#undef cache_enable
#undef cache_disable
#undef tlb_invalidate_all
#undef mmu_mmap_init
#undef mmu_mmap_add
#undef mmu_tt_init
#undef mmu_init

/*
 *  core_support.S
 */
#undef vector_table
#undef dispatch
#undef dispatch_and_migrate
#undef exit_and_migrate
#undef start_dispatch
#undef exit_and_dispatch
#undef call_exit_kernel
#undef start_r
#undef cur_sp0_sync_handler
#undef cur_sp0_irq_handler
#undef cur_sp0_fiq_handler
#undef cur_sp0_serr_handler
#undef cur_spx_irq_handler
#undef cur_spx_sync_handler
#undef cur_spx_fiq_handler
#undef cur_spx_serr_handler
#undef l64_sync_handler
#undef l64_irq_handler
#undef l64_fiq_handler
#undef l64_serr_handler
#undef l32_sync_handler
#undef l32_irq_handler
#undef l32_fiq_handler
#undef l32_serr_handler

/*
 *  core_kernel_impl.c
 */
#undef core_mprc_initialize
#undef core_initialize
#undef core_terminate
#undef initialize_exception
#undef xlog_sys
#undef default_exc_handler
#undef default_int_handler
#undef arm_fpu_initialize

/*
 *  kernel_cfg.c
 */
#undef p_inh_table
#undef p_intcfg_table
#undef p_exc_table
#undef idstkpt_table

/*
 *  gic_kernel_impl.c
 */
#undef gic_init
#undef gicc_stop
#undef gicd_disable_int
#undef gicd_enable_int
#undef gicd_clear_pending
#undef gicd_set_pending
#undef gicd_probe_pending
#undef gicd_config
#undef gicd_set_priority
#undef gicd_set_target
#undef gicd_initialize
#undef gicd_terminate
#undef initialize_interrupt

/*
 *  gic_support.S
 */
#undef irc_begin_int
#undef irc_end_int
#undef irc_get_intpri
#undef irc_begin_exc
#undef irc_end_exc

/*
 *  psci_support.S
 */
#undef psci_smc_cpuon


#ifdef TOPPERS_LABEL_ASM

/*
 *  arm64.c
 */
#undef _icache_invalidate_all
#undef _dcache_invalidate_all
#undef _dcache_clean_and_invalidate_all
#undef _icache_flush
#undef _dcache_flush
#undef _cache_flush
#undef _icache_enable
#undef _icache_disable
#undef _dcache_enable
#undef _dcache_disable
#undef _cache_enable
#undef _cache_disable
#undef _tlb_invalidate_all
#undef _mmu_mmap_init
#undef _mmu_mmap_add
#undef _mmu_tt_init
#undef mmu_init

/*
 *  core_support.S
 */
#undef _vector_table
#undef _dispatch
#undef _dispatch_and_migrate
#undef _exit_and_migrate
#undef _start_dispatch
#undef _exit_and_dispatch
#undef _call_exit_kernel
#undef _start_r
#undef _cur_sp0_sync_handler
#undef _cur_sp0_irq_handler
#undef _cur_sp0_fiq_handler
#undef _cur_sp0_serr_handler
#undef _cur_spx_irq_handler
#undef _cur_spx_sync_handler
#undef _cur_spx_fiq_handler
#undef _cur_spx_serr_handler
#undef _l64_sync_handler
#undef _l64_irq_handler
#undef _l64_fiq_handler
#undef _l64_serr_handler
#undef _l32_sync_handler
#undef _l32_irq_handler
#undef _l32_fiq_handler
#undef _l32_serr_handler

/*
 *  core_kernel_impl.c
 */
#undef _core_mprc_initialize
#undef _core_initialize
#undef _core_terminate
#undef _initialize_exception
#undef _xlog_sys
#undef _default_exc_handler
#undef _default_int_handler
#undef _arm_fpu_initialize

/*
 *  kernel_cfg.c
 */
#undef _p_inh_table
#undef _p_intcfg_table
#undef _p_exc_table
#undef _idstkpt_table

/*
 *  gic_kernel_impl.c
 */
#undef _gic_init
#undef _gicc_stop
#undef _gicd_disable_int
#undef _gicd_enable_int
#undef _gicd_clear_pending
#undef _gicd_set_pending
#undef _gicd_probe_pending
#undef _gicd_config
#undef _gicd_set_priority
#undef _gicd_set_target
#undef _gicd_initialize
#undef _gicd_terminate
#undef _initialize_interrupt

/*
 *  gic_support.S
 */
#undef _irc_begin_int
#undef _irc_end_int
#undef _irc_get_intpri
#undef _irc_begin_exc
#undef _irc_end_exc

/*
 *  psci_support.S
 */
#undef _psci_smc_cpuon

#endif /* TOPPERS_LABEL_ASM */

#endif /* TOPPERS_CORE_RENAME_H */
