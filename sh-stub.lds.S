#include "config.h"
#ifdef CONFIG_LITTLE_ENDIAN
OUTPUT_FORMAT("elf32-shl", "elf32-shl", "elf32-shl")
#else
OUTPUT_FORMAT("elf32-sh", "elf32-sh", "elf32-sh")
#endif
OUTPUT_ARCH(sh)
#if defined (CONFIG_TEST)
#include "machine/test.mem"
#elif defined(CONFIG_CQSH3)
#include "machine/cqsh3.mem"
#elif defined(CONFIG_CQSH4)
#include "machine/cqsh4.mem"
#elif defined(CONFIG_SESH3)
#include "machine/sesh3.mem"
#elif defined(CONFIG_SESH4)
#include "machine/sesh4.mem"
#elif defined(CONFIG_APSH4)
#include "machine/apsh4.mem"
#elif defined(CONFIG_HP600)
#include "machine/hp600.mem"
#endif
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
