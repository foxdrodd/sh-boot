#include "config.h"
#include "linkage.h"

/* Bus State Controler initialize */

ENTRY(init_bsc)
	mov.l	WTCSR_A,r1	/* WTCSR Address */
	mov.w	WTCSR_D0,r0	/* WTCSR Data (WDT Disable) */
	mov.w	r0,@r1

	mov.w	WTCSR_D1,r0	/* WTCSR Data (WDT clock set) */
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

/* Configure & Reset Compact flash */
#ifdef CONFIG_DIRECT_COMPACT_FLASH
# ifdef CONFIG_IDE
        mov.l   CF_CIS_A,r1
        !
        mov.w   @r1+,r0
        extu.b  r0,r0
        mov.w   @r1+,r2
        extu.b  r2,r2
        swap.b  r2,r2
        or      r2,r0
        !
        mov.l   CF_CIS_ID,r1
        cmp/eq  r0,r1
        bf      3f
        !
        mov.l   CF_COR_A,r1
        mov.w   CF_COR_D1,r0
        mov.w   r0,@r1
        nop
        nop
        nop
        mov.l   CF_COR_A,r1
        mov.w   CF_COR_D2,r0
        mov.w   r0,@r1
        nop
# endif
3:
#endif
	rts
	 nop

	.balign 4

WTCSR_A:
	.long	0xffffff84	/* WDT CSR */
FRQCR_A:	
	.long	0xffffff80	/* FRQCR */	
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
SDMR_A:
	.long	0xffffe000 + (0x0220 * 4)
LED_A:
	.long	0xb4007400	/* LED Address */

#ifdef CONFIG_DIRECT_COMPACT_FLASH
# ifdef CONFIG_IDE
CF_CIS_A:       .long   (CONFIG_CIS_BASE+0x0074)
CF_CIS_ID:
        .long   0x00000101      /* disk_interface [ide] */
CF_COR_A:
        .long   (CONFIG_CIS_BASE+0x200)
CF_COR_D1:
        .word   0x00C2  /* Reset, configuration: 0x02, Level interrupt: 0x40 */
CF_COR_D2:
        .word   0x0042  /* Select configuration: 0x20, Level interrupt: 0x40 */
# endif
#endif
WTCSR_D0:	
	.word	0xa500
WTCSR_D1:
	.word	0xa584
/*
 MODE=2, PLL2=x4, PLL1=x2, PRE1=x1, PRE2=x1/3
         STC=001, IFC=000, PFC=100, CKOEN=1
         0010 0001 1001 0000
*/
FRQCR_D:	
	.word	0x2110
BCR1_D:
	.word	0x300b
BCR2_D:
	.word	0x29d4
WCR1_D:
	.word	0x3c02
WCR2_D:
	.word	0xfedf
PCR_D:
	.word	0x00ff
RTCSR_D:
	.word	0xa508
RTCOR_D:
	.word	0xa5bf
MCR_D:
	.word	0x1164
RFCR_D:
	.word	0xa400
LED_D:
	.word	0xffff		/* LED DATA */
