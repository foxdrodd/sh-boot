#include "config.h"
#ifdef CONFIG_LITTLE_ENDIAN
OUTPUT_FORMAT("elf32-sh-linux", "elf32-sh-linux", "elf32-sh-linux")
#else
OUTPUT_FORMAT("elf32-shbig-linux", "elf32-shbig-linux", "elf32-shbig-linux")
#endif
OUTPUT_ARCH(sh)
MEMORY
{
	ROM (rx):      ORIGIN = CONFIG_ROM_START, LENGTH = CONFIG_ROM_LENGTH
#ifdef CONFIG_ETHERNET
	BOOT_RAM(rx):  ORIGIN = CONFIG_RAM_BOOT, LENGTH = 64k
#endif
	RAM (rw):      ORIGIN = CONFIG_RAM_START, LENGTH = 2560
	STACK (rw):    ORIGIN = CONFIG_RAM_START+0xa00, LENGTH = 1536
}

SECTIONS
{
  .text :
  {
    entry.o(.text)
    *(.text)
    *(.text.*)
    *(.stub)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t*)
    *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*)
  } > ROM
  _etext = .;
  PROVIDE (etext = .);
#ifdef CONFIG_ETHERNET
  __ethboot_data_rom_start = .;
  .ethboot_data : AT ( ADDR (.text) + SIZEOF (.text) )
  {
    __ethboot_data_start = . ;
    ethboot/etherboot.o(.data)
    ethboot/etherboot.o*(.data.*)
    __got_start = . ;
    ethboot/etherboot.o(.got)
    __got_end = . ;
    ethboot/etherboot.o(.got.plt)
    ethboot/etherboot.o(.data1)
    __ethboot_data_end = . ;
  } > BOOT_RAM
  . = ALIGN(4);
  .ethboot_bss :
  {
    __ethboot_bss_start = .;		/* BSS */
    ethboot/etherboot.o(.bss)
    ethboot/etherboot.o(.bss.*)
    ethboot/etherboot.o(COMMON)
    __ethboot_bss_end = .;
  } > BOOT_RAM
#endif
  . = ALIGN(4);
  .data :
  {
    *(.data)
    *(.gnu.linkonce.d*)
  } > RAM
  .bss :
  {
    __bss_start = .;		/* BSS */
    *(.bss)
    *(.bss.*)
    *(COMMON)
    __bss_end = .;
  } > RAM
  . = ALIGN(4);
  .stack   :
  {
    *(.stack)
  } > STACK
  .stab :
  {
    *(.stab)
  }
  .comment :
  {
    *(.comment)
  }
  .stabstr :
  {
    *(.stabstr)
  }
}
