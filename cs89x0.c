/*
 * Most part is taken from cs89x0.[ch] of etherboot 4.6.12
 */

/* cs89x0.c: A Crystal Semiconductor CS89[02]0 driver for etherboot. */
/*
  Permission is granted to distribute the enclosed cs89x0.[ch] driver
  only in conjunction with the Etherboot package.  The code is
  ordinarily distributed under the GPL.
  
  Russ Nelson, January 2000

  ChangeLog:

  Thu Dec 6 22:40:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * disabled all "advanced" features; this should make the code more reliable

  * reorganized the reset function

  * always reset the address port, so that autoprobing will continue working

  * some cosmetic changes

  * 2.5

  Thu Dec 5 21:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * tested the code against a CS8900 card

  * lots of minor bug fixes and adjustments

  * this is the first release, that actually works! it still requires some
    changes in order to be more tolerant to different environments

  * 4

  Fri Nov 22 23:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * read the manuals for the CS89x0 chipsets and took note of all the
    changes that will be neccessary in order to adapt Russel Nelson's code
    to the requirements of a BOOT-Prom

  * 6

  Thu Nov 19 22:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * Synched with Russel Nelson's current code (v1.00)

  * 2

  Thu Nov 12 18:00:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * Cleaned up some of the code and tried to optimize the code size.

  * 1.5

  Sun Nov 10 16:30:00 1996  Markus Gutschke  <gutschk@math.uni-muenster.de>

  * First experimental release. This code compiles fine, but I
  have no way of testing whether it actually works.

  * I did not (yet) bother to make the code 16bit aware, so for
  the time being, it will only work for Etherboot/32.

  * 12

  */

#include "config.h"
#include "string.h"
#include "defs.h"
#include "cs89x0.h"
#include "io.h"

#define ETH_ALEN		6	/* Size of Ethernet address */
#define ETHER_ADDR_SIZE		ETH_ALEN	/* deprecated */
#define ETH_HLEN		14	/* Size of ethernet header */
#define ETHER_HDR_SIZE		ETH_HLEN	/* deprecated */
#define ETH_MIN_PACKET		64
#define ETH_MAX_PACKET		1518

#define TICKS_PER_SEC		128

extern int printf(const char *fmt, ...);

static int cs89x0_probe(void);

static unsigned char	node_addr[ETHER_ADDR_SIZE];
static unsigned long	eth_nic_base;
static unsigned long    eth_mem_start;
static unsigned short   eth_irq;
static unsigned short   eth_cs_type;	/* one of: CS8900, CS8920, CS8920M  */
static unsigned short   eth_auto_neg_cnf;
static unsigned short   eth_adapter_cnf;
static unsigned short	eth_linectl;

/*************************************************************************
	CS89x0 - specific routines
**************************************************************************/

static inline int inw(unsigned long addr)
{
	return *(volatile unsigned short *)addr;
}

static inline void insw(int port, unsigned short *buf, int len)
{
	while(len--)
		*buf++ = *(volatile unsigned short *)port;
}

static inline void outb(unsigned short data, unsigned long addr)
{
	*(volatile unsigned char *)addr = data;
}

static inline void outw(unsigned short data, unsigned long addr)
{
	*(volatile unsigned short *)addr = data;
}

static inline void outsw(int port, unsigned short *data, int len)
{
	while(len--)
		*(volatile unsigned short *)port = *data++;
}

static inline int readreg(int portno)
{
	outw(portno, eth_nic_base + ADD_PORT);
	return inw(eth_nic_base + DATA_PORT);
}

static inline void writereg(int portno, int value)
{
	outw(portno, eth_nic_base + ADD_PORT);
	outw(value, eth_nic_base + DATA_PORT);
	return;
}

/*************************************************************************
EEPROM access
**************************************************************************/

static int wait_eeprom_ready(void)
{
	int st;
	unsigned long tmo;

	/* check to see if the EEPROM is ready, a timeout is used -
	   just in case EEPROM is ready when SI_BUSY in the
	   PP_SelfST is clear */
	tmo = get_tick() + TICKS_PER_SEC*100/1000;
	while((st = readreg(PP_SelfST)) & SI_BUSY) {
		if (get_tick() >= tmo) {
			printf("\n%s: TIMEOUT PP_SelfST=0x%x\n", __FUNCTION__, st);
			return -1;
		}
	}
	return 0;
}

static int get_eeprom_data(int off, int len, unsigned short *buffer)
{
	int i;

#ifdef	EDEBUG
	printf("\ncs: EEPROM data from %x for %x:",off,len);
#endif
	for (i = 0; i < len; i++) {
		if (wait_eeprom_ready() < 0)
			return -1;
		/* Now send the EEPROM read command and EEPROM location
		   to read */
		writereg(PP_EECMD, (off + i) | EEPROM_READ_CMD);
		if (wait_eeprom_ready() < 0)
			return -1;
		buffer[i] = readreg(PP_EEData);
#ifdef	EDEBUG
		if (!(i%10))
			printf("\ncs: ");
		printf("%x ", buffer[i]);
#endif
	}
#ifdef	EDEBUG
	printf("\n");
#endif

	return(0);
}

static int get_eeprom_chksum(int off, int len, unsigned short *buffer)
{
	int  i, cksum;

	cksum = 0;
	for (i = 0; i < len; i++)
		cksum += buffer[i];
	cksum &= 0xffff;
	if (cksum == 0)
		return 0;
	return -1;
}

/*************************************************************************
Activate all of the available media and probe for network
**************************************************************************/

static int detect_tp(void)
{
	unsigned long tmo;

	/* Turn on the chip auto detection of 10BT/ AUI */

        /* If connected to another full duplex capable 10-Base-T card
	   the link pulses seem to be lost when the auto detect bit in
	   the LineCTL is set.  To overcome this the auto detect bit
	   will be cleared whilst testing the 10-Base-T interface.
	   This would not be necessary for the sparrow chip but is
	   simpler to do it anyway. */
	writereg(PP_LineCTL, eth_linectl &~ AUI_ONLY);

        /* Delay for the hardware to work out if the TP cable is present - max 4sec */
	for (tmo = get_tick() + TICKS_PER_SEC*4; get_tick() < tmo; ) {
		if ((readreg(PP_LineST) & LINK_OK) != 0)
			break;
	}

	if ((readreg(PP_LineST) & LINK_OK) == 0)
		return 0;

	if (eth_cs_type != CS8900) {

		writereg(PP_AutoNegCTL, eth_auto_neg_cnf & AUTO_NEG_MASK);

		if ((eth_auto_neg_cnf & AUTO_NEG_BITS) == AUTO_NEG_ENABLE) {
			printf(" negotiating duplex... ");
			tmo = get_tick() + TICKS_PER_SEC*100/1000;
			while (readreg(PP_AutoNegST) & AUTO_NEG_BUSY) {
				if (get_tick() - tmo > TICKS_PER_SEC*40/1000) {
					printf("time out ");
					break;
				}
			}
		}
		if (readreg(PP_AutoNegST) & FDX_ACTIVE)
			printf("using full duplex");
		else
			printf("using half duplex");
	}

	return A_CNF_MEDIA_10B_T;
}

/**************************************************************************
ETH_RESET - Reset adapter
***************************************************************************/

static void reset_chip(void)
{
	unsigned long tmo;

	writereg(PP_SelfCTL, readreg(PP_SelfCTL) | POWER_ON_RESET);

	/* wait 30ms */
	for(tmo = get_tick()+TICKS_PER_SEC*30/1000; get_tick() < tmo; )
		asm volatile("":::"memory");

	if (eth_cs_type != CS8900) {
		/* Hardware problem requires PNP registers to be reconfigured
		   after a reset */
		if (eth_irq != 0xFFFF) {
			outw(PP_CS8920_ISAINT, eth_nic_base + ADD_PORT);
			outb(eth_irq, eth_nic_base + DATA_PORT);
			outb(0, eth_nic_base + DATA_PORT + 1); }

		if (eth_mem_start) {
			outw(PP_CS8920_ISAMemB, eth_nic_base + ADD_PORT);
			outb((eth_mem_start >> 8) & 0xff, eth_nic_base + DATA_PORT);
			outb((eth_mem_start >> 24) & 0xff, eth_nic_base + DATA_PORT + 1); } }

	/* Wait until the chip is reset */
	for (tmo = get_tick() + TICKS_PER_SEC*100/1000;
	     (readreg(PP_SelfST) & INIT_DONE) == 0 &&
		     get_tick() < tmo; );

	/* disable interrupts and memory accesses */
	writereg(PP_BusCTL, 0);

	wait_eeprom_ready();
}
int
eth_reset(unsigned int start_or_stop)
{
	int  i;

	if (start_or_stop == 1)	/* STOP */
		return 0;	/* XXX: Not implemented yet. */

	if (eth_nic_base == 0) { /* not initialized yet */
		if (cs89x0_probe() == 0) { /* not found */
			printf("cs89x0: device initialization failed.\n");
			return 0;
		}
	}

	/* set the ethernet address */
	for (i=0; i < ETHER_ADDR_SIZE/2; i++)
		writereg(PP_IA+i*2,
			 (int)node_addr[i*2] |
			 ((int)node_addr[i*2+1] << 8));

	/* receive only error free packets addressed to this card */
	writereg(PP_RxCTL, DEF_RX_ACCEPT);

	/* do not generate any interrupts on receive operations */
	writereg(PP_RxCFG, 0);

	/* do not generate any interrupts on transmit operations */
	writereg(PP_TxCFG, 0);

	/* do not generate any interrupts on buffer operations */
	writereg(PP_BufCFG, 0);

	/* reset address port, so that autoprobing will keep working */
	outw(PP_ChipID, eth_nic_base + ADD_PORT);

	return 0;
}

/**************************************************************************
ETH_TRANSMIT - Transmit a frame
***************************************************************************/

int
eth_transmit(
	const char *d,			/* Destination */
	unsigned int t,			/* Type */
	unsigned int s,			/* size */
	const char *p)			/* Packet */
{
	int           sr;
	unsigned long tmo;

	/* does this size have to be rounded??? please,
	   somebody have a look in the specs */
	if ((sr = ((s + ETHER_HDR_SIZE + 1)&~1)) < ETH_MIN_PACKET)
		sr = ETH_MIN_PACKET;

retry:
	/* initiate a transmit sequence */
	outw(TX_AFTER_ALL, eth_nic_base + TX_CMD_PORT);
	outw(sr, eth_nic_base + TX_LEN_PORT);

	/* Test to see if the chip has allocated memory for the packet */
	if ((readreg(PP_BusST) & READY_FOR_TX_NOW) == 0) {
		/* Oops... this should not happen! */
		for (tmo = get_tick() + TICKS_PER_SEC*100/1000; get_tick() < tmo; )
			asm volatile("":::"memory");
		goto retry; }

	/* Write the contents of the packet */
	outsw(eth_nic_base + TX_FRAME_PORT, (unsigned short *)d, ETHER_ADDR_SIZE/2);
	outsw(eth_nic_base + TX_FRAME_PORT, (unsigned short *)&node_addr[0],
	      ETHER_ADDR_SIZE/2);
	outw(((t >> 8)&0xFF)|(t << 8), eth_nic_base + TX_FRAME_PORT);
	outsw(eth_nic_base + TX_FRAME_PORT, (unsigned short *)p, (s+1)/2);
	for (sr = sr/2 - (s+1)/2 - ETHER_ADDR_SIZE - 1; sr-- > 0;
	     outw(0, eth_nic_base + TX_FRAME_PORT));

	/* wait for transfer to succeed */
	for (tmo = get_tick() + TICKS_PER_SEC;
	     (s = readreg(PP_TxEvent)&~0x1F) == 0 && get_tick() < tmo;)
		asm volatile("":::"memory") ;
	if ((s & TX_SEND_OK_BITS) != TX_OK) {
		printf("\ntransmission error %#x\n", s);
	}

	return 0;
}

/**************************************************************************
ETH_POLL - Wait for a frame
***************************************************************************/

int
eth_receive (char *packet, unsigned int *len_p)
{
	int status, packetlen;

	status = readreg(PP_RxEvent);

	if ((status & RX_OK) == 0)
		return(0);

	status = inw(eth_nic_base + RX_FRAME_PORT);
	packetlen = inw(eth_nic_base + RX_FRAME_PORT);
	insw(eth_nic_base + RX_FRAME_PORT, (unsigned short *)packet, packetlen >> 1);
	if (packetlen & 1)
		packet[packetlen-1] = inw(eth_nic_base + RX_FRAME_PORT);
	if(len_p)
		*len_p = packetlen;
	return 1;
}

/**************************************************************************
ETH_PROBE - Look for an adapter
***************************************************************************/

static int cs89x0_probe(void)
{
	static const unsigned int netcard_portlist[] = {
#ifdef	CS_SCAN
		CS_SCAN,
#else	/* use "conservative" default values for autoprobing */
		0x300,0x320,0x340,0x200,0x220,0x240,
		0x260,0x280,0x2a0,0x2c0,0x2e0,
	/* if that did not work, then be more aggressive */
		0x301,0x321,0x341,0x201,0x221,0x241,
		0x261,0x281,0x2a1,0x2c1,0x2e1,
#endif
		0};

	int      i, result = -1;
	unsigned rev_type = 0, ioaddr, ioidx, cs_revision;
	unsigned short eeprom_buff[CHKSUM_LEN];


	for (ioidx = 0; (ioaddr=netcard_portlist[ioidx++]) != 0;) {
		/* if they give us an odd I/O address, then do ONE write to
		   the address port, to get it back to address zero, where we
		   expect to find the EISA signature word. */
		if (ioaddr & 1) {
			ioaddr &= ~1;
			if ((inw(ioaddr + ADD_PORT) & ADD_MASK) != ADD_SIG)
				continue;
			outw(PP_ChipID, ioaddr + ADD_PORT);
		}

		if (inw(ioaddr + DATA_PORT) != CHIP_EISA_ID_SIG)
			continue;
		eth_nic_base = ioaddr;

		/* get the chip type */
		rev_type = readreg(PRODUCT_ID_ADD);
		eth_cs_type = rev_type &~ REVISON_BITS;
		cs_revision = ((rev_type & REVISON_BITS) >> 8) + 'A';

		printf("\ncs: cs89%c0%s rev %c, base %#lx",
		       eth_cs_type==CS8900?'0':'2',
		       eth_cs_type==CS8920M?"M":"",
		       cs_revision,
		       eth_nic_base);

		reset_chip();

	        /* Here we read the current configuration of the chip. If there
		   is no Extended EEPROM then the idea is to not disturb the chip
		   configuration, it should have been correctly setup by automatic
		   EEPROM read on reset. So, if the chip says it read the EEPROM
		   the driver will always do *something* instead of complain that
		   adapter_cnf is 0. */
	        if ((readreg(PP_SelfST) & (EEPROM_OK | EEPROM_PRESENT)) == 
		      (EEPROM_OK|EEPROM_PRESENT)) {
		        /* Load the MAC. */
			for (i=0; i < ETH_ALEN/2; i++) {
		                unsigned int Addr;
				Addr = readreg(PP_IA+i*2);
			        node_addr[i*2] = Addr & 0xFF;
			        node_addr[i*2+1] = Addr >> 8;
			}

		   	/* Load the Adapter Configuration. 
			   Note:  Barring any more specific information from some 
			   other source (ie EEPROM+Schematics), we would not know 
			   how to operate a 10Base2 interface on the AUI port. 
			   However, since we  do read the status of HCB1 and use 
			   settings that always result in calls to control_dc_dc(dev,0) 
			   a BNC interface should work if the enable pin 
			   (dc/dc converter) is on HCB1. It will be called AUI 
			   however. */

			eth_adapter_cnf = 0;
			i = readreg(PP_LineCTL);
			/* Preserve the setting of the HCB1 pin. */
			if ((i & (HCB1 | HCB1_ENBL)) ==  (HCB1 | HCB1_ENBL))
				eth_adapter_cnf |= A_CNF_DC_DC_POLARITY;
			/* Save the sqelch bit */
			if ((i & LOW_RX_SQUELCH) == LOW_RX_SQUELCH)
				eth_adapter_cnf |= A_CNF_EXTND_10B_2 | A_CNF_LOW_RX_SQUELCH;
			/* Check if the card is in 10Base-t only mode */
			if ((i & (AUI_ONLY | AUTO_AUI_10BASET)) == 0)
				eth_adapter_cnf |=  A_CNF_10B_T | A_CNF_MEDIA_10B_T;
			/* Check if the card is in AUI only mode */
			if ((i & (AUI_ONLY | AUTO_AUI_10BASET)) == AUI_ONLY)
				eth_adapter_cnf |=  A_CNF_AUI | A_CNF_MEDIA_AUI;
			/* Check if the card is in Auto mode. */
			if ((i & (AUI_ONLY | AUTO_AUI_10BASET)) == AUTO_AUI_10BASET)
				eth_adapter_cnf |=  A_CNF_AUI | A_CNF_10B_T | 
				A_CNF_MEDIA_AUI | A_CNF_MEDIA_10B_T | A_CNF_MEDIA_AUTO;

			printf("cs: PP_LineCTL=0x%x, adapter_cnf=0x%x\n",
					i, eth_adapter_cnf);

			printf( "[Cirrus EEPROM] ");
		}

	        printf("\n");

		/* First check to see if an EEPROM is attached. */
		if ((readreg(PP_SelfST) & EEPROM_PRESENT) == 0) {
			printf("cs: No EEPROM....\n");
			continue;
		}
		else if (get_eeprom_data(START_EEPROM_DATA,CHKSUM_LEN,eeprom_buff) < 0) {
			printf("\ncs: EEPROM read failed.\n");
			continue;
	        } else if (get_eeprom_chksum(START_EEPROM_DATA,CHKSUM_LEN,eeprom_buff) < 0) {
			/* Check if the chip was able to read its own configuration starting
			   at 0 in the EEPROM*/
			if ((readreg(PP_SelfST) & (EEPROM_OK | EEPROM_PRESENT)) !=
			    (EEPROM_OK|EEPROM_PRESENT)) {
	                	printf("cs: Extended EEPROM checksum bad and no Cirrus EEPROM\n");
	                	continue;
	                }

	        } else {
			/* This reads an extended EEPROM that is not documented
			   in the CS8900 datasheet. */
			
	                /* get transmission control word  but keep the autonegotiation bits */
	                if (!eth_auto_neg_cnf) eth_auto_neg_cnf = eeprom_buff[AUTO_NEG_CNF_OFFSET/2];
	                /* Store adapter configuration */
	                if (!eth_adapter_cnf) eth_adapter_cnf = eeprom_buff[ADAPTER_CNF_OFFSET/2];
	                /* Store ISA configuration */
	                eth_mem_start = eeprom_buff[PACKET_PAGE_OFFSET/2] << 8;

	                /* eeprom_buff has 32-bit ints, so we can't just memcpy it */
	                /* store the initial memory base address */
	                for (i = 0; i < ETH_ALEN/2; i++) {
	                        node_addr[i*2] = eeprom_buff[i];
	                        node_addr[i*2+1] = eeprom_buff[i] >> 8;
	                }
	        }

		printf("cs89x0 media %s%s%s",
		       (eth_adapter_cnf & A_CNF_10B_T)?"RJ-45,":"",
		       (eth_adapter_cnf & A_CNF_AUI)?"AUI,":"",
		       (eth_adapter_cnf & A_CNF_10B_2)?"BNC,":"");

		printf("programmed I/O");

		/* print the ethernet address. */
		printf(", MAC ");
		for (i = 0; i < ETH_ALEN; i++)
			printf("%s%02x", i ? ":" : "", node_addr[i]);
		printf("\n");

		/* Set the LineCTL quintuplet based on adapter
		   configuration read from EEPROM */
		if ((eth_adapter_cnf & A_CNF_EXTND_10B_2) &&
		    (eth_adapter_cnf & A_CNF_LOW_RX_SQUELCH))
			eth_linectl = LOW_RX_SQUELCH;
		else
			eth_linectl = 0;

		/* check to make sure that they have the "right"
		   hardware available */
		switch(eth_adapter_cnf & A_CNF_MEDIA_TYPE) {
		case A_CNF_MEDIA_10B_T: result = eth_adapter_cnf & A_CNF_10B_T;
			break;
		case A_CNF_MEDIA_AUI:   result = eth_adapter_cnf & A_CNF_AUI;
			break;
		case A_CNF_MEDIA_10B_2: result = eth_adapter_cnf & A_CNF_10B_2;
			break;
		default: result = eth_adapter_cnf & (A_CNF_10B_T | A_CNF_AUI |
						     A_CNF_10B_2);
		}
		if (!result) {
			printf("cs: EEPROM is configured for unavailable media\n");
		error:
			writereg(PP_LineCTL, readreg(PP_LineCTL) &
				 ~(SERIAL_TX_ON | SERIAL_RX_ON));
			outw(PP_ChipID, eth_nic_base + ADD_PORT);
			continue;
		}

		/* set the hardware to the configured choice */
		switch(eth_adapter_cnf & A_CNF_MEDIA_TYPE) {
		case A_CNF_MEDIA_10B_T:
			result = detect_tp();
			if (!result) {
				printf("10Base-T (RJ-45) has no cable\n"); }
			/* check "ignore missing media" bit */
			if (eth_auto_neg_cnf & IMM_BIT)
				/* Yes! I don't care if I see a link pulse */
				result = A_CNF_MEDIA_10B_T;
			break;
		case A_CNF_MEDIA_AUTO:
			writereg(PP_LineCTL, eth_linectl | AUTO_AUI_10BASET);
			if (eth_adapter_cnf & A_CNF_10B_T)
				if ((result = detect_tp()) != 0)
					break;
			printf("no media detected\n");
			goto error;
		}
		switch(result) {
		case 0:                 printf("no network cable attached to configured media\n");
			goto error;
		case A_CNF_MEDIA_10B_T: printf("using 10Base-T (RJ-45)\n");
			break;
		case A_CNF_MEDIA_AUI:   printf("using 10Base-5 (AUI)\n");
			break;
		case A_CNF_MEDIA_10B_2: printf("using 10Base-2 (BNC)\n");
			break;
		}

		/* Turn on both receive and transmit operations */
		writereg(PP_LineCTL, readreg(PP_LineCTL) | SERIAL_RX_ON |
			 SERIAL_TX_ON);
		return 1;
	}
	return 0;
}

int
eth_node_addr (unsigned int func, char *p)
{
	 if (func != 0) /* Only "get" is supported */
		return -1;

	if (eth_nic_base == 0) { /* not initialized yet */
		if (cs89x0_probe() == 0) { /* not found */
			printf("cs89x0: device initialization failed.\n");
			return -1;
		}
	}
	memcpy (p, node_addr, ETHER_ADDR_SIZE);
	return 0;
}

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */

