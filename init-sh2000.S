#include "config.h"
#include "linkage.h"

/* Bus State Controler initialize */

ENTRY(init_bsc)
	/* Setup WDT for FRQCR change */
	mov.l	WTCSR_A,r1	/* WTCSR Address */
	mov.w	WTCSR_D,r0	/* WTCSR Data (WDT Disable) */
	mov.w	r0,@r1

	mov.l	WTCNT_A,r1	/* WTCNT Address */
	mov.w	WTCNT_D,r0	/* WTCNT Data (WDT overflows after 18msec) */
	mov.w	r0,@r1

	mov.l	FRQCR_A,r1	/* FRQCR Address */
	mov.w	FRQCR_D,r0	/* FRQCR Data */
	mov.w	r0,@r1

	mov.l	BCR1_A,r1	/* BCR1 Address */
	mov.w	BCR1_D,r0	/* BCR1 Data */
	mov.w	r0,@r1

	mov.l	BCR2_A,r1	/* BCR2 Address */
	mov.w	BCR2_D,r0	/* BCR2 Data */
	mov.w	r0,@r1

	mov.l	BCR3_A,r1	/* BCR3 Address */
	mov.w	BCR3_D,r0	/* BCR3 Data */
	mov.w	r0,@r1

	mov.l	WCR1_A,r1	/* WCR1 Address */
	mov.w	WCR1_D,r0	/* WCR1 Data */
	mov.w	r0,@r1

	mov.l	WCR2_A,r1	/* WCR2 Address */
	mov.w	WCR2_D,r0	/* WCR2 Data */
	mov.w	r0,@r1

	mov.l	PCR_A,r1	/* PCR Address */
	mov.w	PCR_D,r0	/* PCR Data */
	mov.w	r0,@r1

	mov.l	RTCOR_A,r1	/* RTCOR Address */
	mov.w	RTCOR_D,r0	/* RTCOR Data */
	mov.w	r0,@r1

	mov.l	RTCSR_A,r1	/* RTCSR Address */
	mov.w	RTCSR_D,r0	/* RTCSR Data */
	mov.w	r0,@r1

	mov.l	MCR_A,r1	/* MCR Address */
	mov.w	MCR_D,r0	/* MCR Data1 */
	mov.w	r0,@r1

	mov.l   SDMR_A,r1       /* SDRAM MODE REGISTER */
	mov	#0,r0
	mov.b	r0,@r1

	mov.l	ICR0_A,r1	/* ICR0 Address */
	mov.w	ICR0_D,r0	/* ICR0 Data */
	mov.w	r0,@r1

	mov.l	ICR1_A,r1	/* ICR1 Address */
	mov.w	ICR1_D,r0	/* ICR1 Data */
	mov.w	r0,@r1

	mov.l	ICR2_A,r1	/* ICR2 Address */
	mov.w	ICR2_D,r0	/* ICR2 Data */
	mov.w	r0,@r1

	mov.l	PDCR_A,r1	/* PDCR Address */
	mov.w	PDCR_D,r0	/* PDCR Data */
	mov.w	r0,@r1

	mov.l	PECR_A,r1	/* PECR Address */
	mov.w	@r1,r0
	mov.w	PECR_MASK,r2	/* PECR MASK */
	and	r2,r0
	mov.w	r0,@r1

	mov.l	PHCR_A,r1	/* PHCR Address */
	mov.w	@r1,r0
	mov.w	PHCR_MASK,r2	/* PHCR MASK */
	and	r2,r0
	mov.w	r0,@r1

	mov.l	IRR0_A,r1	/* IRR0 Address */
	mov.w	@r1,r0
	mov	#0,r0
	mov.w	r0,@r1

	mov.l	LED_A,r1	/* LED Address */
	mov.w	LED_D,r0	/* LED Data */
	mov.w	r0,@r1

	/* Wait DRAM refresh 8 times */
	mov.l	RFCR_A,r1	/* RFCR Address */
	mov.l	RFCR_D,r0	/* RFCR Data */
	mov.l	r0,@r1		/* Clear reflesh counter */
	mov	#8,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b

#if defined(CONFIG_DIRECT_COMPACT_FLASH) && defined(CONFIG_IDE)
	/* wait until power on reset complete */
	mov.l	DLY_CNT,r0
2:	dt	r0
	bf	2b

	/* Configure & Reset Compact flash */
	mov.l	CF_COR_A,r1
	mov	#0x80,r0
	mov.b	r0,@r1

	mov.l	DLY_CNT2,r0
2:	dt	r0
	bf	2b

	mov.l	CF_COR_A,r1
	mov	#0,r0
	mov.b	r0,@r1

	mov.l	DLY_CNT2,r0
2:	dt	r0
	bf	2b

	mov.l	CF_COR_A,r1
	mov	#0x42,r0
	mov.b	r0,@r1
3:
#endif
	rts
	 nop

	.align 2

BCR1_A:
	.long	0xffffff60	/* BCR1 Address */
BCR2_A:
	.long	0xffffff62	/* BCR2 Address */
WCR1_A:
	.long	0xffffff64	/* WCR1 Address */
WCR2_A:
	.long	0xffffff66	/* WCR2 Address */
MCR_A:
	.long	0xffffff68	/* MCR Address */
PCR_A:
	.long	0xffffff6c	/* PCR Address */
RTCSR_A:
	.long	0xffffff6e	/* RTCSR Address */
RTCOR_A:
	.long	0xffffff72	/* RTCOR Address */
RFCR_A:
	.long	0xffffff74	/* RFCR Address */
BCR3_A:
	.long	0xffffff7e	/* BCR3 Address */
FRQCR_A:	
	.long	0xffffff80	/* FRQCR */	
WTCNT_A:
	.long	0xffffff84	/* WDT CNT */
WTCSR_A:
	.long	0xffffff86	/* WDT CSR */
SDMR_A:
	.long	0xffffe000 + (0x0220 * 4)
ICR0_A:
	.long	0xfffffee0
ICR1_A:
	.long	0xa4000010
ICR2_A:
	.long	0xa4000012
PDCR_A:
	.long	0xa4000106
PECR_A:
	.long	0xa4000108
PHCR_A:
	.long	0xa400010e
IRR0_A:
	.long	0xa4000004
LED_A:
	.long	0xb4800000	/* LED Address */

#if defined(CONFIG_DIRECT_COMPACT_FLASH) && defined(CONFIG_IDE)
DLY_CNT:
	.long	40000
DLY_CNT2:
	.long	4000
CF_COR_A:
        .long   (CONFIG_CIS_BASE+0x200)
#endif
WTCNT_D:
	.word	0x5a80
WTCSR_D:	
	.word	0xa507
/*
 MODE=2, PLL2=x4, PLL1=x2, PRE1=x1, PRE2=x1/4
         STC=001, IFC=000, PFC=010, CKOEN=1
         0000 0001 0001 0010
*/
FRQCR_D:	
	.word	0x0112
BCR1_D:
	.word	0xf00b
BCR2_D:
	.word	0x2af0
BCR3_D:
	.word	0x0000
WCR1_D:
	.word	0xbff3
WCR2_D:
	.word	0xffdf
PCR_D:
	.word	0x0033
RTCSR_D:
	.word	0xa508
RTCOR_D:
	.word	0xa5e1
MCR_D:
	.word	0x5564
RFCR_D:
	.word	0xa400
ICR0_D:
	.word	0x0aaa
ICR1_D:
	.word	0x0001
ICR2_D:
	.word	0x0001
PDCR_D:
	.word	0x000a
PECR_MASK:
	.word	~0x0f00
PHCR_MASK:
	.word	~0x03f3
LED_D:
	.word	0xffff		/* LED DATA */
