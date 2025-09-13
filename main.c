/* $Id: main.c,v 1.4 2005/01/24 01:27:37 doyu Exp $
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
#include "init.h"


extern void start(void);
static int shutdown(unsigned int);

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
    __attribute__ ((section(".stack"), unused));
int stub_stack[stub_stack_size]
    __attribute__ ((section(".stack")));

int *stub_sp;

static const char *const banner = "\n"
    "SH IPL+g version 0.13, Copyright (C) 2001 Free Software Foundation, Inc.\n"
    "\n"
    "This software comes with ABSOLUTELY NO WARRANTY; for details type `w'.\n"
    "This is free software, and you are welcome to redistribute it under\n"
    "certain conditions; type `l' for details.\n\n";

static const char *const license_message = "\n"
    "SH IPL+g is free software; you can redistribute it and/or modify it under\n"
    "the terms of the GNU Lesser General Public License as published by \n"
    "the Free Software Foundatin; either version 2.1 of the License, or (at your\n"
    "option) any later version.\n";

static const char *const warranty_message = "\n"
    "SH IPL+g is distributed in the hope that it will be useful, but\n"
    "WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
    "Lesser General Public License for more details.\n";

static const char *const help_message = "\n"
    "SH IPL+g version 0.11, Copyright (C) 2001 Free Software Foundation, Inc.\n"
    "    ? --- Show this message (HELP)\n"
    "    b --- Boot the system\n"
    "    g --- Invoke GDB stub\n"
    "    l --- Show about license\n" "    w --- Show about (no)warranty\n\n"
#if defined(CONFIG_ETHERNET)
    "    e --- Ether Boot\n"
#endif
#if defined(CONFIG_BOOT_LOADER1)
    "    1 --- Boot loader 1\n"
#endif
#if defined(CONFIG_BOOT_LOADER2)
    "    2 --- Boot loader 2\n"
#endif
    ;				/* Nasty dangling ; for initialisation - don't delete! */

static const char *prompt = "> ";
int expevt_on_start;

#if defined(CONFIG_IDE)
static void boot(void)
{
	register long __sc0 __asm__("r0") = 2;	/* READ SECTORS */
	register long __sc4 __asm__("r4") = (long)0;
	register long __sc5 __asm__("r5") = (long)0;
	register long __sc6 __asm__("r6") = (long)CONFIG_RAM_BOOT;
	register long __sc7 __asm__("r7") = (long)1;

	asm volatile ("trapa	#0x3F":"=z" (__sc0)
		      :"0"(__sc0), "r"(__sc4), "r"(__sc5), "r"(__sc6),
		      "r"(__sc7)
		      :"memory");

	if (__sc0 < 0)
		printf("MBR read error\n");
	else
		asm volatile ("jmp @r0; nop"::"z" (CONFIG_RAM_BOOT));
}
#endif

void start_main(void)
{
	initcall_t *call;
	char c;

	in_nmi = 0;
	dofault = 1;
	stepped = 0;

	stub_sp = stub_stack + stub_stack_size;

	rtc(0, 0);

	for (call = &__initcall_start; call < &__initcall_end; call++)
		(*call) ();

	printf(banner);

	if (expevt_on_start != 0) {	/* Reboot */
		printf("!\n");
#ifdef CONFIG_IDE
		if (ide_detect_devices() == 0) {
			sti();
			ide_startup_devices();
			boot();
		}
#endif
#ifdef CONFIG_ETHERNET
		sti();
		etherboot();
#endif
#ifdef CONFIG_BOOT_LOADER1
		sti();
		asm volatile ("jmp @r0; nop"::"z" (CONFIG_BOOT_LOADER1));
#endif
	}

	while (1) {
		printf(prompt);
		c = getDebugChar();
		switch (c) {
		case '?':
		case 'h':
			/* Help */
			printf("%c %s\n", c, help_message);
			break;
		case 'b':
			/* Boot */
			printf("b\n");
#if defined(CONFIG_IDE)
			if (ide_detect_devices() == 0) {
				sti();
				ide_startup_devices();
				boot();
			} else
				printf("IDE: device not detected.\n");
#else
			printf("Not Implemented Yet.\n");
#endif
			break;
		case 'l':
			/* License */
			printf("l");
			printf(license_message);
			break;
		case 'g':
		case '#':
			/* GDB Stub */
			sti();
			printf("g\n");
			breakpoint();
			break;
		case 'w':
			/* Warranty */
			printf("w");
			printf(warranty_message);
			break;
#if defined(CONFIG_ETHERNET)
		case 'e':
			/* Ether Boot */
			sti();
			printf("e");
			etherboot();
			break;
#endif
#if defined(CONFIG_BOOT_LOADER1)
		case '1':
			/* Boot loader 1 */
			sti();
			printf("1");
			asm volatile ("jmp @r0; nop"::"z"
				      (CONFIG_BOOT_LOADER1));
			break;
#endif
#if defined(CONFIG_BOOT_LOADER2)
		case '2':
			sti();
			/* Boot loader 2 */
			printf("2");
			asm volatile ("jmp @r0; nop"::"z"
				      (CONFIG_BOOT_LOADER2));
			break;
#endif
		default:
			printf("\n");
			break;
		}
	}

	asm volatile ("jmp @r0; nop"::"z" (start));
}

static void serial_output(const char *p, unsigned int len)
{
	if (ingdbmode) {
		/* encode the output in gdb protocol & wait for gdb response */
		remcomOutBuffer[0] = 'O';
		mem2hex(p, remcomOutBuffer + 1, len);
		putpacket(remcomOutBuffer);
	} else {
		/* send the output directly to the serial port */
		while (len--) {
			if (*p == '\n')
				putDebugChar('\r');
			putDebugChar(*p++);
		}
	}
}

/* 
   R0: Function Number
   R4-R7: Input Arguments
   R0: Return value
 */
void handle_bios_call(void)
{
	unsigned int func = registers[R0];
	unsigned int arg0 = registers[R4];
	unsigned int arg1 = registers[R5];
#if defined(CONFIG_IDE) || defined(CONFIG_ETHERNET)
	unsigned int arg2 = registers[R6];
	unsigned int arg3 = registers[R7];
#endif
	int ret;
	char ch;

	/* When this is called, R0 will contain the BIOS function, and
	 * R4 through R7 the arguments.
	 */
	switch (func) {
	case 0:
		/* Serial output */
		serial_output((char *)arg0, arg1);
		ret = 0;
		break;

	case 1:
		/* Serial input */
		ret = getDebugCharTimeout((int)arg0);
		break;

	case 2:
		/* Second storage access */
#if defined(CONFIG_IDE)
		ret = ide_read_sectors((int)arg0,
				       (unsigned long)arg1,
				       (unsigned char *)arg2, (int)arg3);
#else
		ret = -1;
#endif
		break;

	case 3:
		{		/* Feature query */
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
		ret = cache_control(arg0);
		break;
#if defined(CONFIG_ETHERNET)
	case 7:
		/* Ethernet: RESET */
		ret = eth_reset(arg0);
		break;
	case 8:
		/* Ethernet: RECEIVE */
		ret = eth_receive((char *)arg0, (unsigned int *)arg1);
		break;
	case 9:
		/* Ethernet: TRANSMIT */
		ret =
		    eth_transmit((const char *)arg0, arg1, arg2,
				 (const char *)arg3);
		break;
	case 10:
		/* Ethernet: NODE Address */
		ret = eth_node_addr(arg0, (char *)arg1);
		break;
#endif
	case 11:
		/* Shutdown (Run Monitor again, Reboot, Powerdown (if supported)... */
		ret = shutdown(arg0);
		break;

	case 12:
		/* RTC handling: Initialization, start/stop, set/get, sleep */
		ret = rtc(arg0, arg1);
		break;

	case 31:
		/* Serial output of one char:  Emits the lowest 8 bits of R4 */
		ch = arg0 & 0xff;	/* Take the lower bits of R4 */
		serial_output(&ch, 1);
		ret = 0;
		break;

	case 32:
		/* Serial string output */
		{
			char *p = (char *)arg0;
			serial_output(p, strlen(p));
			ret = 0;
			break;
		}

	case 254:		/* Query gdb mode variable */
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
		if (ingdbmode) {
			ingdbmode = 0;
			/*
			 * Tell gdb on the host that the inferior exited with status 0
			 * which is not the same as a real detach but as close as we can get.
			 */
			putpacket("W00");
			/* Wait for user to type a character at terminal program on host. */
			getDebugChar();
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

void printouthex(int x)
{
	remcomOutBuffer[0] = highhex(x);
	remcomOutBuffer[1] = lowhex(x);
	remcomOutBuffer[2] = ' ';
	remcomOutBuffer[3] = '\n';
	remcomOutBuffer[4] = '\0';
	printf(remcomOutBuffer);
}

void printouthex32(unsigned int x)
{
	int i;
	const char *hexdigits = "0123456789abcdef";
	for (i = 0; i < 8; i++) {
		remcomOutBuffer[i] = hexdigits[x >> 28];
		x <<= 4;
	}
	remcomOutBuffer[9] = '\0';
	printf(remcomOutBuffer);
}

static inline void set_BL(void)
{
	unsigned long __dummy;

	asm volatile ("stc	sr, %0\n\t"
		      "or	%1, %0\n\t" "ldc	%0, sr":"=&r" (__dummy)
		      :"r"(0x10000000)
		      :"memory");
}

static inline void set_RB(void)
{
	unsigned long __dummy;

	asm volatile ("stc	sr, %0\n\t"
		      "or	%1, %0\n\t" "ldc	%0, sr":"=&r" (__dummy)
		      :"r"(0x20000000)
		      :"memory");
}

static void disable_MMU(void);
static void reset_interrupt_request_sources(void);

static int shutdown(unsigned int func)
{
	unsigned long __dummy;

	set_BL();
	disable_MMU();
	reset_interrupt_request_sources();	/* Stop timer and others  */
	/* If possible, put RESET signal down to cause hardware reset */
	/* If possible, we need reset all: bus, modules, and the peripherals */
	/* XXX: Reset all variables and memory... */
	switch (func) {
	default:
	case 0:		/* HALT */
		printf("halted.\n");
		asm volatile ("1: sleep; bra 1b; nop");
		break;
	case 1:		/* REBOOT */
		printf("restart.\n");
		/* Cause address error with BL=1 */
		asm volatile ("1: mov.l @%1,%0; bra 1b; nop":"=r" (__dummy):"r"
			      (0x80000001));
		break;
		/* ACPI support? */
	}

	/* Never reached */
	return 0;
}

#if defined(CONFIG_CPU_SH3)
#define MMUCR		0xFFFFFFE0	/* MMU Control Register */
#define MMU_CONTROL_INIT_DISABLE	0x006	/* SV=0, TF=1, IX=1, AT=0 */
#elif defined(CONFIG_CPU_SH4)
#define MMUCR		0xFF000010	/* MMU Control Register */
#define MMU_CONTROL_INIT_DISABLE	0x204	/* SQMD=1, SV=0, TI=1, AT=0 */
#endif

static void disable_MMU(void)
{
#ifdef MMUCR
	p4_outl(MMUCR, MMU_CONTROL_INIT_DISABLE);
#endif
}

#if defined(CONFIG_CPU_SH2)
#define CMT_CMSTR	0xffff83d0	/* Word access */
#elif defined(CONFIG_CPU_SH3)
#define TMU_TSTR	0xfffffe92	/* Byte access */
#elif defined(CONFIG_CPU_SH4)
#define TMU_TSTR	0xffd80004	/* Byte access */
#endif

static void reset_interrupt_request_sources(void)
{
	/* Stop the timer */
#ifdef CMT_CMSTR
	p4_outw(CMT_CMSTR, 0);
#endif
#ifdef TMU_TSTR
	p4_outb(TMU_TSTR, 0);
#endif
	/* ... and others */
}
