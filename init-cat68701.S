#include "config.h"
#include "linkage.h"

/* Bus State Controler initialize */

ENTRY(init_bsc)
	mov.l	WTCSR_A,r1	/* WTCSR Address */
	mov.w	WTCSR_D,r0	/* WTCSR Data (WDT Disable) */
	mov.w	r0,@r1

	mov.l	WTCNT_A,r1	/* WTCNT Address */
	mov.w	WTCNT_D,r0	/* WTCNT Data */
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
	mov.w	RFCR_D,r0	/* RFCR Data */
	mov.w	r0,@r1		/* Clear reflesh counter */
	mov	#8,r3
1:
	mov.w	@r1,r0
	extu.w	r0,r2
	cmp/hi	r3,r2
	bf	1b
	bra	COMPACTFLASH
	 nop

	.balign 4

WTCNT_A:
	.long	0xffffff84	/* WDT CNT */
WTCSR_A:
	.long	0xffffff86	/* WDT CSR */
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

WTCNT_D:
	.word	0x5a80
WTCSR_D:
	.word	0xa507
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
	.word	0x3c03		/* 2001-06-11  3c02->3c03 by Y.E. */
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

	
/*---------------------------------------------------------------------*/
/* Configure & Reset Compact flash */
	

	.balign 4	
COMPACTFLASH:
		
#if defined(CONFIG_DIRECT_COMPACT_FLASH) && defined(CONFIG_IDE)
        /* wait	until power on reset complete */
	mov.l   DLY_CNT,r0
2:	dt      r0
	bf      2b

/* Configure & Reset Compact flash */

	mov.l   CF_COR_A,r1  /* soft reset */
	mov     #0x80,r0
	mov.w   r0,@r1

	mov.l   DLY_CNT2,r0  /* wait a moment */
2:	dt      r0
	bf      2b
	
	mov.l   CF_COR_A,r1  /* soft reset clear */
	mov     #0,r0
	mov.w   r0,@r1

	mov.l	CF_PRR_A,r1  /* wait until RRdy/Busy clear */
2:	
	mov.w	@r1,r0
	and	#2,r0
	tst	r0,r0
	bt	2b

	mov.l   CF_CCSR_A,r1 /* Powrdown mode bit clear */
	mov     #0,r0
	mov.w   r0,@r1

	mov.l	CF_PRR_A,r1  /* wait until RRdy/Busy clear */
2:	
	mov.w	@r1,r0
	and	#2,r0
	tst	r0,r0
	bt	2b

	mov.l   CF_COR_A,r1
	mov     #0x42,r0
	mov.w   r0,@r1       /* IRQ=level Index=2 */

	mov.l	CF_PRR_A,r1  /* wait until RRdy/Busy clear */
2:	
	mov.w	@r1,r0
	and	#2,r0
	tst	r0,r0
	bt	2b

	bra	READ_EEPROM
	 nop

	.balign 4
DLY_CNT:
	.long   80000
DLY_CNT2:
	.long   8000
CF_COR_A:
	.long   (CONFIG_CIS_BASE+0x200)
CF_CCSR_A:
	.long   (CONFIG_CIS_BASE+0x202)
CF_PRR_A:
	.long   (CONFIG_CIS_BASE+0x204)

#endif

	
/* ---------------------------------------------------------------------*/


	.balign 4
READ_EEPROM:
	mov.l	CS8900_ADDR,r1	/* CS8900 dummy read */
	mov.w	@r1,r0

	/* writereg(PP_SelfCTL, readreg(PP_SelfCTL) | POWER_ON_RESET);
	   PP_SelfCTL=0x0114
	   POWER_ON_RESET=0x0040 */

	mov.w   PP_SelfCTL,r2
	mov.l   CS8900_ADDR,r1
	mov.l   CS8900_DATA,r4
	mov.w   r2,@r1
	mov.w   @r4,r3
	extu.w  r3,r0
	or      #0x40,r0
	mov.w   r2,@r1
	mov.w   r0,@r4

	/* for(i=10000; i>0; i--) ; */
	mov.l   L10000,r1
loop:
	add     #-1,r1
	cmp/pl  r1
	bt	loop
	nop
				
	/* writereg(PP_EECMD, offset|EEPROM_READ_CMD);
	   PP_EECMD=0x40
	   EEPROM_READ_CMD=0x0200 */

	mov.l	CS8900_ADDR,r1	/* CS8900 Address Register*/
	mov	#0x40,r0	/* CS8900 PP_EECMD (0x0040) */
	mov.w	r0,@r1

	mov.l	CS8900_DATA,r1	/* CS8900 Data Register*/
	mov.w	CS8900_D2,r0	/* CS8900 EEPROM offset 0x10 +  0x0200 */
	mov.w	r0,@r1

	/* wait_eeprom_ready()
             while(readreg(PP_SelfST) & SI_BUSY); 
           PP_SelfST=0x0136
	   SI_BUSY=0x100
	*/
wait:	
        mov.w   PP_SELFST,r6
        mov.l   CS8900_ADDR,r7
        mov.l   CS8900_DATA,r3
        mov.w   SI_BUSY,r2
wait2:	
        mov.w   r6,@r7
        mov.w   @r3,r4
        extu.w  r4,r0
        tst     r2,r0
        bf      wait2
	nop
				
	/* readreg(PP_EEData)
	   PP_EEData=0x42 */
	
	mov.l	CS8900_ADDR,r1	/* CS8900 Address Register*/
	mov	#0x42,r0	/* CS8900 Data3 (0x0042) */
	mov.w	r0,@r1

	mov.l	CS8900_DATA,r1	/* CS8900 Data Register*/
	mov.w	@r1,r0

	/* if(r0==0x10) then goto end */
	
	cmp/eq  #0x10,r0
	bt	end

	mov.l	MCR_A_32,r1	/* MCR Address */
	mov.w	MCR_D_32,r0	/* MCR Data for 32Mbyte */
	mov.w	r0,@r1

end:			
	/* writereg(PP_SelfCTL, readreg(PP_SelfCTL) | POWER_ON_RESET);
           PP_SelfCTL=0x0114
	   POWER_ON_RESET=0x0040 */

	mov.w   PP_SelfCTL,r2
	mov.l   CS8900_ADDR,r1
	mov.l   CS8900_DATA,r4
	mov.w   r2,@r1
	mov.w   @r4,r3
	extu.w  r3,r0
	or      #0x40,r0
	mov.w   r2,@r1
	mov.w   r0,@r4

	rts
	 nop

	.balign 4
MCR_A_32:
	.long	0xffffff68	/* MCR Address */
CS8900_ADDR:
	.long	0xb400000a	/* CS8900 Address */
CS8900_DATA:
	.long	0xb400000c	/* CS8900 Address */
L10000:	
	.long	10000
	
MCR_D_32:
	.word	0x116c
PP_SELFST:
	.word	0x136
SI_BUSY:
	.word	0x100
PP_SelfCTL:
	.word	0x114
POWER_ON_RESET:
	.word	0x0040
CS8900_D2:	
	.word	0x0200+0x10

