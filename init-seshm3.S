/* $Id: init-seshm3.S,v 1.1 2005/01/24 01:31:55 doyu Exp $
 *
 *  gdb-sh-stub/init-sesh3.S
 *
 *  Data taken from Chapter 11 of "SH7709A SolutionEngine Manual"
 *
 */

#include "config.h"
#include "linkage.h"

ENTRY(init_bsc)
CPG	= 0xA4150000
FRQCR	= CPG
BSC	= 0xfec10000
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

	mov.l	MSTPCR0_A, r1
	mov.l	MSTPCR0_D, r0
	mov.l	r0, @r1
	
	mov.l	MSTPCR2_A, r1
	mov.l	MSTPCR2_D, r0
	mov.l	r0, @r1

	mov.l	BSC_A,r0
	ldc	r0,gbr
	
	mov.l	CMNCR_D,r0
	mov.l	r0,@(CMNCR-BSC,gbr)

	mov.l	CS0BCR_D,r0
	mov.l	r0,@(CS0BCR-BSC,gbr)

	mov.l	CS2BCR_D,r0
	mov.l	r0,@(CS2BCR-BSC,gbr)

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

	rts
	 nop

	.align 2
MSTPCR0_A:	.long	0xa4150030
MSTPCR2_A:	.long	0xa4150038
BSC_A:		.long	BSC

MSTPCR0_D:	.long	0x00000000
MSTPCR2_D:	.long	0xffffeec0

CMNCR_D:	.long	0x0000001B
CS0BCR_D:	.long	0x12410400
CS2BCR_D:	.long	0x36DB4400
CS4BCR_D:	.long	0x12410400
CS5ABCR_D:	.long	0x36DB0400
CS5BBCR_D:	.long	0x36DB0400
CS6ABCR_D:	.long	0x12410400
CS6BBCR_D:	.long	0x36DB0400

CS0WCR_D:	.long	0x00000500
CS2WCR_D:	.long	0x00000100
CS4WCR_D:	.long	0x00000500
CS5AWCR_D:	.long	0x00000e03
CS5BWCR_D:	.long	0x00000540
CS6AWCR_D:	.long	0x00000500
CS6BWCR_D:	.long	0x00000540
