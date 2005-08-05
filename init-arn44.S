/*
 *	$Id: init-arn44.S,v 1.4 2004/03/06 23:01:59 socio Exp $
 *	gdb-sh-stub/init-arn44.S
 */

#include "linkage.h"

ENTRY(init_bsc)
	/*
	 * PFC initialization
	 */
	mov.l	PACRL1_A, r1	/* PACRL1 Address */
	mov.w	PACRL1_D, r0	/* PACRL1 Data */
	mov.w	r0, @r1

	mov.l	PACRL2_A, r1	/* PACRL2 Address */
	mov.w	PACRL2_D, r0	/* PACRL2 Data */
	mov.w	r0, @r1

	mov.l	PBCR1_A, r1	/* PBCR1 Address */
	mov.w	PBCR1_D, r0	/* PBCR1 Data */
	mov.w	r0, @r1

	mov.l	PBCR2_A, r1	/* PBCR2 Address */
	mov.w	PBCR2_D, r0	/* PBCR2 Data */
	mov.w	r0, @r1

	mov.l	PCCR_A, r1	/* PCCR Address */
	mov.w	PCCR_D, r0	/* PCCR Data */
	mov.w	r0, @r1

	mov.l	PDCRL_A, r1	/* PDCRL Address */
	mov.w	PDCRL_D, r0	/* PDCRL Data */
	mov.w	r0, @r1

	/*
	 * BSC initialization
	 */
	mov.l	BCR1_A, r1	/* BCR1 Address */
	mov.w	BCR1_D, r0	/* BCR1 Data */
	mov.w	r0, @r1

	mov.l	BCR2_A, r1	/* BCR2 Address */
	mov.w	BCR2_D, r0	/* BCR2 Data */
	mov.w	r0, @r1

	mov.l	WCR1_A, r1	/* WCR1 Address */
	mov.w	WCR1_D, r0	/* WCR1 Data */
	mov.w	r0, @r1

	mov.l	PAIORL_A, r1	/* PAIORL Address */
	mov.w	PAIORL_D, r0	/* PAIORL Data */
	mov.w	r0, @r1

	/*
	 * I/O Port initialization
	 */
	mov.l	PADRH_A, r1	/* PADRH Address */
	mov.w	PADRH_D, r0	/* PADRH Data */
	mov.w	r0, @r1

	mov.l	PADRL_A, r1	/* PADRL Address */
	mov.w	PADRL_D, r0	/* PADRL Data */
	mov.w	r0, @r1

	rts
	 nop

	.align 2

!
! Register definitions
!
PADRH_A:	.long	0xffff8380
PADRL_A:	.long	0xffff8382
PAIORL_A:	.long	0xffff8386
PACRL1_A:	.long	0xffff838c
PACRL2_A:	.long	0xffff838e
PBCR1_A:	.long	0xffff8398
PBCR2_A:	.long	0xffff839a
PCCR_A:		.long	0xffff839c
PDCRL_A:	.long	0xffff83ac
BCR1_A:		.long	0xffff8620
BCR2_A:		.long	0xffff8622
WCR1_A:		.long	0xffff8624

!
! Register values
!
PADRH_D:	.word	0xfedf
PADRL_D:	.word	0x00ff
PAIORL_D:	.word	0x8116
PACRL1_D:	.word	0x1550
PACRL2_D:	.word	0xa145
PBCR1_D:	.word	0x000a
PBCR2_D:	.word	0xa005
PCCR_D:		.word	0xffff
PDCRL_D:	.word	0xffff
BCR1_D:		.word	0x200f
BCR2_D:		.word	0xc099
WCR1_D:		.word	0xf004
