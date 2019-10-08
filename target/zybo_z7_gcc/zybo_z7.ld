/*
 *  $Id: zybo_z7.ld 178 2019-10-08 13:55:00Z ertl-honda $
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm") 
OUTPUT_ARCH(arm)
ENTRY(start);

MEMORY {
	RAM (xrw) : ORIGIN = 0x00100000, LENGTH = 511M
}

PROVIDE(hardware_init_hook = 0);
PROVIDE(software_init_hook = 0);
PROVIDE(software_term_hook = 0);

SECTIONS {
	.text : {
		__start_text = .;
		*(.vector)
		*(.text)
		*(.text.*)
		*(.glue_7t)
		*(.glue_7)
		__end_kernel = .;
	} > RAM
	_etext = .;
	PROVIDE(etext = .);

	.rodata ALIGN(4) : {
		__start_rodata = .;
		*(.rodata)
		*(.rodata.*)
		/* GCOV SUPPORT */
		. = ALIGN(4) ;
		_ctor_list = . ;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		LONG (0);

		_dtor_list = . ;
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
		LONG (0);
		/* GCOV SUPPORT */
		__end_rodata = .;
	} > RAM
	.preinit_array ALIGN(4) : {
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP (*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);
	} > RAM
	.init_array ALIGN(4) : {
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
	} > RAM
	.fini_array ALIGN(4) : {
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP (*(SORT(.fini_array.*)))
		KEEP (*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);
	} > RAM
	.ARM.exidx ALIGN(4) : {
		__exidx_start = .;
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__exidx_end = .;
	} > RAM

	.data : ALIGN(4) {
		__start_data = .;
		*(.data)
		*(.data.*)
		__end_data = .;
	} > RAM
	__start_idata = LOADADDR(.data);
	. = ALIGN(4);
	_edata = .;
	PROVIDE(edata = .);

	.bss ALIGN(4) : {
		__start_bss = .;
		*(.bss)
		*(.bss.*)
		*(.kernel_data_*)
		*(.stack*)
		*(COMMON)
		__end_bss = .;
	} > RAM
	. = ALIGN(4);
	_end = .;
	_heap = .; /* GCOV SUPPORT */
	PROVIDE(end = .);
	.comment 0 : { *(.comment) }	
	/* GCOV SUPPORT */
	_heap_size = 0x1000000;
	_heap_limit = _heap + _heap_size;
	/* GCOV SUPPORT */

	/*
	 *  DWARF debug sections.
	 *  Symbols in the DWARF debugging sections are relative to
	 *  the beginning of the section so we begin them at 0.
	 */

	/* DWARF 1 */
	.debug			0 : { *(.debug) }
	.line			0 : { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo	0 : { *(.debug_srcinfo) }
	.debug_sfnames	0 : { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges	0 : { *(.debug_aranges) }
	.debug_pubnames	0 : { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info		0 : { *(.debug_info) }
	.debug_abbrev	0 : { *(.debug_abbrev) }
	.debug_line		0 : { *(.debug_line) }
	.debug_frame	0 : { *(.debug_frame) }
	.debug_str		0 : { *(.debug_str) }
	.debug_loc		0 : { *(.debug_loc) }
	.debug_macinfo	0 : { *(.debug_macinfo) }

	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}	
