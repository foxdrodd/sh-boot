/*
 *	$Id: boot.lds,v 1.1 2001/06/07 04:47:07 sugioka Exp $
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
