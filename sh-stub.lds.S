#include "config.h"
#ifdef CONFIG_LITTLE_ENDIAN
OUTPUT_FORMAT("elf32-sh-linux", "elf32-sh-linux", "elf32-sh-linux")
#else
OUTPUT_FORMAT("elf32-shbig-linux", "elf32-shbig-linux", "elf32-shbig-linux")
#endif
OUTPUT_ARCH(sh)
#include MEM

SECTIONS
{
  .text :
  {
    entry.o(.text)
    *(.text)
    *(.rodata)
  } > ROM
  .data :
  {
    *(.data)
  } > RAM
  . = ALIGN(4);
  __bss_start = .;		/* BSS */
  .bss :
  {
    *(.bss)
    *(COMMON)
  } > RAM
  . = ALIGN(4);
  __bss_end = .;
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
