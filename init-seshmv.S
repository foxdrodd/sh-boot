/* $Id: init-seshmv.S,v 1.1 2005/01/24 01:27:37 doyu Exp $
 *
 *  gdb-sh-stub/init-sesh3.S
 *
 *  Data taken from Chapter 11 of "SH7709A SolutionEngine Manual"
 *
 */

#include "config.h"
#include "linkage.h"

PFC	= 0xa4050100
PACR	= PFC+0x00
PBCR	= PFC+0x02
PCCR	= PFC+0x04
PDCR	= PFC+0x06
PECR	= PFC+0x08
PFCR	= PFC+0x0A
PGCR	= PFC+0x0C
PHCR	= PFC+0x0E
PJCR	= PFC+0x10
PKCR	= PFC+0x12
PLCR	= PFC+0x14
PMCR	= PFC+0x18
PNCR	= PFC+0x1A
PQCR	= PFC+0x1C
PSELA	= PFC+0x40
PSELB	= PFC+0x42
PSELC	= PFC+0x44
HIZCRA	= PFC+0x46
HIZCRB	= PFC+0x48
DRVCR	= PFC+0x50
	
ENTRY(init_bsc)
	/* RTWDT */
	mov.l	RWTCSR_A, r1
	mov.w	RWTCSR_D, r0
	mov.w	r0,@r1

	mov.l	MMUCR_A, r1
	mov.l	MMUCR_D, r0
	mov.l	r0, @r1

	/* Initialize PFC */
	mov.l	PFC_A, r0
	ldc	r0,gbr

	mov.w	PSELA_D, r0
	mov.w	r0, @(PSELA-PFC,gbr)

	mov.w	PSELB_D, r0
	mov.w	r0, @(PSELB-PFC,gbr)

	mov.w	PSELC_D, r0
	mov.w	r0, @(PSELC-PFC,gbr)

	mov.w	HIZCRA_D, r0
	mov.w	r0, @(HIZCRA-PFC,gbr)

	mov.w	HIZCRB_D, r0
	mov.w	r0, @(HIZCRB-PFC,gbr)

	mov.w	DRVCR_D, r0
	mov.w	r0, @(DRVCR-PFC,gbr)

	mov.w	PACR_D, r0
	mov.w	r0, @(PACR-PFC,gbr)

	mov.w   PBCR_D, r0
        mov.w   r0, @(PBCR-PFC,gbr)

	mov.w   PCCR_D, r0
        mov.w   r0, @(PCCR-PFC,gbr)

	mov.w   PDCR_D, r0
        mov.w   r0, @(PDCR-PFC,gbr)

	mov.w   PECR_D, r0
        mov.w   r0, @(PECR-PFC,gbr)

	mov.w   PFCR_D, r0
        mov.w   r0, @(PFCR-PFC,gbr)

	mov.w   PGCR_D, r0
        mov.w   r0, @(PGCR-PFC,gbr)

	mov.w   PHCR_D, r0
        mov.w   r0, @(PHCR-PFC,gbr)

	mov.w   PJCR_D, r0
        mov.w   r0, @(PJCR-PFC,gbr)

	mov.w   PKCR_D, r0
        mov.w   r0, @(PKCR-PFC,gbr)

	mov.w   PLCR_D, r0
        mov.w   r0, @(PLCR-PFC,gbr)

	mov.w   PMCR_D, r0
        mov.w   r0, @(PMCR-PFC,gbr)

	mov.w   PNCR_D, r0
        mov.w   r0, @(PNCR-PFC,gbr)

	mov.w   PQCR_D, r0
        mov.w   r0, @(PQCR-PFC,gbr)

	/* CMT Initialize */
	mov.l	CMSTR_A, r1
	mov.w	CMSTR_D, r0
	mov.w	r0,@r1

	mov.l	CMCSR0_A, r1
	mov.w	CMCSR0_D, r0
	mov.w	r0,@r1

	mov.l	CMCOR0_A, r1
	mov.l	CMCOR0_D, r0
	mov.l	r0, @r1

	bra	9f
	 nop	

	.align	2
RWTCSR_A: .long 0xA4520004
RWTCSR_D: .word 0xA500
	.align	2

MMUCR_A: .long	0xFFFFFFE0
MMUCR_D: .long  0x00000004

CMSTR_A:  .long 0xA44A0000
CMCSR0_A: .long 0xA44A0010
CMCOR0_A: .long 0xA44A0018

PFC_A:	.long	PFC
PSELA_D: .word	0x0030
PSELB_D: .word	0x0118
PSELC_D: .word	0x0000
HIZCRA_D: .word	0x0000
HIZCRB_D: .word	0x0100
DRVCR_D:  .word 0x0000
PACR_D:	  .word	0x0000
PBCR_D:	  .word	0x0000
PCCR_D:	  .word	0x0000
PDCR_D:	  .word	0xAAAA
PECR_D:	  .word	0x0000
PFCR_D:	  .word	0x0000
/*PGCR_D:	  .word	0x0AAA*/
PGCR_D:	  .word	0x0000
/*PHCR_D:	  .word	0x2AAA */
PHCR_D:	  .word	0x2000
PJCR_D:	  .word	0x0000
PKCR_D:	  .word	0x0000
PLCR_D:	  .word	0x0000
PMCR_D:	  .word	0x0000
/*PMCR_D:	  .word	0xAAAA*/
PNCR_D:	  .word	0x0000
PQCR_D:	  .word	0x0000
CMSTR_D:  .word 0x0001
CMCSR0_D: .word 0x0002
	.align	2
CMCOR0_D: .long 0x000004B9
9:

FRQCR	= 0xA415FF80
WTCSR	= FRQCR+0x06
BSC	= 0xA4FD0000
CMNCR	= BSC+0x00
CS0BCR	= BSC+0x04
CS2BCR	= BSC+0x08
CS3BCR	= BSC+0x0C
CS4BCR	= BSC+0x10
CS5ABCR	= BSC+0x14
CS5BBCR	= BSC+0x18
CS6ABCR	= BSC+0x1C
CS6BBCR	= BSC+0x20
CS0WCR	= BSC+0x24
CS2WCR	= BSC+0x28
CS3WCR	= BSC+0x2C
CS4WCR	= BSC+0x30
CS5AWCR	= BSC+0x34
CS5BWCR	= BSC+0x38
CS6AWCR	= BSC+0x3C
CS6BWCR	= BSC+0x40
SDCR	= BSC+0x44
RTCSR	= BSC+0x48
RTCNT	= BSC+0x4C
RTCOR	= BSC+0x50
	
	mov.l	FRQCR_A,r0
	ldc	r0,gbr

	mov.w	WTCSR_D,r0
	mov.w	r0,@(WTCSR-FRQCR,gbr)

	mov.w	FRQCR_D,r0
	mov.w	r0,@(0,gbr)

	mov.l	BSC_A,r0
	ldc	r0,gbr
	
	mov.l	CMNCR_D,r0
	mov.l	r0,@(CMNCR-BSC,gbr)

	mov.l	CS0BCR_D,r0
	mov.l	r0,@(CS0BCR-BSC,gbr)

	mov.l	CS2BCR_D,r0
	mov.l	r0,@(CS2BCR-BSC,gbr)

	mov.l	CS3BCR_D,r0
	mov.l	r0,@(CS3BCR-BSC,gbr)

	mov.l	CS4BCR_D,r0
	mov.l	r0,@(CS4BCR-BSC,gbr)

	mov.l	CS5ABCR_D,r0
	mov.l	r0,@(CS5ABCR-BSC,gbr)

	mov.l	CS5BBCR_D,r0
	mov.l	r0,@(CS5BBCR-BSC,gbr)

	mov.l	CS6ABCR_D,r0
	mov.l	r0,@(CS6ABCR-BSC,gbr)

	mov.l	CS6BBCR_D,r0
	mov.l	r0,@(CS6BBCR-BSC,gbr)

	mov.l	CS0WCR_D,r0
	mov.l	r0,@(CS0WCR-BSC,gbr)

	mov.l	CS2WCR_D,r0
	mov.l	r0,@(CS2WCR-BSC,gbr)

	mov.l	CS3WCR_D,r0
	mov.l	r0,@(CS3WCR-BSC,gbr)

	mov.l	CS4WCR_D,r0
	mov.l	r0,@(CS4WCR-BSC,gbr)

	mov.l	CS5AWCR_D,r0
	mov.l	r0,@(CS5AWCR-BSC,gbr)

	mov.l	CS5BWCR_D,r0
	mov.l	r0,@(CS5BWCR-BSC,gbr)

	mov.l	CS6AWCR_D,r0
	mov.l	r0,@(CS6AWCR-BSC,gbr)

	mov.l	CS6BWCR_D,r0
	mov.l	r0,@(CS6BWCR-BSC,gbr)

	mov.l	LED_A,r1	/* LED Address */
	mov	#0x5a,r0	/* LED half 'on' */
	shll8	r0
	mov.w	r0,@r1

	! Do you need PCMCIA setting?
	! If so, please add the lines here...

	mov.l	SDCR_D,r0	/* SDCR Data */
	mov.l	r0,@(SDCR-BSC,gbr)
	
	mov.l   SDMR3_A,r1      /* Set SDRAM mode */
	mov.w   SDMR3_D,r0
	mov.w	r0,@r1

	mov.l	RTCNT_D,r0	/* RTCNT Data */
	mov.l	r0,@(RTCNT-BSC,gbr)

	mov.l	RTCOR_D,r0	/* RTCOR Data */
	mov.l	r0,@(RTCOR-BSC,gbr)

	mov.l	RTCSR_D,r0      /* RTCSR Data */
	mov.l	r0,@(RTCSR-BSC,gbr)

	mov.l	SDMR3_A,r1	/* Set SDRAM mode */
        mov.w   SDMR3_D,r0
        mov.w   r0,@r1

1:
	mov.l	BOOT_CONFIG_WORD,r1
	mov.w	@r1,r0
	exts.b	r0,r0
	cmp/eq	#0xde,r0	! MAGIC # for "doDEs"
	bf	1f
	mov.l	0f,r5
	mov.l	0f,r6
	mov.l	0f,r7
	bra	1f
	 nop
	.align	2
0:	.long 0xbabeface
1:
	rts
	 nop

	.align 2

FRQCR_A:	.long	FRQCR
BSC_A:		.long	BSC
SDMR3_A:	.long	0xA4FD5440	/* SDMR3 Address */

#if CONFIG_PCLK_FREQ == 19800000
CMNCR_D:	.long	0x00000013
CS0BCR_D:	.long	0x12410400
CS2BCR_D:	.long	0x36DB0400
CS3BCR_D:	.long	0x12414400
CS4BCR_D:	.long	0x12410400
CS5ABCR_D:	.long	0x36DB0400
CS5BBCR_D:	.long	0x36DB0400
CS6ABCR_D:	.long	0x36DB0400
CS6BBCR_D:	.long	0x36DB0400
CS0WCR_D:	.long	0x00040180
CS2WCR_D:	.long	0x00001D03
CS3WCR_D:	.long	0x00006c93
CS4WCR_D:	.long	0x00040180
!CS5AWCR_D:	.long	0x00001D03
CS5AWCR_D:	.long	0x00000503	/* from MSTLAN manual */
CS5BWCR_D:	.long	0x00001D03
CS6AWCR_D:	.long	0x000409C1
CS6BWCR_D:	.long	0x000409C1
SDCR_D:		.long	0x00091812

RTCSR_D:	.long	0xA55A0099
RTCNT_D:	.long	0xA55A0000
RTCOR_D:	.long	0xA55A0010
SDMR3_D:	.word	0xEFAB
FRQCR_D:	.word	0x1404		/* 6:1:1 (Mode0) */
WTCSR_D:	.word	0xA502	/	/* pclk/16 */
#endif
#if CONFIG_PCLK_FREQ == 33333333
CMNCR_D:	.long	0x00000013
CS0BCR_D:	.long	0x36db0400
CS2BCR_D:	.long	0x36db0400
CS3BCR_D:	.long	0x36db4400
CS4BCR_D:	.long	0x36db0400
CS5ABCR_D:	.long	0x36db0400
CS5BBCR_D:	.long	0x36db0400
CS6ABCR_D:	.long	0x36db0400
CS6BBCR_D:	.long	0x36db0400
CS0WCR_D:	.long	0x00001382
CS2WCR_D:	.long	0x00001d03
CS3WCR_D:	.long	0x00006c9b
CS4WCR_D:	.long	0x00000380
CS5AWCR_D:	.long	0x00000e03
CS5BWCR_D:	.long	0x00001d03
CS6AWCR_D:	.long	0x00001c43
CS6BWCR_D:	.long	0x00001c43
SDCR_D:		.long	0x00090812

RTCSR_D:	.long	0xa55a000a
RTCNT_D:	.long	0xa55a0000
RTCOR_D:	.long	0xa55a0036
SDMR3_D:	.word	0xefab
FRQCR_D:	.word	0x1103		/* 2:1:0.5 */
WTCSR_D:	.word	0xA502		/* pclk/16 */
#endif
	.align 2
LED_A:	.long	0xB0800000	/* LED Address */

BOOT_CONFIG_WORD:	.long	0xb0800000

