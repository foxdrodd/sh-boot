/*
 *	$Id: boot.lds,v 1.2 2003/12/12 15:34:44 lethal Exp $
 *	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>
 */

OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
OUTPUT_ARCH(sh)
ENTRY(start)
SECTIONS {
	. = 0x8c010000;
	.text : {
		*(.text)
		*(.rodata)
	}
	. = ALIGN(4);
	.data : {
		*(.data)
	}
	. = ALIGN(4);
	__bss_start = .;
	.bss : {
		*(.bss)
		*(COMMON)
	}
	. = ALIGN(4);
	__bss_end = .;
}
