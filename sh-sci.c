/* $Id: sh-sci.c,v 1.48 2003/08/31 05:07:21 yoshii Exp $
 *
 * sh-ipl+g/sh-scif.c
 *
 * Support for Serial I/O using on chip SCI/SCIF of SuperH
 *
 *  Copyright (C) 1999  Takeshi Yaegachi & Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */

#include "config.h"
#include "io.h"

#if (defined(CONFIG_SCI) || defined(CONFIG_SCIF))
/* with serial-console */

#if defined(CONFIG_SCI)
#define SCSCR_INIT 	0x0030 /* TIE=0,RIE=0,TE=1,RE=1 */
#if defined(CONFIG_CPU_SH3)
#define SCSMR  (volatile unsigned char *)0xfffffe80
#define SCBRR  0xfffffe82
#define SCSCR  (volatile unsigned char *)0xfffffe84
#define SC_TDR  0xfffffe86
#define SC_SR  (volatile unsigned char *)0xfffffe88
#define SC_RDR  0xfffffe8a
#elif defined(CONFIG_CPU_SH4)
#define SCSMR	(volatile unsigned char *)0xffe00000
#define SCBRR	0xffe00004
#define SCSCR	(volatile unsigned char *)0xffe00008
#define SC_TDR	0xffe0000c
#define SC_SR	(volatile unsigned char *)0xffe00010
#define SC_RDR	0xffe00014
#endif
#elif defined(CONFIG_SCIF)
#define SCSCR_INIT 	0x0038 /* TIE=0,RIE=0,REIE=1,TE=1,RE=1 */
#if defined(CONFIG_CPU_SUBTYPE_SH7300)
/* Pin function controller */
/* SCIF */
#define SCPCR 	(volatile unsigned short *)0xA4050116
#define SCPDR   (volatile unsigned short *)0xA4050136

#define SCSMR  (volatile unsigned short *)0xA4430000
#define SCBRR  0xA4430004
#define SCSCR  (volatile unsigned short *)0xA4430008
#define SCTDSR (volatile unsigned char  *)0xA443000C
#define SCFER  (volatile unsigned short *)0xA4430010
#define SCSSR  (volatile unsigned short *)0xA4430014
#define SC_SR SCSSR
#define SCFCR  (volatile unsigned short *)0xA4430018
#define SCFDR  0xA443001C
#define SCFTDR 0xA4430020
#define SCFRDR 0xA4430024
#define SC_TDR SCFTDR
#define SC_RDR SCFRDR

#elif defined(CONFIG_CPU_SH3)
#define SCSMR  (volatile unsigned char *)0xA4000150
#define SCBRR  0xA4000152
#define SCSCR  (volatile unsigned char *)0xA4000154
#define SC_TDR 0xA4000156
#define SC_SR  (volatile unsigned short *)0xA4000158
#define SC_RDR 0xA400015A

#define SCFCR  (volatile unsigned char *)0xA400015C
#define SCFDR  0xA400015e

#define SCPCR  0xA4000116
#define SCPDR  0xA4000136

#elif defined(CONFIG_CPU_SH4)
#ifdef CONFIG_SCIF_TTY0
#define SCIF_BASE 0xFFE00000
#else
#define SCIF_BASE 0xFFE80000
#endif
#define SCSMR  (volatile unsigned short *)(SCIF_BASE + 0x00)
#define SCBRR  (SCIF_BASE + 0x04)
#define SCSCR  (volatile unsigned short *)(SCIF_BASE + 0x08)
#define SC_TDR (SCIF_BASE + 0x0C)
#define SC_SR  (volatile unsigned short *)(SCIF_BASE + 0x10)
#define SC_RDR (SCIF_BASE + 0x14)

#define SCSPTR (SCIF_BASE + 0x20)

#define SCFCR  (volatile unsigned short *)(SCIF_BASE + 0x18)
#define SCFDR  (SCIF_BASE + 0x1C)

#define SCLSR  (SCIF_BASE + 0x24)

#endif /* CONFIG_CPU_SH4 */
#endif /* CONFIG_SCIF */

#if defined(CONFIG_CPU_SUBTYPE_SH7300)
#define BPS_SETTING_VALUE	(((CONFIG_PCLK_FREQ*1.0)/(16*CONFIG_BPS))-0.5)
#else
#define BPS_SETTING_VALUE	(((CONFIG_PCLK_FREQ*1.0)/(32*CONFIG_BPS))-0.5)
#endif /* CONFIG_CPU_SUBTYPE_SH7300 */


#if defined(CONFIG_SE09)
#define SCSCR_CKE 3
#else
#define SCSCR_CKE 0
#endif

#if defined(CONFIG_SCI)

#define SCI_ER    0x0000
#define SCI_TD_E  0x0080
#define SCI_BRK   0x0020
#define SCI_FER   0x0010
#define SCI_PER   0x0008
#define SCI_RD_F  0x0040

#define SCI_TDRE_CLEAR		0x0078
#define SCI_RDRF_CLEAR		0x00bc
#define SCI_ERROR_CLEAR		0x00c4

#elif defined(CONFIG_SCIF)

#if defined(CONFIG_CPU_SUBTYPE_SH7300)
#define SCI_ER        (1<<7)
#define SCI_TEND      (1<<6)
#define SCI_TD_E      (1<<5)
#define SCI_BRK       (1<<4)
#define SCI_FER       (1<<3)
#define SCI_PER       (1<<2)
#define SCI_RD_F      (1<<1)
#define SCIF_ORER     (1<<9)

#define SCI_TDRE_CLEAR    ((~(SCI_TEND|SCI_TD_E))&0xffff)
#define SCI_RDRF_CLEAR    ((~(SCI_RD_F))&0xffff)
#define SCI_ERROR_CLEAR   0
#else
#define SCI_ER    0x0080
#define SCI_TD_E  0x0020
#define SCI_BRK   0x0010
#define SCI_FER   0x0008
#define SCI_PER   0x0004
#define SCI_RD_F  0x0003
#define SCIF_ORER 0x0001

#define SCI_TDRE_CLEAR		0x00df
#define SCI_RDRF_CLEAR		0x00fc
#define SCI_ERROR_CLEAR		0x0063
#define SCIF_ORERR_CLEAR	0x0000

#endif
#endif

#define WAIT_RFCR_COUNTER	500

void
handleError (void)
{
  p4_in(SC_SR);
  p4_out(SC_SR, SCI_ERROR_CLEAR);
#if defined(CONFIG_SCIF) && defined(CONFIG_CPU_SH4)
  p4_inw(SCLSR);
  p4_outw(SCLSR, SCIF_ORERR_CLEAR);
#endif
}

void
init_serial(void)
{
  p4_out(SCSCR, 0x0000);	/* TE=0, RE=0, CKE1=0 */
#if defined(CONFIG_SCIF)
  p4_out(SCFCR, 0x0006);	/* TFRST=1, RFRST=1 */
#endif
  p4_out(SCSCR, SCSCR_CKE);	/* TE=0, RE=0 */
  p4_out(SCSMR, 0x0000);	/* CHR=0, PE=0, STOP=0, CKS=00 */
  			/* 8-bit, non-parity, 1 stop bit, pf/1 clock */

  p4_outb(SCBRR, BPS_SETTING_VALUE);

  sleep128(1);			/* wait at least 1bit-time */

#if defined(CONFIG_SCIF)
#if defined(CONFIG_CPU_SH3)
  { /* For SH7709, SH7709A, SH7729 */
    unsigned short data;
    /* We need to set SCPCR to enable RTS/CTS */
    data = p4_inw(SCPCR);
    /* Clear out SCP7MD1,0, SCP4MD1,0,
       Set SCP6MD1,0 = {01} (output)  */
    p4_outw(SCPCR, (data&0x0cff)|0x1000);

    data = p4_inb(SCPDR);
    /* Set /RTS2 (bit6) = 0 */
    p4_outb(SCPDR, data&0xbf);
  }
#elif defined(CONFIG_CPU_SH4)
  p4_outw(SCSPTR, 0x0080); /* Set RTS = 1 */
#endif
  p4_out(SCFCR, 0x0000);	/* RTRG=00, TTRG=00 */
  				/* MCE=0,TFRST=0,RFRST=0,LOOP=0 */
#endif

  p4_out(SCSCR, SCSCR_INIT|SCSCR_CKE);
}

static inline int
getDebugCharReady (void)
{
  unsigned short status;

  status = p4_in(SC_SR);
  if (status & ( SCI_PER | SCI_FER | SCI_ER | SCI_BRK))
    handleError ();
#if defined(CONFIG_SCIF) && defined(CONFIG_CPU_SH4)
  if (p4_inw(SCLSR) & SCIF_ORER)
    handleError ();
#endif

  return (status & SCI_RD_F);
}

static void
delayX (void)
{
  /* XXX: Look RTC? */
  int i;

  for (i=0; i<256*10; i++)
    asm volatile ("": : : "memory");
}

int
getDebugCharTimeout (int count)
{
  unsigned short status;
  char ch;

  while (1)
    {
      if (getDebugCharReady())
	break;

      delayX ();
      if (--count == 0)
	return -1;
    }

  ch = p4_inb(SC_RDR);
  status = p4_in(SC_SR);
  p4_out(SC_SR, SCI_RDRF_CLEAR);

  if (status & (SCI_PER | SCI_FER | SCI_ER | SCI_BRK))
    handleError ();
#if defined(CONFIG_SCIF) && defined(CONFIG_CPU_SH4)
  if (p4_inw(SCLSR) & SCIF_ORER)
    handleError ();
#endif

  return ch;
}

char 
getDebugChar (void)
{
  unsigned short status;
  char ch;

  while ( ! getDebugCharReady())
    ;

  ch = p4_inb(SC_RDR);
  status = p4_in(SC_SR);
  p4_out(SC_SR, SCI_RDRF_CLEAR);

  if (status & (SCI_PER | SCI_FER | SCI_ER | SCI_BRK))
    handleError ();
#if defined(CONFIG_SCIF) && defined(CONFIG_CPU_SH4)
  if (p4_inw(SCLSR) & SCIF_ORER)
    handleError ();
#endif

  return ch;
}

static inline int 
putDebugCharReady (void)
{
  unsigned short status;

  status = p4_in(SC_SR);
  return (status & SCI_TD_E);
}

void
putDebugChar (char ch)
{
  while (!putDebugCharReady())
    ;

  p4_outb(SC_TDR, ch);
  p4_in(SC_SR);
  p4_out(SC_SR, SCI_TDRE_CLEAR);
}

void
putString (char *str)
{
  char *p;

  for (p = str; *p; p++)
    {
      if (*p == '\n')
	putDebugChar ('\r');
      putDebugChar (*p);
    }
}

#else /*if (defined(CONFIG_SCI) || defined(CONFIG_SCIF))*/
/* without serial-console */

void
handleError (void)
{
}

void
init_serial(void)
{
}

int
getDebugCharTimeout (int count)
{
	return 0;
}

char 
getDebugChar (void)
{
	return 0;
}

void
putDebugChar (char ch)
{
}

void
putString (char *str)
{
}

#endif /*if (defined(CONFIG_SCI) || defined(CONFIG_SCIF))*/
