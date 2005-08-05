#include "linkage.h"
#include "config.h"
#include "regs.h"
	
	.text
	.align 2

saveRegisters:
	/*
	 * At this point, the stack will look like:
	 *
	 * arg
	 * r1
	 * r0
	 * pc
	 * sr
	 */

	mov.l	1f, r0				! registers[]
	add	#88, r0				! skip to end

	mov.l	@(0x10, r15), r1		! save SR
	mov.l	r1, @r0

	sts.l	macl, @-r0			! save macl
	sts.l	mach, @-r0			! save mach
	stc.l	vbr, @-r0			! save vbr
	stc.l	gbr, @-r0			! save gbr
	sts.l	pr, @-r0			! save pr

	mov.l	@(0x0c, r15), r1		! save PC
	mov.l	r1, @-r0

	/*
	 * Fixup r15 so it's back in the same state it was before we took
	 * the exception.
	 */
	mov	r15, r1
	add	#20, r1
	mov.l	r1, @-r0			! save R15

	mov.l	r14, @-r0			! save R14 ... R2
	mov.l	r13, @-r0
	mov.l	r12, @-r0
	mov.l	r11, @-r0
	mov.l	r10, @-r0
	mov.l	r9, @-r0
	mov.l	r8, @-r0
	mov.l	r7, @-r0
	mov.l	r6, @-r0
	mov.l	r5, @-r0
	mov.l	r4, @-r0
	mov.l	r3, @-r0
	mov.l	r2, @-r0

	mov.l	@(0x04, r15), r1
	mov.l	r1, @-r0			! save R1
	
	mov.l	@(0x08, r15), r1
	mov.l	r1, @-r0			! save R0

	mov.l	@r15, r4

!	mov.l	2f, r2
	mov.l	3f, r3
!	mov.l	@r2, r15
	jsr	@r3
	 nop

	mov.l	2f, r0
	mov.l	1f, r1
	bra	restoreRegisters
	 mov.l	r15, @r0

	.align 2
1:	.long	SYMBOL_NAME(registers)
2:	.long	SYMBOL_NAME(stub_sp)
3:	.long	SYMBOL_NAME(handle_exception)

restoreRegisters:
	mov.l	1f, r0
	add	#8, r0				! skip R0 and R1 for now ..

	mov.l	@r0+, r2
	mov.l	@r0+, r3
	mov.l	@r0+, r4
	mov.l	@r0+, r5
	mov.l	@r0+, r6
	mov.l	@r0+, r7
	mov.l	@r0+, r8
	mov.l	@r0+, r9
	mov.l	@r0+, r10
	mov.l	@r0+, r11
	mov.l	@r0+, r12
	mov.l	@r0+, r13
	mov.l	@r0+, r14
	mov.l	@r0+, r15
	
	add	#-4, r15			! placeholder for SR

	mov.l	@r0+, r1
	mov.l	r1, @-r15			! push PC

	lds.l	@r0+, pr
	ldc.l	@r0+, gbr
	ldc.l	@r0+, vbr
	lds.l	@r0+, mach
	lds.l	@r0+, macl

	mov.l	@r0, r1
	mov.l	r1, @(0x04, r15)		! push SR

	add	#-88, r0			! rewind to beginning

	mov.l	@(0x04, r0), r1			! restore R1
	mov.l	@r0, r0				! restore R0

	rte
	 nop

	.align 2
1:	.long	SYMBOL_NAME(registers)

#define BUILD_TRAPA_HANDLER(trapa_nr)		\
ENTRY(trapa##trapa_nr##_handler)		\
	mov.l	r0, @-r15;			\
	mov.l	r1, @-r15;			\
						\
	mov.l	1f, r0;				\
	bra	saveRegisters;			\
	 mov.l	r0, @-r15;			\
	 					\
	.align 2;				\
1:	.long	trapa_nr

BUILD_TRAPA_HANDLER(32);
BUILD_TRAPA_HANDLER(33);
BUILD_TRAPA_HANDLER(34);
BUILD_TRAPA_HANDLER(35);
BUILD_TRAPA_HANDLER(36);
BUILD_TRAPA_HANDLER(37);
BUILD_TRAPA_HANDLER(38);
BUILD_TRAPA_HANDLER(39);
BUILD_TRAPA_HANDLER(40);
BUILD_TRAPA_HANDLER(41);
BUILD_TRAPA_HANDLER(42);
BUILD_TRAPA_HANDLER(43);
BUILD_TRAPA_HANDLER(44);
BUILD_TRAPA_HANDLER(45);
BUILD_TRAPA_HANDLER(46);
BUILD_TRAPA_HANDLER(47);
BUILD_TRAPA_HANDLER(48);
BUILD_TRAPA_HANDLER(49);
BUILD_TRAPA_HANDLER(50);
BUILD_TRAPA_HANDLER(51);
BUILD_TRAPA_HANDLER(52);
BUILD_TRAPA_HANDLER(53);
BUILD_TRAPA_HANDLER(54);
BUILD_TRAPA_HANDLER(55);
BUILD_TRAPA_HANDLER(56);
BUILD_TRAPA_HANDLER(57);
BUILD_TRAPA_HANDLER(58);
BUILD_TRAPA_HANDLER(59);
BUILD_TRAPA_HANDLER(60);
BUILD_TRAPA_HANDLER(61);
BUILD_TRAPA_HANDLER(62);
BUILD_TRAPA_HANDLER(63);

do_IRQ:
	rte
	 nop

do_timer:
	mov.l	1f, r0		/* Clear CMF */
	mov.w	@r0, r1
	mov.w	2f, r2
	and	r2, r1
	mov.w	r1, @r0
	rte
	 nop

	 .align 2
1:	.long	0xffff83d2	/* CMCSR0 */
2:	.long	0xff7f

	.section .vect
ENTRY(vbr_base)
	.long start		/*  0: power-on reset */
	.long 0xfffffffc
	.long start		/*  2: manual reset */
	.long 0xfffffffc
	.long do_IRQ		/*  4: general illegal instruction */
	.long do_IRQ		/*  5: reserved */
	.long do_IRQ		/*  6: slot illegal instruction */
	.long do_IRQ		/*  7: reserved */
	.long do_IRQ		/*  8: reserved */
	.long do_IRQ		/*  9: cpu address error */
	.long do_IRQ		/* 10: dmac/dtc address error */
	.long do_IRQ		/* 11: nmi */
	.long do_IRQ		/* 12: ubc */
	.long do_IRQ		/* 13: reserved */
	.long do_IRQ		/* 14: reserved */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 31: reserved */
	.long trapa32_handler	/* 32: trapa 32 */
	.long trapa33_handler
	.long trapa34_handler
	.long trapa35_handler
	.long trapa36_handler
	.long trapa37_handler
	.long trapa38_handler
	.long trapa39_handler
	.long trapa40_handler
	.long trapa41_handler
	.long trapa42_handler
	.long trapa43_handler
	.long trapa44_handler
	.long trapa45_handler
	.long trapa46_handler
	.long trapa47_handler
	.long trapa48_handler
	.long trapa49_handler
	.long trapa50_handler
	.long trapa51_handler
	.long trapa52_handler
	.long trapa53_handler
	.long trapa54_handler
	.long trapa55_handler
	.long trapa56_handler
	.long trapa57_handler
	.long trapa58_handler
	.long trapa59_handler
	.long trapa60_handler
	.long trapa61_handler
	.long trapa62_handler
	.long trapa63_handler	/* 63: trapa 63 */
	
	/* Interrupt exception vectors */
	.long do_IRQ		/* 64: IRQ0 */
	.long do_IRQ		/* 65: IRQ1 */
	.long do_IRQ		/* 66: IRQ2 */
	.long do_IRQ		/* 67: IRQ3 */
	.long do_IRQ		/* 68: IRQ4 */
	.long do_IRQ		/* 69: IRQ5 */
	.long do_IRQ		/* 70: IRQ6 */
	.long do_IRQ		/* 71: IRQ7 */
	.long do_IRQ		/* 72: DMAC0 DEI0 */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 76: DMAC1 DEI1 */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 80: DMAC2 DEI2 */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 84: DMAC3 DEI3 */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 88: MTU0 TGI0A */
	.long do_IRQ		/* 89:      TGI0B */
	.long do_IRQ		/* 90:      TGI0C */
	.long do_IRQ		/* 91:      TGI0D */
	.long do_IRQ		/* 92:      TGI0V */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 96: MTU1 TGI1A */
	.long do_IRQ		/* 97:      TGI1B */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 100:     TCI1V */
	.long do_IRQ		/* 101:     TCI1U */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 104: MTU2 TGI2A */
	.long do_IRQ		/* 105:      TGI2B */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 108:      TCI2V */
	.long do_IRQ		/* 109:      TCI2U */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 112: MTU3 TGI3A */
	.long do_IRQ		/* 113:      TGI3B */
	.long do_IRQ		/* 114:      TGI3C */
	.long do_IRQ		/* 115:      TGI3D */
	.long do_IRQ		/* 116:      TGI3V */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 120: MTU4 TGI4A */
	.long do_IRQ		/* 121:      TGI4B */
	.long do_IRQ		/* 122:      TGI4C */
	.long do_IRQ		/* 123:      TGI4D */
	.long do_IRQ		/* 124:      TGI4V */
	.long do_IRQ		/* 125:      reserved */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 128: SCI0 ERI0 */
	.long do_IRQ		/* 129:      RXI0 */
	.long do_IRQ		/* 130:      TXI0 */
	.long do_IRQ		/* 131:      TEI0 */
	.long do_IRQ		/* 132: SCI1 ERI1 */
	.long do_IRQ		/* 133:      RXI1 */
	.long do_IRQ		/* 134:      TXI1 */
	.long do_IRQ		/* 135:      TEI1 */
	.long do_IRQ		/* 136: ADC ADI */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 140: DTC SWDTE */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_timer		/* 144: CMT0 CMI0 */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 148: CMT1 CMI1 */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 152: WDT ITI */
	.long do_IRQ		/* 153: BSC CMI */
	.long do_IRQ
	.long do_IRQ
	.long do_IRQ		/* 156: I/O OEI */

	.space (256-157) * 4
