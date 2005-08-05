/* $Id: sh-stub.c,v 1.38 2001/06/04 09:09:19 gniibe Exp $
 *
 * gdb-sh-stub/sh-stub.c -- debugging stub for the Hitachi-SH. 
 * Based on sh-stub.c distributed with GDB-4.18.
 */

/*   This is originally based on an m68k software stub written by Glenn
     Engel at HP, but has changed quite a bit. 

     Modifications for the SH by Ben Lee and Steve Chamberlain

*/

/****************************************************************************

		THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or it's performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/* Remote communication protocol.

   A debug packet whose contents are <data>
   is encapsulated for transmission in the form:

	$ <data> # CSUM1 CSUM2

	<data> must be ASCII alphanumeric and cannot include characters
	'$' or '#'.  If <data> starts with two characters followed by
	':', then the existing stubs interpret this as a sequence number.

	CSUM1 and CSUM2 are ascii hex representation of an 8-bit 
	checksum of <data>, the most significant nibble is sent first.
	the hex digits 0-9,a-f are used.

   Receiver responds with:

	+	- if CSUM is correct and ready for next packet
	-	- if CSUM is incorrect

   <data> is as follows:
   All values are encoded in ascii hex digits.

	Request		Packet

	read registers  g
	reply		XX....X		Each byte of register data
					is described by two hex digits.
					Registers are in the internal order
					for GDB, and the bytes in a register
					are in the same order the machine uses.
			or ENN		for an error.

	write regs	GXX..XX		Each byte of register data
					is described by two hex digits.
	reply		OK		for success
			ENN		for an error

        write reg	Pn...=r...	Write register n... with value r...,
					which contains two hex digits for each
					byte in the register (target byte
					order).
	reply		OK		for success
			ENN		for an error
	(not supported by all stubs).

	read mem	mAA..AA,LLLL	AA..AA is address, LLLL is length.
	reply		XX..XX		XX..XX is mem contents
					Can be fewer bytes than requested
					if able to read only part of the data.
			or ENN		NN is errno

	write mem	MAA..AA,LLLL:XX..XX
					AA..AA is address,
					LLLL is number of bytes,
					XX..XX is data
	reply		OK		for success
			ENN		for an error (this includes the case
					where only part of the data was
					written).

	write mem	XAA..AA,LLLL:XX..XX
	 (binary)			AA..AA is address,
					LLLL is number of bytes,
					XX..XX is binary data
	reply		OK		for success
			ENN		for an error

	cont		cAA..AA		AA..AA is address to resume
					If AA..AA is omitted,
					resume at same address.

	step		sAA..AA		AA..AA is address to resume
					If AA..AA is omitted,
					resume at same address.

	last signal     ?               Reply the current reason for stopping.
                                        This is the same reply as is generated
					for step or cont : SAA where AA is the
					signal number.

	There is no immediate reply to step or cont.
	The reply comes when the machine stops.
	It is		SAA		AA is the "signal number"

	or...		TAAn...:r...;n:r...;n...:r...;
					AA = signal number
					n... = register number
					r... = register contents
	or...		WAA		The process exited, and AA is
					the exit status.  This is only
					applicable for certains sorts of
					targets.
	kill request	k

	toggle debug	d		toggle debug flag (see 386 & 68k stubs)
	reset		r		reset -- see sparc stub.
	reserved	<other>		On other requests, the stub should
					ignore the request and send an empty
					response ($#<checksum>).  This way
					we can extend the protocol and GDB
					can tell whether the stub it is
					talking to uses the old or the new.
	search		tAA:PP,MM	Search backwards starting at address
					AA for a match with pattern PP and
					mask MM.  PP and MM are 4 bytes.
					Not supported by all stubs.

	general query	qXXXX		Request info about XXXX.
	general set	QXXXX=yyyy	Set value of XXXX to yyyy.
	query sect offs	qOffsets	Get section offsets.  Reply is
					Text=xxx;Data=yyy;Bss=zzz
	console output	Otext		Send text to stdout.  Only comes from
					remote target.

	Responses can be run-length encoded to save space.  A '*' means that
	the next character is an ASCII encoding giving a repeat count which
	stands for that many repititions of the character preceding the '*'.
	The encoding is n+29, yielding a printable character where n >=3 
	(which is where rle starts to win).  Don't use an n > 126. 

	So 
	"0* " means the same as "0000".  */

#include "config.h"
#include "defs.h"
#include "string.h"
#include "setjmp.h"

#define COND_BR_MASK		0xff00
#define UCOND_DBR_MASK		0xe000
#define UCOND_RBR_MASK		0xf0df
#define TRAPA_MASK		0xff00

#define COND_DISP		0x00ff
#define UCOND_DISP		0x0fff
#define UCOND_REG		0x0f00

#define BF_INSTR		0x8b00
#define BT_INSTR		0x8900
#define BFS_INSTR		0x8f00
#define BTS_INSTR		0x8d00
#define BRA_INSTR		0xa000
#define BRAF_INSTR		0x0023
#define BSRF_INSTR		0x0003
#define BSR_INSTR		0xb000
#define JMP_INSTR		0x402b
#define JSR_INSTR		0x400b
#define RTS_INSTR		0x000b
#define RTE_INSTR		0x002b
#define TRAPA_INSTR		0xc300

#define SSTEP_TRAP		0x20
#define BIOS_CALL_TRAP		0x3f

#ifdef CONFIG_CPU_SH2
#define BREAK_TRAP		32
#else
#define BREAK_TRAP		0xff
#endif

#define SSTEP_INSTR		(TRAPA_INSTR | SSTEP_TRAP)

#define T_BIT_MASK		0x0001

/*
 * Forward declarations
 */

static int hex(char);
static char *hex2mem(const char *, char *, int);
static char *ebin2mem(const char *, char *, int);
static int hexToInt(char **, int *);
static void getpacket(char *);
/* static void putpacket (char *); */
static int computeSignal(int exceptionVector);

#ifdef CONFIG_SESH4
static unsigned char load_led_value;
static void leds(unsigned char);
#endif

/* When you link take care that this is at address 0 -
   or wherever your vbr points */

#ifdef CONFIG_CPU_SH2
#define INVALID_INSN_VEC	 4
#define INVALID_SLOT_VEC	 6
#define ADDRESS_ERROR_VEC	 9
#define DMAC_ADDRESS_ERROR_VEC	10
#define NMI_VEC			11
#define USER_BREAK_VEC		12
#define TRAP_VEC_BEGIN		32
#define TRAP_VEC_END		63
#else
#define ADDRESS_ERROR_LOAD_VEC   7
#define ADDRESS_ERROR_STORE_VEC  8
#define TRAP_VEC          	11
#define INVALID_INSN_VEC	12
#define INVALID_SLOT_VEC	13
#define NMI_VEC			14
#define USER_BREAK_VEC		15
#define SERIAL_BREAK_VEC	58
#endif

char in_nmi;			/* Set when handling an NMI, so we don't reenter */
int dofault;			/* Non zero, bus errors will raise exception */
char stepped;

/* debug > 0 prints ill-formed commands in valid packets & checksum errors */
static int remote_debug;

/* jump buffer used for setjmp/longjmp */
static jmp_buf remcomEnv;

typedef struct {
	short *memAddr;
	short oldInstr;
} stepData;

unsigned int registers[NUMREGBYTES / 4];
static stepData instrBuffer;
static const char hexchars[] = "0123456789abcdef";
char remcomInBuffer[BUFMAX];
char remcomOutBuffer[OUTBUFMAX];

char highhex(int x)
{
	return hexchars[(x >> 4) & 0xf];
}

char lowhex(int x)
{
	return hexchars[x & 0xf];
}

/*
 * Routines to handle hex data
 */

static int hex(char ch)
{
	if ((ch >= 'a') && (ch <= 'f'))
		return (ch - 'a' + 10);
	if ((ch >= '0') && (ch <= '9'))
		return (ch - '0');
	if ((ch >= 'A') && (ch <= 'F'))
		return (ch - 'A' + 10);
	return (-1);
}

/* convert the memory, pointed to by mem into hex, placing result in buf */
/* return a pointer to the last char put in buf (null) */
char *mem2hex(const char *mem, char *buf, int count)
{
	int i;
	int ch;
	unsigned short s_val;
	unsigned long l_val;

	if (count == 2 && ((long)mem & 1) == 0) {
		s_val = *(unsigned short *)mem;
		mem = (char *)&s_val;
	} else if (count == 4 && ((long)mem & 3) == 0) {
		l_val = *(unsigned long *)mem;
		mem = (char *)&l_val;
	}
	for (i = 0; i < count; i++) {
		ch = *mem++;
		*buf++ = highhex(ch);
		*buf++ = lowhex(ch);
	}
	*buf = 0;
	return (buf);
}

/* convert the hex array pointed to by buf into binary, to be placed in mem */
/* return a pointer to the character after the last byte written */

static char *hex2mem(const char *buf, char *mem, int count)
{
	int i;
	unsigned char ch;
	for (i = 0; i < count; i++) {
		ch = hex(*buf++) << 4;
		ch = ch + hex(*buf++);
		*mem++ = ch;
	}
	return (mem);
}

/*
 * Convert the escaped-binary array pointed to by buf into binary, to
 * be placed in mem. Return a pointer to the character after the last
 * byte written.
 */

static char *ebin2mem(const char *buf, char *mem, int count)
{
	for (; count > 0; count--, buf++) {
		if (*buf == 0x7d)
			*mem++ = *(++buf) ^ 0x20;
		else
			*mem++ = *buf;
	}
	return (mem);
}

/**********************************************/
/* WHILE WE FIND NICE HEX CHARS, BUILD AN INT */
/* RETURN NUMBER OF CHARS PROCESSED           */
/**********************************************/
static int hexToInt(char **ptr, int *intValue)
{
	int numChars = 0;
	int hexValue;

	*intValue = 0;

	while (**ptr) {
		hexValue = hex(**ptr);
		if (hexValue >= 0) {
			*intValue = (*intValue << 4) | hexValue;
			numChars++;
		} else
			break;

		(*ptr)++;
	}

	return (numChars);
}

/*
 * Routines to get and put packets
 */

/* scan for the sequence $<data>#<checksum>     */

static void getpacket(char *buffer)
{
	unsigned char checksum;
	unsigned char xmitcsum;
	int i;
	int count;
	char ch;
	do {
		/* wait around for the start character, ignore all other characters */
		while ((ch = getDebugChar()) != '$') ;
		checksum = 0;
		xmitcsum = -1;

		count = 0;

		/* now, read until a # or end of buffer is found */
		while (count < BUFMAX) {
			ch = getDebugChar();
			if (ch == '#')
				break;
			checksum = checksum + ch;
			buffer[count] = ch;
			count = count + 1;
		}
		buffer[count] = 0;

		if (ch == '#') {
			xmitcsum = hex(getDebugChar()) << 4;
			xmitcsum += hex(getDebugChar());
			if (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			else {
				putDebugChar('+');	/* successful transfer */
				/* if a sequence char is present, reply the sequence ID */
				if (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					count = strlen(buffer);
					for (i = 3; i <= count; i++)
						buffer[i - 3] = buffer[i];
				}
			}
		}
	}
	while (checksum != xmitcsum);

}

/* send the packet in buffer.  The host get's one chance to read it.
   This routine does not wait for a positive acknowledge.  */

void putpacket(register char *buffer)
{
	register int checksum;

	/*  $<packet info>#<checksum>. */
	do {
		char *src = buffer;
		putDebugChar('$');
		checksum = 0;

		while (*src) {
			int runlen;

			/* Do run length encoding */
			for (runlen = 0; runlen < 100; runlen++) {
				if (src[0] != src[runlen] || runlen == 99) {
					if (runlen > 3) {
						int encode;
						/* Got a useful amount */
						putDebugChar(*src);
						checksum += *src;
						putDebugChar('*');
						checksum += '*';
						checksum += (encode =
							     runlen + ' ' - 4);
						putDebugChar(encode);
						src += runlen;
					} else {
						putDebugChar(*src);
						checksum += *src;
						src++;
					}
					break;
				}
			}
		}

		putDebugChar('#');
		putDebugChar(highhex(checksum));
		putDebugChar(lowhex(checksum));
	}
	while (getDebugChar() != '+');

}

/* a bus error has occurred, perform a longjmp
   to return execution and allow handling of the error */

void handle_buserror(void)
{
	longjmp(remcomEnv, 1);
}

/*
 * this function takes the SH-3/SH-4 exception number and attempts to
 * translate this number into a unix compatible signal value
 */
static int computeSignal(int exceptionVector)
{
	int sigval;
	switch (exceptionVector) {
	case INVALID_INSN_VEC:
	case INVALID_SLOT_VEC:
		sigval = 4;	/* SIGILL */
		break;
#ifdef CONFIG_CPU_SH2
	case ADDRESS_ERROR_VEC:
#else
	case ADDRESS_ERROR_LOAD_VEC:
	case ADDRESS_ERROR_STORE_VEC:
#endif
		sigval = 10;	/* SIGSEGV is 11??? */
		break;
#ifndef CONFIG_CPU_SH2
	case SERIAL_BREAK_VEC:
#endif
	case NMI_VEC:
		sigval = 2;	/* SIGINT */
		break;

	case USER_BREAK_VEC:
#ifdef CONFIG_CPU_SH2
	case TRAP_VEC_BEGIN ... TRAP_VEC_END:
#else
	case TRAP_VEC:
#endif
		sigval = 5;	/* SIGTRAP */
		break;

	default:
		sigval = 7;	/* "software generated" */
		break;
	}
	return (sigval);
}

static inline unsigned int ctrl_inl(unsigned long addr)
{
	return *(volatile unsigned long *)addr;
}

static inline void ctrl_outl(unsigned int b, unsigned long addr)
{
	*(volatile unsigned long *)addr = b;
}

/*
 * Jump to P2 area.
 * When handling TLB or caches, we need to do it from P2 area.
 */
#define jump_to_P2()			\
do {					\
	unsigned long __dummy;		\
	__asm__ __volatile__(		\
		"mov.l	1f, %0\n\t"	\
		"or	%1, %0\n\t"	\
		"jmp	@%0\n\t"	\
		" nop\n\t" 		\
		".balign 4\n"		\
		"1:	.long 2f\n"	\
		"2:"			\
		: "=&r" (__dummy)	\
		: "r" (0x20000000));	\
} while (0)

/*
 * Back to P1 area.
 */
#define back_to_P1()					\
do {							\
	unsigned long __dummy;				\
	__asm__ __volatile__(				\
		"nop;nop;nop;nop;nop;nop;nop\n\t"	\
		"mov.l	1f, %0\n\t"			\
		"jmp	@%0\n\t"			\
		" nop\n\t"				\
		".balign 4\n"				\
		"1:	.long 2f\n"			\
		"2:"					\
		: "=&r" (__dummy));			\
} while (0)

#if defined(CONFIG_CPU_SH2)
#define CCR		 0xffff8740
#define CCR_CACHE_INIT   0x0000001f	/* CS0=1,CS1=1,CS2=1,CS3=1,DRAM=0 */
#define CCR_CACHE_STOP	 0x00000000
#define CCR_CACHE_ENABLE CCR_CACHE_INIT

#define flush_icache_range(start,end)	do {} while(0)

#define CACHE_OC_ADDRESS_ARRAY	0xfffff000
#define CACHE_OC_WAY_SHIFT	0	/* No way specification */
#define CACHE_OC_NUM_ENTRIES	256
#define CACHE_OC_ENTRY_SHIFT	2
#define CACHE_OC_NUM_WAYS	1
#elif defined(CONFIG_CPU_SH3)
#define CCR		 0xffffffec
#define CCR_CACHE_INIT	 0x0000000d	/* 8k-byte cache, CF, P1-wb, enable */
#define CCR_CACHE_STOP	 0x00000008
#define CCR_CACHE_ENABLE 0x00000001

#define flush_icache_range(start,end)	do {} while(0)

/* SH7707, SH7708, SH7709 has less cache than SH7709A,
   but it's OK to have bigger value. */
#define CACHE_OC_ADDRESS_ARRAY	0xf0000000
#define CACHE_OC_WAY_SHIFT	12	/*  11 */
#define CACHE_OC_NUM_ENTRIES	256	/* 128 */
#define CACHE_OC_ENTRY_SHIFT    4
#define CACHE_OC_NUM_WAYS	4

#elif defined(CONFIG_CPU_SH4)
#define CCR		 0xff00001c
#ifdef CONFIG_CPU_SUBTYPE_SH_R
#define CCR_CACHE_INIT	 0x8000090d	/* EMODE,ICI,ICE(16k),OCI,P1-wb,OCE(32k) */
#else
#define CCR_CACHE_INIT	 0x0000090d	/* ICI,ICE(8k), OCI,P1-wb,OCE(16k) */
#endif
#define CCR_CACHE_STOP	 0x00000808
#define CCR_CACHE_ENABLE 0x00000101
#define CCR_CACHE_ICI	 0x00000800

#define L1_CACHE_BYTES 32
#define CACHE_IC_ADDRESS_ARRAY	0xf0000000
#define CACHE_IC_ENTRY_MASK	0x1fe0

#define CACHE_OC_ADDRESS_ARRAY	0xf4000000
#define CACHE_OC_WAY_SHIFT       14
#define CACHE_OC_NUM_ENTRIES	512
#define CACHE_OC_ENTRY_SHIFT      5
#ifdef CONFIG_CPU_SUBTYPE_SH_R
#define CACHE_OC_NUM_WAYS	  2
#elif defined(CONFIG_CPU_SUBTYPE_SH73180)
#define CACHE_OC_NUM_WAYS	  4
#else
#define CACHE_OC_NUM_WAYS	  1
#endif

/* Write back data caches, and invalidates instructiin caches */
void flush_icache_range(unsigned long start, unsigned long end)
{
	unsigned long v;

	start &= ~(L1_CACHE_BYTES - 1);

	for (v = start; v < end; v += L1_CACHE_BYTES) {
		/* Write back O Cache */
		asm volatile ("ocbwb	@%0":	/* no output */
			      :"r" (v));
	}
	/* Invalidate I Cache */
	jump_to_P2();
	ctrl_outl(ctrl_inl(CCR) | CCR_CACHE_ICI, CCR);
	back_to_P1();
}

#endif

#define CACHE_VALID	  1
#define CACHE_UPDATED	  2

static inline void cache_wback_all(void)
{
	unsigned long addr, data, i, j;

	jump_to_P2();
	for (i = 0; i < CACHE_OC_NUM_ENTRIES; i++)
		for (j = 0; j < CACHE_OC_NUM_WAYS; j++) {
#ifdef CONFIG_CPU_SH2
			addr = CACHE_OC_ADDRESS_ARRAY | (i << CACHE_OC_ENTRY_SHIFT);
#else
			addr = CACHE_OC_ADDRESS_ARRAY | (j << CACHE_OC_WAY_SHIFT)
				| (i << CACHE_OC_ENTRY_SHIFT);
#endif
			data = ctrl_inl(addr);
			if (data & CACHE_UPDATED) {
				data &= ~CACHE_UPDATED;
				ctrl_outl(data, addr);
			}
		}
	back_to_P1();
}

#define CACHE_ENABLE      0
#define CACHE_DISABLE     1

int cache_control(unsigned int command)
{
	unsigned long ccr;

	jump_to_P2();
	ccr = ctrl_inl(CCR);

	if (ccr & CCR_CACHE_ENABLE)
		cache_wback_all();

	if (command == CACHE_DISABLE)
		ctrl_outl(CCR_CACHE_STOP, CCR);
	else
		ctrl_outl(CCR_CACHE_INIT, CCR);
	back_to_P1();

	return 0;
}

/*
 * Setup a single-step.  Replace the instruction immediately
 * after (i.e. next in the expected flow of control) the current
 * instruction with a trap instruction, so that returning will cause
 * only a single instruction to be executed.
 *
 * Note that this model is slightly broken for instructions with
 * delay slots (e.g. B[TF]S, BSR, BRA etc), where both the branch
 * and the instruction in the delay slot will be executed.
 */
static void doSStep(void)
{
	short *instrMem;
	unsigned int displacement;
	int reg;
	unsigned short opcode;

	instrMem = (short *)registers[PC];

	opcode = *instrMem;
	stepped = 1;

	/*
	 * Calculate the location where the flow of control will go to when
	 * the opcode is executed, using the opcode and saved processor state.
	 * We need to deal with all flow-transfer instructions individually;
	 * other instructions are just a trivial PC increment.
	 */
	if ((opcode & COND_BR_MASK) == BT_INSTR) {	/* BT */
		if (registers[SR] & T_BIT_MASK) {	/* if T, take branch */
			/* Displacement is lower 8 bits of instruction,
			 * sign extended, and multiplied by 2 (to get bytes).
			 */
			displacement = (opcode & COND_DISP) << 1;
			if (displacement & 0x100)
				displacement |= 0xfffffe00;
			/*
			 * Remember PC points to second instr.
			 * after PC of branch ... so add 4
			 */
			instrMem = (short *)(registers[PC] + displacement + 4);
		} else		/* if !T, drop through to next instruction */
			instrMem += 1;
	} else if ((opcode & COND_BR_MASK) == BF_INSTR) {	/* BF */
		if (registers[SR] & T_BIT_MASK)	/* if T, drop through to next instruction */
			instrMem += 1;
		else {
			displacement = (opcode & COND_DISP) << 1;
			if (displacement & 0x100)
				displacement |= 0xfffffe00;
			/*
			 * Remember PC points to second instr.
			 * after PC of branch ... so add 4
			 */
			instrMem = (short *)(registers[PC] + displacement + 4);
		}
	} else if ((opcode & COND_BR_MASK) == BTS_INSTR) {	/* BTS */
		if (registers[SR] & T_BIT_MASK) {	/* if T, take branch */
			displacement = (opcode & COND_DISP) << 1;
			if (displacement & 0x100)
				displacement |= 0xfffffe00;
			/*
			 * Remember PC points to second instr.
			 * after PC of branch ... so add 4
			 */
			instrMem = (short *)(registers[PC] + displacement + 4);
		} else		/* if !T, drop through to next instruction */
			instrMem += 2;	/* We should not place trapa in the delay slot */
	} else if ((opcode & COND_BR_MASK) == BFS_INSTR) {	/* BFS */
		if (registers[SR] & T_BIT_MASK)	/* if T, drop through to next instruction */
			instrMem += 2;	/* We should not place trapa in the delay slot */
		else {		/* if !T, take branch */

			displacement = (opcode & COND_DISP) << 1;
			if (displacement & 0x100)
				displacement |= 0xfffffe00;
			/*
			 * Remember PC points to second instr.
			 * after PC of branch ... so add 4
			 */
			instrMem = (short *)(registers[PC] + displacement + 4);
		}
	} else if ((opcode & UCOND_DBR_MASK) == BRA_INSTR) {	/* BRA/BSR */
		/* Displacement is lower 12 bits of instruction,
		 * sign extended, and multiplied by 2 (to get bytes).
		 */
		displacement = (opcode & UCOND_DISP) << 1;
		if (displacement & 0x1000)
			displacement |= 0xffffe000;

		/*
		 * Remember PC points to second instr.
		 * after PC of branch ... so add 4
		 */
		instrMem = (short *)(registers[PC] + displacement + 4);
	} else if ((opcode & UCOND_RBR_MASK) == JSR_INSTR) {	/* JMP/JSR */
		/* register is bits 8-11 */
		reg = (char)((opcode & UCOND_REG) >> 8);

		instrMem = (short *)registers[reg];
	} else if ((opcode & UCOND_RBR_MASK) == BSRF_INSTR) {	/* BRAF/BSRF */
		reg = (char)((opcode & UCOND_REG) >> 8);

		instrMem = (short *)(registers[reg] + registers[PC] + 4);
	} else if (opcode == RTS_INSTR)	/* 0x000b */
		instrMem = (short *)registers[PR];
	else if (opcode == RTE_INSTR)	/* or, 0x002b ;-) */
		instrMem = (short *)registers[ /*SPC*/ 15];
#if 0				/* following code is for SH-1 */
	else if ((opcode & TRAPA_MASK) == TRAPA_INSTR)
		instrMem = (short *)((opcode & ~TRAPA_MASK) << 2);
#endif
	else
		instrMem += 1;

	/*
	 * Insert a single-step trap instruction at the calculated location.
	 */
	instrBuffer.memAddr = instrMem;
	instrBuffer.oldInstr = *instrMem;
	/* ensure the instruction executed is the one we just wrote */
	*instrMem = SSTEP_INSTR;
	flush_icache_range((unsigned long)instrMem,
			   (unsigned long)(instrMem + 1));
}

/* Undo the effect of a previous doSStep.  If we single stepped,
   restore the old instruction. */

static void undoSStep(void)
{
	if (stepped) {
		short *instrMem;
		instrMem = instrBuffer.memAddr;
		*instrMem = instrBuffer.oldInstr;
		flush_icache_range((unsigned long)instrMem,
				   (unsigned long)(instrMem + 1));
	}
	stepped = 0;
}

/*
This function does all exception handling.  It only does two things -
it figures out why it was called and tells gdb, and then it reacts
to gdb's requests.

When in the monitor mode we talk a human on the serial line rather than gdb.

*/

static void gdb_handle_exception(int exceptionVector, int trapa_value)
{
	int sigval;
	int addr, length;
	char *ptr;

	/* reply to host that an exception has occurred */
	sigval = computeSignal(exceptionVector);
	remcomOutBuffer[0] = 'S';
	remcomOutBuffer[1] = highhex(sigval);
	remcomOutBuffer[2] = lowhex(sigval);
	remcomOutBuffer[3] = 0;

	putpacket(remcomOutBuffer);

	/*
	 * TRAP_VEC exception indicates a software trap
	 * inserted in place of code ... so back up
	 * PC by one instruction, since this instruction
	 * will later be replaced by its original one!
	 */
#ifdef CONFIG_CPU_SH2
	if (exceptionVector >= TRAP_VEC_BEGIN && exceptionVector <= TRAP_VEC_END)
#else
	if (exceptionVector == TRAP_VEC && trapa_value != (0xff << 2))
#endif
		registers[PC] -= 2;

	/*
	 * Do the thangs needed to undo
	 * any stepping we may have done!
	 */
	undoSStep();

	while (1) {
		remcomOutBuffer[0] = 0;
		getpacket(remcomInBuffer);

		switch (remcomInBuffer[0]) {
		case '?':
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = highhex(sigval);
			remcomOutBuffer[2] = lowhex(sigval);
			remcomOutBuffer[3] = 0;
			break;
		case 'd':
			remote_debug = !(remote_debug);	/* toggle debug flag */
			break;
		case 'g':	/* return the value of the CPU registers */
			mem2hex((char *)registers, remcomOutBuffer,
				NUMREGBYTES);
			break;
		case 'G':	/* set the value of the CPU registers - return OK */
			hex2mem(&remcomInBuffer[1], (char *)registers,
				NUMREGBYTES);
			strcpy(remcomOutBuffer, "OK");
#if CONFIG_SESH4
			/* Bump the leds, but only if it's part of a program load */
			if (length > 32)
				leds(load_led_value++);
#endif
			break;

			/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
		case 'm':
			if (setjmp(remcomEnv) == 0) {
				dofault = 0;
				/* TRY, TO READ %x,%x.  IF SUCCEED, SET PTR = 0 */
				ptr = &remcomInBuffer[1];
				if (hexToInt(&ptr, &addr))
					if (*(ptr++) == ',')
						if (hexToInt(&ptr, &length)) {
							ptr = 0;
							if (length * 2 >
							    OUTBUFMAX)
								length =
								    OUTBUFMAX /
								    2;
							mem2hex((char *)addr,
								remcomOutBuffer,
								length);
						}
				if (ptr)
					strcpy(remcomOutBuffer, "E01");
			} else
				strcpy(remcomOutBuffer, "E03");

			/* restore handler for bus error */
			dofault = 1;
			break;

			/* MAA..AA,LLLL: Write LLLL bytes (encoded hex) at address AA.AA return OK */
			/* XAA..AA,LLLL: Write LLLL bytes (encoded escaped-binary) at address AA.AA return OK */
		case 'M':
		case 'X':
			if (setjmp(remcomEnv) == 0) {
				dofault = 0;

				/* TRY, TO READ '%x,%x:'.  IF SUCCEED, SET PTR = 0 */
				ptr = &remcomInBuffer[1];
				if (hexToInt(&ptr, &addr))
					if (*(ptr++) == ',')
						if (hexToInt(&ptr, &length))
							if (*(ptr++) == ':') {
								if (remcomInBuffer[0] == 'M')
									hex2mem
									    (ptr,
									     (char
									      *)
									     addr,
									     length);
								else
									ebin2mem
									    (ptr,
									     (char
									      *)
									     addr,
									     length);
								flush_icache_range
								    (addr,
								     addr +
								     length);
								ptr = 0;
								strcpy
								    (remcomOutBuffer,
								     "OK");
							}
				if (ptr)
					strcpy(remcomOutBuffer, "E02");
			} else
				strcpy(remcomOutBuffer, "E03");

			/* restore handler for bus error */
			dofault = 1;
			break;

			/* cAA..AA    Continue at address AA..AA(optional) */
			/* sAA..AA   Step one instruction from AA..AA(optional) */
		case 'c':
		case 's':
			{
				/* tRY, to read optional parameter, pc unchanged if no parm */
				ptr = &remcomInBuffer[1];
				if (hexToInt(&ptr, &addr))
					registers[PC] = addr;

				if (remcomInBuffer[0] == 's')
					doSStep();
			}
			return;
			break;

			/* kill the program */
		case 'k':	/* do nothing */
			break;

			/* detach          D               Reply OK. */
		case 'D':	/* detach from program */
			ingdbmode = 0;
			/* reply to the request */
			putpacket("OK");
			/* Wait for user to type a character at terminal program on host. */
			getDebugChar();
			/* continue execution */
			return;
		}		/* switch */

		/* reply to the request */
		putpacket(remcomOutBuffer);
	}
}

#if defined(CONFIG_CPU_SH3)
#define TRA 0xffffffd0
#elif defined(CONFIG_CPU_SH4)
#define TRA 0xff000020
#endif

char ingdbmode;			/* nonzero -> gdb is listening on host */

#ifdef CONFIG_SESH4
/*
 * Set current value of the 8 LEDs on the SH4 SolutionEngine board.
 */
static void leds(unsigned char v)
{
#ifdef CONFIG_CPU_SUBTYPE_SH7751
	*((volatile unsigned short *)0xba000000) = (v << 8);
#else				/* !CONFIG_CPU_SUBTYPE_SH7751 */
	*((volatile unsigned short *)0xb0c00000) = (v << 8);
#endif				/* !CONFIG_CPU_SUBTYPE_SH7751 */
}
#endif

/* We've had an exception - choose to go into the monitor or
   the gdb stub */
void handle_exception(int exceptionVector)
{
#ifdef CONFIG_CPU_SH2
	int trapa_value = exceptionVector;
	int trap = trapa_value;
#else
	int trapa_value = ctrl_inl(TRA);
	int trap = trapa_value >> 2;
#endif
	if (exceptionVector == NMI_VEC) {
		ingdbmode = 1;
		gdb_handle_exception (exceptionVector, BREAK_TRAP);
		return;
	}

	switch (trap) {
	case BIOS_CALL_TRAP:
		/* BIOS call */
		handle_bios_call();
		break;

	default:
		/* among others, handles the breakpoint instruction trapa #BREAK_TRAP */
		ingdbmode = 1;
		gdb_handle_exception(exceptionVector, trapa_value);
		break;
	}
}

/* This function will generate a breakpoint exception.  It is used at the
   beginning of a program to sync up with a debugger and can be used
   otherwise as a quick means to stop program execution and "break" into
   the debugger. */

void breakpoint(void)
{
#ifdef CONFIG_CPU_SH2
	asm volatile("trapa	#32");
#else
	asm volatile ("trapa	#0xff");
#endif
}
