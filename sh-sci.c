/* $Id: sh-sci.c,v 1.45 2001/06/06 12:29:07 sugioka Exp $
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

#if defined(CONFIG_SCI)
#define SCSCR_INIT 	0x0030 /* TIE=0,RIE=0,TE=1,RE=1 */
#if defined(__sh3__)
#define SCSMR  (volatile unsigned char *)0xfffffe80
#define SCBRR  0xfffffe82
#define SCSCR  (volatile unsigned char *)0xfffffe84
#define SC_TDR  0xfffffe86
#define SC_SR  (volatile unsigned char *)0xfffffe88
#define SC_RDR  0xfffffe8a
#elif defined(__SH4__)
#define SCSMR	(volatile unsigned char *)0xffe00000
#define SCBRR	0xffe00004
#define SCSCR	(volatile unsigned char *)0xffe00008
#define SC_TDR	0xffe0000c
#define SC_SR	(volatile unsigned char *)0xffe00010
#define SC_RDR	0xffe00014
#endif
#elif defined(CONFIG_SCIF)
#define SCSCR_INIT 	0x0038 /* TIE=0,RIE=0,REIE=1,TE=1,RE=1 */
#if defined(__sh3__)
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

#elif defined(__SH4__)
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

#endif /* __SH4__ */
#endif /* CONFIG_SCIF */

#if defined(__sh3__)

#define RFCR    0xffffff74
#if defined(CONFIG_SESH3)
#define BPS_SETTING_VALUE	8 /* 8: 115200 bps */
#elif defined(CONFIG_HP600)
#define BPS_SETTING_VALUE	5
#elif defined(CONFIG_CAT68701)
#define BPS_SETTING_VALUE       8 /* 115200 bps */ 
#elif defined(CONFIG_SH2000)
#define BPS_SETTING_VALUE       7 /* 115200 bps */ 
#else
#define BPS_SETTING_VALUE	3 /* 3: 115200 bps */
#endif

#elif defined(__SH4__)

#define RFCR    0xFF800028
#if defined(CONFIG_APSH4)
#define BPS_SETTING_VALUE	6 /* 6: 115200 bps */
#elif defined(CONFIG_DREAMCAST)
#define BPS_SETTING_VALUE	13 /* 13: 115200 bps */
#elif defined(CONFIG_SESH4) && defined(CONFIG_CPU_SUBTYPE_SH7751)
#define BPS_SETTING_VALUE	6 /* 6: 115200 bps */
#else
#define BPS_SETTING_VALUE	8 /* 3: 230400 bps */
				  /* 8: 115200 bps */
				  /* 54: 19200 bps */
				  /* 108: 9600 */
#endif
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

#define WAIT_RFCR_COUNTER	500

void
handleError (void)
{
  p4_in(SC_SR);
  p4_out(SC_SR, SCI_ERROR_CLEAR);
#if defined(CONFIG_SCIF) && defined(__SH4__)
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
  p4_out(SCSMR, 0x0000);	/* CHR=0, PE=0, STOP=0, CKS=00 */
  			/* 8-bit, non-parity, 1 stop bit, pf/1 clock */

  p4_outb(SCBRR, BPS_SETTING_VALUE);

  p4_outw(RFCR, 0xa400);		/* Refresh counter clear */
  while(p4_inw(RFCR) < WAIT_RFCR_COUNTER)
    ;

#if defined(CONFIG_SCIF)
#if defined(__sh3__)
  { /* For SH7709, SH7709A, SH7729 */
    unsigned short data;
    /* We need to set SCPCR to enable RTS/CTS */
    data = p4_inw(SCPCR);
    /* Clear out SCP7MD1,0, SCP4MD1,0,
       Set SCP6MD1,0 = {01} (output)  */
    p4_outw(SCPCR, (data&0x0fcf)|0x1000);

    data = p4_inb(SCPDR);
    /* Set /RTS2 (bit6) = 0 */
    p4_outb(SCPDR, data&0xbf);
  }
#elif defined(__SH4__)
  p4_outw(SCSPTR, 0x0080); /* Set RTS = 1 */
#endif
  p4_out(SCFCR, 0x0000);	/* RTRG=00, TTRG=00 */
  				/* MCE=0,TFRST=0,RFRST=0,LOOP=0 */
#endif

  p4_out(SCSCR, SCSCR_INIT);
}

static inline int
getDebugCharReady (void)
{
  unsigned short status;

  status = p4_in(SC_SR);
  if (status & ( SCI_PER | SCI_FER | SCI_ER | SCI_BRK))
    handleError ();
#if defined(CONFIG_SCIF) && defined(__SH4__)
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
#if defined(CONFIG_SCIF) && defined(__SH4__)
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
#if defined(CONFIG_SCIF) && defined(__SH4__)
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
