/*
 *	$Id: boot.lds,v 1.3 2005/08/05 01:25:20 lethal Exp $
 *	Copyright (C) 2000 YAEGASHI Takeshi <yaegashi@ma.kcom.ne.jp>
 */

OUTPUT_FORMAT("elf32-sh-linux", "elf32-sh-linux", "elf32-sh-linux")
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
