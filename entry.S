/* $Id: entry.S,v 1.33 2001/08/09 12:55:58 sugioka Exp $
 *
 * sh-ipl+g/entry.S
 *
 * Some code was taken from sh-stub.c of GDB 4.18.
 *
 *  Copyright (C) 1999, 2000  Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */

#include "config.h"
#include "linkage.h"

#if defined(CONFIG_CPU_SH3)
TRA      = 0xffffffd0
EXPEVT   = 0xffffffd4
INTEVT   = 0xffffffd8
TMU0_TCR = 0xfffffe9c
#define CCR			0xffffffec	/* Cache Control Register */
#define CCR_CACHE_DISABLE	0x008		/* Flush the cache, disable */
#define CCR_CACHE_INIT		0x0		/* disable */
#elif defined(CONFIG_CPU_SH4)
TRA      = 0xff000020
EXPEVT   = 0xff000024
INTEVT   = 0xff000028
TMU0_TCR = 0xffd80010
#define CCR			0xFF00001C	/* Cache Control Register */
#define CCR_CACHE_DISABLE	0x0808		/* Flush the cache, disable */
#define CCR_CACHE_INIT		0x0		/* disable */
#endif

	.text
	.balign 	4096,0,4096

ENTRY(start)
   	/*
	 * This code executes at powerup and manual reset,
	 * at location 0xa0000000 (P2) with the MMU disabled.
	 */
	mov.l	8f,r0
	mov.l	@r0,r13			/* save EXPEVT */

	/* Disable cache */
	mov.l	CCR_A, r1		/* CCR Address */
	mov.l	CCR_DISABLE, r0	/* CCR Data */
	mov.l	r0, @r1

	/* Initialize BSC */
	mov.l	1f, r0
	jsr	@r0
	 nop

	/* Initialize cache */
	mov.l	CCR_A, r1	/* CCR Address */
	mov.l	CCR_D, r0	/* CCR Data */
	mov.l	r0, @r1

	/* Initialize VBR */
	/* While the initial code executes from P2, the new
	 * value of the VBR points to P2 (0xA0000000) or P1 (0x80000000) which
	 * is where the stub code is linked.  This ensures that
	 * the exception handlers can run regardless of the
	 * setting of the MMU and cache when the trap occurs.
	 * This enables traps into the stub from the kernel
	 * to work in the brief window between initialising
	 * the MMU and initialising the kernel's trap handlers.
	 */
	mov.l	VBR_INIT, r0
	ldc	r0, vbr

	/* SR: MD=1, BL=1, RB=0, IMASK=0xf */
	mov.l	INITIAL_SR, r0
	ldc	r0, sr

	mov.l	INITIAL_STACK, r0
	mov	r0, r15

	/* Initialize BSS */
	mov.l	2f, r1
	add	#4, r1
	mov.l	3f, r2
	mov	#0, r0
9:	cmp/hs	r2, r1
	bf/s	9b		! while (r1 < r2)
	 mov.l	r0, @-r2

	/* Initialize Serial Port */
	mov.l	4f, r0
	jsr	@r0
	 nop
#if defined(CONFIG_IDE)
	/* Initialize IDE */
	mov.l	5f, r0
	jsr	@r0
	 nop
#endif
	mov.l	7f,r0
	mov.l	r13,@r0		/* set initial EXPEVT */

	/* Jump to GDB Stub */
	mov.l	6f, r0
	jmp	@r0
	 nop

	.align	2
1:	.long	SYMBOL_NAME(init_bsc)
2:	.long	SYMBOL_NAME(__bss_start)
3:	.long	SYMBOL_NAME(__bss_end)
4:	.long	SYMBOL_NAME(init_serial)
#if defined(CONFIG_IDE)
5:	.long	SYMBOL_NAME(init_ide)
#endif
6:	.long	SYMBOL_NAME(start_main)
7:	.long	SYMBOL_NAME(expevt_on_start)
8:	.long	EXPEVT
CCR_A:	.long	CCR
CCR_DISABLE:
	.long	CCR_CACHE_DISABLE
CCR_D:	.long	CCR_CACHE_INIT
INITIAL_SR:
	.long	0x500000f0	/* MD=1, RD=0, BL=1, IMASK=0xf */
INITIAL_STACK:
	.long	SYMBOL_NAME(stub_stack)
VBR_INIT:
	.long	SYMBOL_NAME(start)

!
!
	.balign 	256,0,256
general_exception:
	mov.l	1f, r2
	bra	do_exception
	 mov.l	@r2, r2
	.align	2
1:	.long	EXPEVT
!
!
	.balign 	1024,0,1024
tlb_miss:
	mov.l	1f, r2
	bra	do_exception
	 mov.l	@r2, r2
!
	.balign 	512,0,512
interrupt:
	mov.l	2f, r2
	bra	do_exception
	 mov.l	@r2, r2
	.align	2
1:	.long	EXPEVT
2:	.long	INTEVT

!
!
do_exception:
	shlr2	r2
	shlr2	r2
	shlr	r2
	mov.l	2f, r0
	stc	sr, r1		! back to normal register bank
	and	r0, r1		! ..
	ldc	r1, sr		! ...changed here.
	! Then, dispatch to the handler, according to the exception code.
	ldc	r0, r0_bank
	ldc	r1, r1_bank
	stc	r2_bank, r1		! exception vector code
	shll2	r1
	mov.l	1f, r0
	add	r1, r0
	stc	r1_bank, r1
	mov.l	@r0, r0
	jmp	@r0
	 stc	r0_bank, r0		! recovering r0..
	.align	2
1:	.long	exception_handling_table
2:	.long	0xdfffffff	! RB=0

exception_handling_table:
	.long	0 ! poweron reset
	.long	0 ! manual reset
	.long	0 ! tlb_miss_load
	.long	0 ! tlb_miss_store
	.long	0 ! initial_page_write
	.long	0 ! tlb_protection_violation_load
	.long	0 ! tlb_protection_violation_store
	.long	address_error_load
	.long	address_error_store
	.long	0 ! FPU exception
	.long	0 ! multi-tlb hit
	.long	unconditional_trap
	.long	reserved_instruction
	.long	illegal_slot_instruction
	.long	nmi
	.long	user_break
ENTRY(interrupt_table)
	! external hardware
	.long	do_IRQ	! 0000
	.long	do_IRQ	! 0001
	.long	do_IRQ	! 0010
	.long	do_IRQ	! 0011
	.long	do_IRQ	! 0100
	.long	do_IRQ	! 0101
	.long	do_IRQ	! 0110
	.long	do_IRQ	! 0111
	.long	do_IRQ	! 1000
	.long	do_IRQ	! 1001
	.long	do_IRQ	! 1010
	.long	do_IRQ	! 1011
	.long	do_IRQ	! 1100
	.long	do_IRQ	! 1101
	.long	do_IRQ	! 1110
	.long	0	! <0x3e0>
	! Internal hardware
	.long	do_timer	! TMU0 tuni0
	.long	do_IRQ		! TMU1 tuni1
	.long	do_IRQ		! TMU2 tuni2
	.long	do_IRQ		!      ticpi2
	.long	do_IRQ		! RTC  ati
	.long	do_IRQ		!      pri
	.long	do_IRQ		!      cui
	.long	do_IRQ		! SCI  eri
	.long	do_IRQ		!      rxi
	.long	do_IRQ		!      txi
	.long	do_IRQ		!      tei
	.long	do_IRQ		! WDT  iti
	.long	do_IRQ		! REF  rcmi
	.long	do_IRQ		!      rovi
	.long	do_IRQ
	.long	do_IRQ
	.long	do_IRQ		! Hitachi UDI
	.long	do_IRQ
	.long	do_IRQ		! DMAC dmte0
	.long	do_IRQ		!      dmte1
	.long	do_IRQ		!      dmte2
	.long	do_IRQ		!      dmte3
	.long	do_IRQ		!      dmae
	.long	do_IRQ
	.long	do_IRQ		! SCIF eri
	.long	do_IRQ		!      rxi
	.long	break		!      bri
	.long	do_IRQ		!      txi
	.long	0 ! <0x780>
	.long	0 ! <0x7a0>
	.long	0 ! <0x7c0>
	.long	0 ! <0x7e0>
	.long	0 ! FPU disabled (general)
	.long	0 ! FPU disabled (slot)
#if CONFIG_CPU_SUBTYPE_SH7751
	.long	0 ! <0x840>
	.long	0 ! <0x860>
	.long	0 ! <0x880>
	.long	0 ! <0x8a0>
	.long	0 ! <0x8c0>
	.long	0 ! <0x8e0>
	.long	0 ! <0x900>
	.long	0 ! <0x920>
	.long	0 ! <0x940>
	.long	0 ! <0x960>
	.long	0 ! <0x980>
	.long	0 ! <0x9a0>
	.long	0 ! <0x9c0>
	.long	0 ! <0x9e0>
	.long	do_pciIRQ ! PCIC SERR  <0xa00>
	.long	do_pciIRQ ! PCIC DMA3  <0xa20>
	.long	do_pciIRQ ! PCIC DMA2  <0xa40>
	.long	do_pciIRQ ! PCIC DMA1  <0xa60>
	.long	do_pciIRQ ! PCIC DMA0  <0xa80>
	.long	do_pciIRQ ! PCIC PWON  <0xaa0>
	.long	do_pciIRQ ! PCIC PWDWN <0xac0>
	.long	do_pciIRQ ! PCIC ERR   <0xae0>
	.long	do_IRQ	 ! TMU3 tuni3
	.long	0 ! <0xb20>
	.long	0 ! <0xb40>
	.long	0 ! <0xb60>
       	.long  	do_IRQ	 ! TMU4 tuni4
#endif /* CONFIG_CPU_SUBTYPE_SH7751 */

saveRegisters:
	mov.l	L_reg, r0
	mov.l	@r15+, r1			! pop R0
	mov.l	r2, @(0x08, r0)			! save R2
	mov.l	r1, @r0				! save R0
	mov.l	@r15+, r1			! pop R1
	mov.l	r3, @(0x0c, r0)			! save R3
	mov.l	r1, @(0x04, r0)			! save R1
	mov.l	r4, @(0x10, r0)			! save R4
	mov.l	r5, @(0x14, r0)			! save R5
	mov.l	r6, @(0x18, r0)			! save R6
	mov.l	r7, @(0x1c, r0)			! save R7
	mov.l	r8, @(0x20, r0)			! save R8
	mov.l	r9, @(0x24, r0)			! save R9
	mov.l	r10, @(0x28, r0)		! save R10
	mov.l	r11, @(0x2c, r0)		! save R11
	mov.l	r12, @(0x30, r0)		! save R12
	mov.l	r13, @(0x34, r0)		! save R13
	mov.l	r14, @(0x38, r0)		! save R14
	mov.l	@r15+, r4			! save arg to handleException
	mov.l	r15, @(0x3c, r0)		! save R15
	add	#92, r0				! readjust register pointer
	stc	ssr, r2				! R2 has SR
	stc	spc, r1				! R1 has PC
	mov.l	r2, @-r0			! save SR
	sts.l	macl, @-r0			! save MACL
	sts.l	mach, @-r0			! save MACH
	stc.l	vbr, @-r0			! save VBR
	stc.l	gbr, @-r0			! save GBR
	sts.l	pr, @-r0			! save PR
	mov.l	L_stubstack, r2
	mov.l	L_hdl_except, r3
	mov.l	@r2, r15
	jsr	@r3
	 mov.l	r1, @-r0			! save PC
	mov.l	L_stubstack, r0
	mov.l	L_reg, r1
	bra	restoreRegisters
	 mov.l	r15, @r0			! save __stub_stack

	.balign	4
L_reg:
	.long	SYMBOL_NAME(registers)
L_stubstack:
	.long	SYMBOL_NAME(stub_sp)
L_hdl_except:
	.long	SYMBOL_NAME(handle_exception)

restoreRegisters:
	add	#8, r1				! skip to R2
	mov.l	@r1+, r2			! restore R2
	mov.l	@r1+, r3			! restore R3
	mov.l	@r1+, r4			! restore R4
	mov.l	@r1+, r5			! restore R5
	mov.l	@r1+, r6			! restore R6
	mov.l	@r1+, r7			! restore R7
	mov.l	@r1+, r8			! restore R8
	mov.l	@r1+, r9			! restore R9
	mov.l	@r1+, r10			! restore R10
	mov.l	@r1+, r11			! restore R11
	mov.l	@r1+, r12			! restore R12
	mov.l	@r1+, r13			! restore R13
	mov.l	@r1+, r14			! restore R14
	mov.l	@r1+, r15			! restore programs stack
	mov.l	@r1+, r0
	ldc	r0, spc				! restore PC
	lds.l	@r1+, pr			! restore PR
	ldc.l	@r1+, gbr			! restore GBR		
	ldc.l	@r1+, vbr			! restore VBR
	lds.l	@r1+, mach			! restore MACH
	lds.l	@r1+, macl			! restore MACL
	mov.l	@r1, r0	
	add	#-88, r1			! readjust reg pointer to R1
	ldc	r0, ssr				! restore SR
	mov.l	r2, @-r15
	mov.l	L_in_nmi, r0
	mov	#0, r2
	mov.b	r2, @r0
	mov.l	@r15+, r2
	mov.l	@r1+, r0			! restore R0
	rte
	 mov.l	@r1, r1				! restore R1

address_error_load:
address_error_store:
	add	#-4, r15		! reserve spot on stack
	mov.l	r1, @-r15		! push R1
	  /* Exception 9 (bus errors) are disasbleable - so that you
	     can probe memory and get zero instead of a fault.
	     Because the vector table may be in ROM we don't revector
	     the interrupt like all the other stubs, we check in here
	     */
	mov.l	L_dofault, r1
	mov.l	@r1, r1
	tst	r1, r1
	bf	1f
	mov.l	L_handle_buserror, r0
	jmp	@r0
	 nop
	.align	2
L_dofault:
	.long 	SYMBOL_NAME(dofault)
L_handle_buserror:
	.long	SYMBOL_NAME(handle_buserror)

break:
	add	#-4, r15		! reserve spot on stack
	mov.l	r1, @-r15		! push R1
	mov.l	r0, @-r15		! push R0
	mov.l	r2, @-r15		! push R2
	mov.l	4f, r0
	jsr	@r0
	 mov.l	r3, @-r15		! push R3
	mov.l	@r15+, r3		! pop R3
	bra	2f
	 mov.l	@r15+, r3		! pop R2

	.align	2
4:	.long	SYMBOL_NAME(handleError)

unconditional_trap:
reserved_instruction:
illegal_slot_instruction:
user_break:
	add	#-4, r15		! reserve spot on stack
	mov.l	r1, @-r15		! push R1

1:	mov.l	r0, @-r15		! push R0
	/* Prepare for saving context, we've already pushed r0 and r1, stick
	   exception number into the frame */
2:	mov.l	3f, r0
	stc	sr, r1
	and	r0, r1
	ldc	r1, sr			! sti (enable interrupt)
	mov	r15, r0
	add	#8, r0
	stc	r2_bank, r1
	bra	saveRegisters		! save register values
	 mov.l	r1, @r0			! save exception #

	.align	2
3:	.long	0xefffffff		! BL=0

nmi:
	add	#-4, r15		! reserve spot on stack
	mov.l	r1, @-r15		! push R1
	/* Special case for NMI - make sure that they don't nest */
	mov.l	r0, @-r15		! push R0
	mov.l	L_in_nmi, r0
	tas.b	@r0			! Fend off against addtnl NMIs
	bt	2b
	mov.l	@r15+, r0
	mov.l	@r15+, r1
	add	#4, r15
	rte
	 nop

	.align	2
L_in_nmi:
	.long	SYMBOL_NAME(in_nmi)

do_pciIRQ:
	rte
	 nop

do_IRQ:
	rte
	 nop

do_timer:			/* Clear flag */
	mov.l	1f, r0
	mov.w	@r0, r1
	mov.w	2f, r2
	and	r2, r1
	mov.w	r1, @r0
	rte
	 nop
	.align	2
1:	.long	TMU0_TCR
2:	.word	0xfeff

/* End of entry.S */
