#include "config.h"
#ifdef CONFIG_LITTLE_ENDIAN
OUTPUT_FORMAT("elf32-sh-linux", "elf32-sh-linux", "elf32-sh-linux")
#else
OUTPUT_FORMAT("elf32-shbig-linux", "elf32-shbig-linux", "elf32-shbig-linux")
#endif
OUTPUT_ARCH(sh)
ENTRY(start)	
MEMORY
{
	ROM (rx):      ORIGIN = CONFIG_ROM_START, LENGTH = CONFIG_ROM_LENGTH
	RAM (rw):      ORIGIN = CONFIG_RAM_START+0x2000, LENGTH = 8k
	STACK (rw):    ORIGIN = CONFIG_RAM_START+0xa00, LENGTH = 0x600
}

SECTIONS
{
  .text :
  {
    *(.vect)	
    entry.o(.text)
    *(.text)
    *(.text.init)
    *(.text.*)
    __initcall_start = .;
    *(.initcall.init)
    __initcall_end = .;	
    *(.stub)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    *(.gnu.linkonce.t*)
    *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*)
  } > ROM
  . = ALIGN(16);
  _etext = .;
  PROVIDE (etext = .);
  . = ALIGN(4);
  .data :
  {
    __data_start = .;		/* BSS */	
    *(.data)
    *(.gnu.linkonce.d*)
    __data_end = .;	
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
  .stack (NOLOAD):
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
