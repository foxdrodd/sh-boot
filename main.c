/* $Id: main.c,v 1.45 2001/09/18 06:57:49 gniibe Exp $
 *
 * sh-ipl+g/main.c
 *
 *  Copyright (C) 2000  Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */

#include "config.h"
#include "defs.h"
#include "string.h"
#include "io.h"

static int shutdown (unsigned int);
static int rtc (unsigned int, unsigned int);

/*
  Runs at P2 Area

  VBR = 0xa0000000

  ROM:
   0xa0000000 ---> [ .text  ]

  RAM:
  		   [ .data  ]  <-------   RAM = 0xa8000000
		   [ .bss   ]
   init_stack ---> [ .stack ]  RAM+0x0a00
		      :
		   [        ]
		   [        ]  RAM+0x0eff <--- init_sp (= initial R15)
   stub_stack ---> [ .stack ]  RAM+0x0F00
		      :
		   [        ]
		   [        ]  RAM+0x1000 <--- stub_sp (= stub R15) 
 */

#define stub_stack_size	64
#define init_stack_size	320
static int init_stack[init_stack_size]
	__attribute__ ((section (".stack"),unused));
int stub_stack[stub_stack_size]
	__attribute__ ((section (".stack")));

int *stub_sp;

static const char * const banner = "\n"
"SH IPL+g version 0.11, Copyright (C) 2001 Free Software Foundation, Inc.\n"
"\n"
"This software comes with ABSOLUTELY NO WARRANTY; for details type `w'.\n"
"This is free software, and you are welcome to redistribute it under\n"
"certain conditions; type `l' for details.\n\n";

static const char * const license_message = "\n"
"SH IPL+g is free software; you can redistribute it and/or modify it under\n"
"the terms of the GNU Lesser General Public License as published by \n"
"the Free Software Foundatin; either version 2.1 of the License, or (at your\n"
"option) any later version.\n";

static const char * const warranty_message = "\n"
"SH IPL+g is distributed in the hope that it will be useful, but\n"
"WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
"Lesser General Public License for more details.\n";

static const char * const help_message = "\n"
"SH IPL+g version 0.11, Copyright (C) 2001 Free Software Foundation, Inc.\n"
"    ? --- Show this message (HELP)\n"
"    b --- Boot the system\n"
"    g --- Invoke GDB stub\n"
"    l --- Show about license\n"
"    w --- Show about (no)warranty\n\n"
#if defined(CONFIG_ETHERNET)
"    e --- Ether Boot\n"
#endif
#if defined(CONFIG_BOOT_LOADER1)
"    1 --- Boot loader 1\n"
#endif
#if defined(CONFIG_BOOT_LOADER2)
"    2 --- Boot loader 2\n"
#endif
; /* Nasty dangling ; for initialisation - don't delete! */

static char *prompt;
int expevt_on_start;

#if defined(CONFIG_IDE)
static void
boot (void)
{
  register long __sc0 __asm__ ("r0") = 2; /* READ SECTORS */
  register long __sc4 __asm__ ("r4") = (long) 0;
  register long __sc5 __asm__ ("r5") = (long) 0;
  register long __sc6 __asm__ ("r6") = (long) CONFIG_RAM_BOOT;
  register long __sc7 __asm__ ("r7") = (long) 1;

  asm volatile  ("trapa	#0x3F"
		 : "=z" (__sc0)
		 : "0" (__sc0), "r" (__sc4), "r" (__sc5), "r" (__sc6), 
		   "r" (__sc7)
		 : "memory");

  if (__sc0 < 0)
    putString ("MBR read error\n");
  else
    asm volatile ("jmp @r0; nop"
		  : : "z" (CONFIG_RAM_BOOT));
}
#endif

void
start_main (void)
{
  extern void start ();		/* In entry.S */
  char c;

  in_nmi = 0;
  dofault = 1;
  stepped = 0;
  prompt = "> ";

  stub_sp = stub_stack + stub_stack_size;
  putString (banner);

#if defined(CONFIG_CPU_SUBTYPE_SH7751) && defined(CONFIG_PCI)
  if (init_pcic() != 0)
    {
      putString("Error in PCIC initialization!\n");
    }
#endif

#if defined(CONFIG_AUTO_BOOT_IDE)
  if (AUTO_BOOT_IDE_CHECK && (ide_detect_devices () == 0))
    {
      sti ();
      ide_startup_devices ();
      boot ();
    }
#endif

#if defined(CONFIG_ETHERNET)
  if (AUTO_BOOT_ETHER_CHECK)
    {
      sti ();
      etherboot ();
    }
#endif

  if (expevt_on_start != 0)
    {				/* Reboot */
      putString ("!\n");
#ifdef CONFIG_IDE
      if (ide_detect_devices () == 0)
	{
	  sti ();
	  ide_startup_devices ();
	  boot ();
	}
#endif
#ifdef CONFIG_ETHERNET
      sti ();
      etherboot ();
#endif
#ifdef CONFIG_BOOT_LOADER1
      sti ();
      asm volatile ("jmp @r0; nop"
		    : : "z" (CONFIG_BOOT_LOADER1));
#endif
    }

  while (1)
    {
      putString (prompt);
      c = getDebugChar ();
      switch (c)
	{
	case '?':
	case 'h':
	  /* Help */
	  putString ("h");
	  putString (help_message);
	  break;
	case 'b':
	  /* Boot */
	  putString ("b\n");
#if defined(CONFIG_IDE)
	  if (ide_detect_devices () == 0)
	    {
	      sti ();
	      ide_startup_devices ();
	      boot ();
	    }
	  else
	    putString ("IDE: device not detected.\n");
#else
	  putString ("Not Implemented Yet.\n");
#endif
	  break;
	case 'l':
	  /* License */
	  putString ("l");
	  putString (license_message);
	  break;
	case 'g':
	case '#':
	  /* GDB Stub */
	  sti ();
	  putString ("g\n");
	  breakpoint ();
	  break;
	case 'w':
	  /* Warranty */
	  putString ("w");
	  putString (warranty_message);
	  break;
#if defined(CONFIG_ETHERNET)
	case 'e':
	  /* Ether Boot */
	  sti ();
	  putString ("e");
	  etherboot ();
	  break;
#endif
#if defined(CONFIG_BOOT_LOADER1)
	case '1':
	  /* Boot loader 1 */
	  sti ();
	  putString ("1");
	  asm volatile ("jmp @r0; nop"
			: : "z" (CONFIG_BOOT_LOADER1));
	  break;
#endif
#if defined(CONFIG_BOOT_LOADER2)
	case '2':
	  sti ();
	  /* Boot loader 2 */
	  putString ("2");
	  asm volatile ("jmp @r0; nop"
			: : "z" (CONFIG_BOOT_LOADER2));
	  break;
#endif
	default:
	  putString ("\n");
	  break;
	}
    }

  asm volatile ("jmp @r0; nop"
		: : "z" (start));
}

static void
serial_output(const char *p, unsigned int len)
{
  if (ingdbmode)
    {
      /* encode the output in gdb protocol & wait for gdb response */
      remcomOutBuffer[0] = 'O';
      mem2hex (p, remcomOutBuffer+1, len);
      putpacket (remcomOutBuffer);
    }
  else
    {
      /* send the output directly to the serial port */
      while (len--)
	{
	  if (*p == '\n')
	    putDebugChar ('\r');
	  putDebugChar (*p++);
	}
    }
}

/* 
   R0: Function Number
   R4-R7: Input Arguments
   R0: Return value
 */
void
handle_bios_call (void)
{
  unsigned int func = registers[R0];
  unsigned int arg0 = registers[R4];
  unsigned int arg1 = registers[R5];
  unsigned int arg2 = registers[R6];
  unsigned int arg3 = registers[R7];
  int ret;
  char ch;

  /* When this is called, R0 will contain the BIOS function, and
   * R4 through R7 the arguments.
   */
  switch (func)
    {
    case 0:
      /* Serial output */
      serial_output((char *)arg0, arg1);
      ret = 0;
      break;

    case 1:
      /* Serial input */
      ret = getDebugCharTimeout ((int)arg0);
      break;

    case 2:
      /* Second storage access */
#if defined(CONFIG_IDE)
      ret = ide_read_sectors ((int) arg0,
			      (unsigned long) arg1,
			      (unsigned char *) arg2,
			      (int) arg3);
#else
      ret = -1;
#endif
      break;

    case 3:
      { /* Feature query */
	/*
	  b15-b8: Machine #
	       0: Unknown
	       1: CqREEK with CQ IDE/ISA Bridge
	       2: Reserved
	       3: SolutionEngine
	       4: CAT68701
	       ...
	  b7:    RESERVED
	  b6:    RESERVED
	  b5:    RESERVED
	  b4:    RESERVED
	  b3:    Heartbeat LED support 1: exists 0: none
	  b2-b0: Serial interface type
	       0: SCI
	       1: SCIF
	       2: IRDA
	       3: Reserved
	       4: PC style serial (? not yet)
	       ...
	 */
	int feature;
	int machine_type = 0;
	int serial_type = 0;

#if defined(CONFIG_CQ_BRIDGE)
	machine_type = 1;
#elif defined(CONFIG_SOLUTION_ENGINE)
	machine_type = 3;
#elif defined(CONFIG_CAT68701)
	machine_type = 4;
#endif

#if defined(CONFIG_SCI)
	serial_type = 0;
#elif defined(CONFIG_SCIF)
	serial_type = 1;
#endif

	feature = (machine_type << 8) | serial_type;

	ret = feature;
      }
      break;
#if defined(CONFIG_MEMORY_SIZE)
    case 4:
#if defined(CONFIG_CAT68701)
      ret = cat68701_read_mem_size();
      break;
#else
      /* Memory size in bytes */
      ret = CONFIG_MEMORY_SIZE;
      break;
#endif
#endif
#if defined(CONFIG_IO_BASE)
    case 5:
      /* IO Base */
      ret = CONFIG_IO_BASE;
      break;
#endif
    case 6:
      /* Cache control */
      ret = cache_control (arg0);
      break;
#if defined(CONFIG_ETHERNET)
    case 7:
      /* Ethernet: RESET */
      ret = eth_reset (arg0);
      break;
    case 8:
      /* Ethernet: RECEIVE */
      ret = eth_receive ((char *)arg0, (unsigned int *)arg1);
      break;
    case 9:
      /* Ethernet: TRANSMIT */
      ret = eth_transmit ((const char *)arg0, arg1, arg2, (const char *)arg3);
      break;
    case 10:
      /* Ethernet: NODE Address */
      ret = eth_node_addr (arg0, (char *)arg1);
      break;
#endif
    case 11:
      /* Shutdown (Run Monitor again, Reboot, Powerdown (if supported)... */
      ret = shutdown (arg0);
      break;
    case 12:
      /* RTC handling: Initialization, start/stop, set/get, sleep */
      ret = rtc (arg0, arg1);
      break;

    case 31:
      /* Serial output of one char:  Emits the lowest 8 bits of R4 */
      ch = arg0 & 0xff; /* Take the lower bits of R4 */
      serial_output(&ch, 1);
      ret = 0;
      break;

    case 32:
      /* Serial string output */
      {
	char *p = (char *)arg0;
	serial_output(p, strlen (p));
	ret = 0;
	break;
      }

    case 254:	/* Query gdb mode variable */
      /*
       * Return a *pointer* to the stub's ingdbmode variable.
       * This allows the kernel to subsequently check that variable
       * efficiently (which it needs to do for every character
       * written to the console).
       */
      ret = (int)&ingdbmode;
      break;

    case 255:
      /* Detach gdb mode */
      if (ingdbmode)
	{
	  ingdbmode = 0;
	  /*
	   * Tell gdb on the host that the inferior exited with status 0
	   * which is not the same as a real detach but as close as we can get.
	   */
	  putpacket ("W00");
	  /* Wait for user to type a character at terminal program on host. */
	  getDebugChar ();
	}
      ret = 0;
      break;

    default:
      /* Do nothing */
      ret = -1;
      break;
    }

  registers[R0] = (unsigned int)ret;
}

void
printouthex(int x)
{
  remcomOutBuffer[0] = highhex (x);
  remcomOutBuffer[1] = lowhex (x);
  remcomOutBuffer[2] = ' ';
  remcomOutBuffer[3] = '\n';
  remcomOutBuffer[4] = '\0';
  putString (remcomOutBuffer);
}

void
printouthex32(unsigned int x)
{
  int i;
  const char* hexdigits = "0123456789abcdef";
  for (i=0; i<8; i++)
    {
      remcomOutBuffer[i] = hexdigits[x >> 28];
      x<<=4;
    }
  remcomOutBuffer[9] = '\0';
  putString (remcomOutBuffer);
}

static inline void
set_BL (void)
{
  unsigned long __dummy;

  asm volatile ("stc	sr, %0\n\t"
		 "or	%1, %0\n\t"
		 "ldc	%0, sr"
		: "=&r" (__dummy)
		: "r" (0x10000000)
		: "memory");
}

static inline void
set_RB (void)
{
  unsigned long __dummy;

  asm volatile ("stc	sr, %0\n\t"
		"or	%1, %0\n\t"
		"ldc	%0, sr"
		: "=&r" (__dummy)
		: "r" (0x20000000)
		: "memory");
}



static void disable_MMU (void);
static void reset_interrupt_request_sources (void);

static int
shutdown (unsigned int func)
{
  extern void start ();		/* In entry.S */
  unsigned long __dummy;

  set_BL ();
  disable_MMU ();
  reset_interrupt_request_sources ();	/* Stop timer and others  */
  /* If possible, put RESET signal down to cause hardware reset */
  /* If possible, we need reset all: bus, modules, and the peripherals */
  /* XXX: Reset all variables and memory... */
  switch (func)
    {
    default:
    case 0:			/* HALT */
      putString("halted.\n");
      asm volatile ("1: sleep; bra 1b; nop");
      break;
    case 1:			/* REBOOT */
      putString("restart.\n");
      /* Cause address error with BL=1 */
      asm volatile ("1: mov.l @%1,%0; bra 1b; nop"
		    : "=r" (__dummy) : "r" (0x80000001));
      break;
    /* ACPI support? */
    }

  /* Never reached */ 
  return 0;
}

#if defined(__sh3__)
#define MMUCR		0xFFFFFFE0	/* MMU Control Register */
#define MMU_CONTROL_INIT_DISABLE	0x006	/* SV=0, TF=1, IX=1, AT=0 */
#elif defined(__SH4__)
#define MMUCR		0xFF000010	/* MMU Control Register */
#define MMU_CONTROL_INIT_DISABLE	0x204	/* SQMD=1, SV=0, TI=1, AT=0 */
#endif

static void
disable_MMU (void)
{
  p4_outl (MMUCR, MMU_CONTROL_INIT_DISABLE);
}

#if defined(__sh3__)
#define TMU_TSTR	0xfffffe92	/* Byte access */
#elif defined(__SH4__)
#define TMU_TSTR	0xffd80004	/* Byte access */
#endif

static void
reset_interrupt_request_sources (void)
{
  /* Stop the timer */
  p4_outb(TMU_TSTR, 0);

  /* ... and others */
}

#if defined(__sh3__)
#define R64CNT  	0xfffffec0
#define RSECCNT 	0xfffffec2
#define RMINCNT 	0xfffffec4
#define RHRCNT  	0xfffffec6
#define RCR1    	0xfffffedc

#define RTC_BIT_CHANGE 0x00 /* No bug */
#elif defined(__SH4__)
#define R64CNT  	0xffc80000
#define RSECCNT 	0xffc80004
#define RMINCNT 	0xffc80008
#define RHRCNT  	0xffc8000c
#define RCR1    	0xffc80038

#define RTC_BIT_CHANGE 0x40	/* We have bug on SH-4 */
#endif

#define RCR1_CF		0x80	/* Carry Flag             */

void
sleep128 (unsigned int count)
{
  unsigned int cur128 = p4_inb (R64CNT) ^ RTC_BIT_CHANGE;
  unsigned int n = (cur128 + count) % 128;
  unsigned int m = (cur128 + count) / 128;

  if (count == 0)
    return;

  while (m != 0)
    {
      /* Wait next one tick */
      while ((p4_inb (R64CNT) ^ RTC_BIT_CHANGE) == cur128)
	/* Do nothing */
	asm volatile ("" : : :"memory");

      m--;

      /* Wait next 127 ticks */
      while ((p4_inb (R64CNT) ^ RTC_BIT_CHANGE) != cur128)
	/* Do nothing */
	asm volatile ("" : : :"memory");
    }

  while ((p4_inb (R64CNT) ^ RTC_BIT_CHANGE) != n)
    /* Do nothing */
    asm volatile ("" : : :"memory");
}

/*
 * Unit is 1/128 sec.
 * It doesn't overflow within a day = 24 * 60 * 60 * 128
 */
static unsigned long tick;

#define BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

static unsigned long get_tick_1 (void)
{
  unsigned int cnt128, sec, min, hour;

  while (1)
    {
      p4_outb (RCR1, 0);  /* Clear CF-bit */

      cnt128 = (p4_inb (R64CNT) ^ RTC_BIT_CHANGE);
#if RTC_BIT_CHANGE != 0
      /* RCR1_CF doesn't work well. */
      if (cnt128 == 0)
	continue;
#endif

      sec = p4_inb (RSECCNT);
      min = p4_inb (RMINCNT);
      hour = p4_inb (RHRCNT);

      if ((p4_inb (RCR1) & RCR1_CF) == 0)
	break;
    }

  BCD_TO_BIN(sec);
  BCD_TO_BIN(min);
  BCD_TO_BIN(hour);

  return cnt128 + 128*(sec+60*(min+60*hour));
}

void reset_tick (void)
{
  tick = get_tick_1 ();
}

static int rtc_error;
static unsigned long last_tick;

unsigned long get_tick (void)
{
  unsigned long raw;

  raw = get_tick_1 ();

  /* This bug was hit by SH-4's RTC */
  if (raw < last_tick && last_tick-raw < 128)
    {
      if (rtc_error == 0)
	{
	  putString("Time goes backward!  RTC Problem, work around...OK\n");
	  rtc_error++;
	}

      /* Work around */
      raw = last_tick;
    }

  last_tick = raw;

  if (raw + (24*60*60*128/4) < tick)		/* Wrap around */
    return 24*60*60*128+raw-tick; 
  else
    return raw-tick;
}

/* XXX: This will be hardware dependent... */ 
static int
rtc (unsigned int func, unsigned int arg)
{
  switch (func)
    {
    case 0:	/* Initialize & start */
      return 0;			/* Be called, but not supported yet */

    case 1:	/* Stop */
    case 2:	/* Set */
    case 3:     /* Get */
      return -1;		/* Not supported yet */

    case 4:	/* Sleep128 */
      sleep128 (arg);
      return 0;

    case 5:
      reset_tick ();
      return 0;
    case 6:
      return get_tick ();

    default:
      return -1;
    }
}
