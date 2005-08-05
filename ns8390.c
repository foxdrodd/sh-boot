/*
 * Most part is taken from ns8390.[ch] of etherboot 4.6.10
 */

/**************************************************************************
ETHERBOOT -  BOOTP/TFTP Bootstrap Program

Author: Martin Renters
  Date: May/94

 This code is based heavily on David Greenman's if_ed.c driver

 Copyright (C) 1993-1994, David Greenman, Martin Renters.
  This software may be used, modified, copied, distributed, and sold, in
  both source and binary form provided that the above copyright and these
  terms are retained. Under no circumstances are the authors responsible for
  the proper functioning of this software, nor do the authors assume any
  responsibility for damages incurred with its use.

3c503 support added by Bill Paul (wpaul@ctr.columbia.edu) on 11/15/94
SMC8416 support added by Bill Paul (wpaul@ctr.columbia.edu) on 12/25/94
3c503 PIO support added by Jim Hague (jim.hague@acm.org) on 2/17/98
RX overrun by Klaus Espenlaub (espenlaub@informatik.uni-ulm.de) on 3/10/99
  parts taken from the Linux 8390 driver (by Donald Becker and Paul Gortmaker)

**************************************************************************/

#include "config.h"
#include "string.h"
#include "defs.h"

#ifdef CONFIG_STNIC
#include "stnic-se.h"
#elif defined(CONFIG_NE2000)
#include "ne.h"
#endif

/*
 * TODO: big endian support
 */

#define ETH_MIN_PACKET		64

#define D8390_P0_COMMAND	0x0000
#define D8390_P0_PSTART		0x0001
#define D8390_P0_PSTOP		0x0002
#define D8390_P0_BOUND		0x0003
#define	D8390_P0_TPSR		0x0004
#define D8390_P0_TBCR0		0x0005
#define D8390_P0_TBCR1		0x0006
#define D8390_P0_ISR		0x0007
#define D8390_P0_RSAR0		0x0008
#define D8390_P0_RSAR1		0x0009
#define D8390_P0_RBCR0		0x000A
#define D8390_P0_RBCR1		0x000B
#define D8390_P0_RSR		0x000C
#define D8390_P0_RCR		0x000C
#define D8390_P0_TCR		0x000D
#define D8390_P0_DCR		0x000E
#define D8390_P0_IMR		0x000F

#define D8390_P1_PAR0		0x0001
#define D8390_P1_CURR		0x0007
#define D8390_P1_MAR0		0x0008

/*
 *
 */
#define D8390_COMMAND_PS0	0x00	/* Page 0 select */
#define D8390_COMMAND_PS1	0x40	/* Page 1 select */
#define D8390_COMMAND_RD0	0x08
#define D8390_COMMAND_RD1	0x10
#define	D8390_COMMAND_RD2	0x20	/* Remote DMA control */
#define D8390_COMMAND_STA	0x02	/* Start */
#define D8390_COMMAND_STP	0x01	/* Stop */
#define D8390_COMMAND_TXP	0x04	/* Transmit packet */

#define D8390_RSTAT_PRX		0x01	/* Successful receive */

#define D8390_ISR_OVW		0x10	/* Overflow */
#define D8390_ISR_RDC		0x40	/* Remote DMA complete */
#define D8390_ISR_RST		0x80	/* reset */

#define SWAPUS(uslval) uslval = (((uslval) >> 8) | ((uslval) << 8))

#if defined(__BIG_ENDIAN__)
#define ENDCFG_BOS 0x02
#else				/* must be LITTLE */
#define ENDCFG_BOS 0x00
#endif				/* LITTLE_ENDIAN */

/* Use 0 (byte) or 1 (word) */
#define ENDCFG_WTS 0x01

/* Use 4-word fifo trigger, no loopback, other options */
#define ENDCFG (0x48 | ENDCFG_BOS | ENDCFG_WTS)

/*
 *
 */
#define TX_PAGES	12
#define RX_START	(START_PG+TX_PAGES)

#define ETHER_ADDR_SIZE 	 6
#define ETHER_HDR_SIZE		14
static unsigned char node_addr[ETHER_ADDR_SIZE];

#ifdef CONFIG_STNIC
static const unsigned char node_addr_template[ETHER_ADDR_SIZE]
    = CONFIG_ETHER_ADDRESS_ARRAY;

static inline void set_node_addr(void)
{
	memcpy(node_addr, node_addr_template, ETHER_ADDR_SIZE);
	memcpy(node_addr + 4, (char *)CONFIG_ETHER_CONFIG_WORD, 2);
}
#else
static void set_node_addr(void)
{
	int i;
	unsigned char SA_prom[32];
	static const struct {
		unsigned char value, offset;
	} program_seq[] = {
		{
		D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STP, D8390_P0_COMMAND},	/* Select page 0 */
		{
		0x48, D8390_P0_DCR},	/* Set byte-wide (0x48) access. */
		{
		0x00, D8390_P0_RBCR0},	/* Clear the count regs. */
		{
		0x00, D8390_P0_RBCR1}, {
		0x00, D8390_P0_IMR},	/* Mask completion irq. */
		{
		0xFF, D8390_P0_ISR}, {
		0x20, D8390_P0_RCR},	/* 0x20  Set to monitor */
		{
		0x02, D8390_P0_TCR},	/* 0x02  and loopback mode. */
		{
		32, D8390_P0_RBCR0}, {
		0x00, D8390_P0_RBCR1}, {
		0x00, D8390_P0_RSAR0},	/* DMA starting at 0x0000. */
		{
		0x00, D8390_P0_RSAR1}, {
	D8390_COMMAND_PS0 | D8390_COMMAND_RD0 |
			    D8390_COMMAND_STA, D8390_P0_COMMAND},};

	nic_reset();

	while ((inb(D8390_P0_ISR) & D8390_ISR_RST) == 0) ;
	outb(0xff, D8390_P0_ISR);	/* Ack all intr. */

	for (i = 0; i < sizeof(program_seq) / sizeof(program_seq[0]); i++)
		outb(program_seq[i].value, program_seq[i].offset);

	for (i = 0; i < 32; i += 2) {
		SA_prom[i] = inb(D8390_REMOTE_IO);
		SA_prom[i + 1] = inb(D8390_REMOTE_IO);
		if (SA_prom[i] != SA_prom[i + 1])
			return;	/* XXX: Not NE2000! */
	}

	for (i = 0; i < ETHER_ADDR_SIZE; i++)
		node_addr[i] = SA_prom[i * 2];

	/* We must set the 8390 for word mode. */
	outb(ENDCFG, D8390_P0_DCR);
}
#endif

int eth_reset(unsigned int start_or_stop)
{
	int i;

	if (start_or_stop == 1)	/* STOP */
		return 0;	/* XXX: Not implemented yet. */

	/* Initialize */
	nic_reset();

	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STP,
	     D8390_P0_COMMAND);

	outb(ENDCFG, D8390_P0_DCR);

	/* Clear the remote byte count registers. */
	outb(0, D8390_P0_RBCR0);
	outb(0, D8390_P0_RBCR1);

	/* Set to monitor and loopback mode. */
	outb(0x20, D8390_P0_RCR);
	outb(2, D8390_P0_TCR);

	/* Set the transmit page and receive ring. */
	outb(START_PG, D8390_P0_TPSR);
	outb(RX_START, D8390_P0_PSTART);
	outb(STOP_PG, D8390_P0_PSTOP);
	outb(STOP_PG - 1, D8390_P0_BOUND);

	/* Clear the pending interrupts and mask. */
	outb(0xFF, D8390_P0_ISR);
	outb(0, D8390_P0_IMR);

	outb(D8390_COMMAND_PS1 | D8390_COMMAND_RD2 | D8390_COMMAND_STP,
	     D8390_P0_COMMAND);

	for (i = 0; i < ETHER_ADDR_SIZE; i++)
		outb(node_addr[i], D8390_P1_PAR0 + i);
	for (i = 0; i < ETHER_ADDR_SIZE; i++)
		outb(0xFF, D8390_P1_MAR0 + i);

	outb(RX_START, D8390_P1_CURR);
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);
	outb(0xFF, D8390_P0_ISR);

	outb(0, D8390_P0_TCR);	/* TXCONFIG=0 xmit on */
	outb(4, D8390_P0_RCR);	/* allow broadcast frames *//* RXCONFIG=4 recv on */

	return 0;
}

struct ringbuffer {
	unsigned char status;
	unsigned char next;
	unsigned short len;
};

static void eth_pio_read(unsigned int src, unsigned char *dst, unsigned int cnt)
{
	++cnt;
	cnt &= ~1;

	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);
	outb(cnt, D8390_P0_RBCR0);
	outb(cnt >> 8, D8390_P0_RBCR1);
	outb(src, D8390_P0_RSAR0);
	outb(src >> 8, D8390_P0_RSAR1);
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD0 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);

	cnt >>= 1;

	while (cnt--) {
		unsigned short data = inw(D8390_REMOTE_IO);
		if (dst) {
			*((unsigned short *)dst) = data;
			dst += 2;
		}
	}

	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);
}

static int
eth_pio_write(const unsigned char *src, unsigned int dst, unsigned int cnt)
{
	int i;
	++cnt;
	cnt &= ~1;

	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);
	delay();

	outb(D8390_ISR_RDC, D8390_P0_ISR);
	outb(cnt, D8390_P0_RBCR0);
	outb(cnt >> 8, D8390_P0_RBCR1);
	outb(dst, D8390_P0_RSAR0);
	outb(dst >> 8, D8390_P0_RSAR1);
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD1 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);

	cnt >>= 1;

	while (cnt--) {
		outw(*((unsigned short *)src), D8390_REMOTE_IO);
		src += 2;
	}

	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);

#define PIO_WRITE_TIMEOUT	100000
	for (i = 0; i < PIO_WRITE_TIMEOUT; i++)
		if ((inb(D8390_P0_ISR) & D8390_ISR_RDC)) {
			/* Acknowledge RDC interrupt */
			outb(D8390_ISR_RDC, D8390_P0_ISR);
			return 0;
		} else
			delay();

	/* Timeout */
	eth_reset(0);
	return -1;
}

static int eth_receive_1(char *p, int *len_p)
{
	unsigned char rstat, curr, next;
	unsigned short len, frag;
	unsigned short pktoff;
	struct ringbuffer pkthdr;

	rstat = inb(D8390_P0_RSR);
	if (!(rstat & D8390_RSTAT_PRX))
		return 0;

	next = inb(D8390_P0_BOUND) + 1;
	if (next >= STOP_PG)
		next = RX_START;

	outb(D8390_COMMAND_PS1 | D8390_COMMAND_RD2, D8390_P0_COMMAND);
	curr = inb(D8390_P1_CURR);
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2, D8390_P0_COMMAND);
	if (curr >= STOP_PG)
		curr = RX_START;
	if (curr == next)
		return 0;

	pktoff = next << 8;
	eth_pio_read(pktoff, (char *)&pkthdr, 4);
	pktoff += sizeof(pkthdr);

#if defined(__BIG_ENDIAN__)
	SWAPUS(*(unsigned short *)&pkthdr.status);
	SWAPUS(pkthdr.len);
#endif				/* BIG_ENDIAN */

	len = pkthdr.len - 4;	/* sub CRC */

	if ((pkthdr.status & D8390_RSTAT_PRX) == 0
	    || pkthdr.len < ETH_MIN_PACKET)
		return 0;	/* Just ignore bogus packet */
	else {
		if (len_p)
			*len_p = len;	/* available to caller */
		frag = (STOP_PG << 8) - pktoff;
		if (len > frag) {	/* We have a wrap-around */
			/* read first part */
			eth_pio_read(pktoff, p, frag);
			pktoff = RX_START << 8;
			p += frag;
			len -= frag;
		}

		/* read second part */
		eth_pio_read(pktoff, p, len);
	}

	next = pkthdr.next;	/* frame number of next packet */
	if (next == RX_START)
		next = STOP_PG;

	outb(next - 1, D8390_P0_BOUND);

	return 1;
}

static void eth_rx_overrun(void)
{
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STP,
	     D8390_P0_COMMAND);

	/* Wait for at least 1.6ms */
	sleep128(2);		/* 15.625 ms */

	outb(0, D8390_P0_RBCR0);	/* Reset byte counter */
	outb(0, D8390_P0_RBCR1);

	/* Enter loopback mode and restart NIC. */
	outb(2, D8390_P0_TCR);
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);

	/* Clear the RX ring */
	while (eth_receive_1(0, 0))
		/* Nothing */ ;

	/* Acknowledge overrun interrupt */
	outb(D8390_ISR_OVW, D8390_P0_ISR);

	/* Leave loopback mode - no packets to be resent */
	outb(0, D8390_P0_TCR);
}

int eth_receive(char *p, unsigned int *len_p)
{
	if ((inb(D8390_P0_ISR) & D8390_ISR_OVW)) {
		eth_rx_overrun();
		return 0;
	}

	return eth_receive_1(p, len_p);
}

/* We abuses remcomOutBuffer */
#define ETH_MAX_PACKET 1546
#define pkt remcomInBuffer

int
eth_transmit(const char *dst, unsigned int type,
	     unsigned int size, const char *p)
{
	unsigned short t;

	t = type;
#if !defined(__BIG_ENDIAN__)
	SWAPUS(t);
#endif				/* !__BIG_ENDIAN__ */
	memcpy(pkt, dst, ETHER_ADDR_SIZE);
	memcpy(pkt + ETHER_ADDR_SIZE, node_addr, ETHER_ADDR_SIZE);
	memcpy(pkt + 2 * ETHER_ADDR_SIZE, &t, 2);
	memcpy(pkt + 2 * ETHER_ADDR_SIZE + 2, p, size);
	if (eth_pio_write(pkt, START_PG << 8, ETHER_HDR_SIZE + size) < 0)
		return -1;
	size += ETHER_HDR_SIZE;
	if (size < ETH_MIN_PACKET)
		size = ETH_MIN_PACKET;

	outb(D8390_COMMAND_PS0 | D8390_COMMAND_RD2 | D8390_COMMAND_STA,
	     D8390_P0_COMMAND);
	outb(START_PG, D8390_P0_TPSR);
	outb(size, D8390_P0_TBCR0);
	outb(size >> 8, D8390_P0_TBCR1);
	outb(D8390_COMMAND_PS0 | D8390_COMMAND_TXP | D8390_COMMAND_RD2 |
	     D8390_COMMAND_STA, D8390_P0_COMMAND);

	return 0;
}

int eth_node_addr(unsigned int func, char *p)
{
	if (func != 0)		/* Only "get" is supported */
		return -1;

	set_node_addr();
	memcpy(p, node_addr, ETHER_ADDR_SIZE);
	return 0;
}

#ifdef TEST
unsigned char buf[1546];

/*
0:  <ETH dst>  <ETH src>  <ETH type>

14:  <IPHL,VERSION,TOS,TOTAL_LEN>
18:  <ID><FRAG>
22:  <TTL><PROTO><CHECK>
26:  <SADDR>
30:  <DADDR>
 */
static void send_back(unsigned char *buf, int len)
{
	unsigned char dst[6];
	unsigned int type;
	unsigned char dst_ip[4];

	dst[0] = buf[6];
	dst[1] = buf[7];
	dst[2] = buf[8];
	dst[3] = buf[9];
	dst[4] = buf[10];
	dst[5] = buf[11];

	type = (buf[12] << 8) | buf[13];

	dst_ip[0] = buf[26];
	dst_ip[1] = buf[27];
	dst_ip[2] = buf[28];
	dst_ip[3] = buf[29];

	buf[26] = buf[30];
	buf[27] = buf[31];
	buf[28] = buf[32];
	buf[29] = buf[33];

	buf[30] = dst_ip[0];
	buf[31] = dst_ip[1];
	buf[32] = dst_ip[2];
	buf[33] = dst_ip[3];

	eth_transmit(dst, type, len - 14, buf + 14);
}

void start(void)
{
	eth_reset();
	while (1) {
		int len = 1546;
		if (eth_receive(buf, &len)) {
			if (buf[0] != 0xff) {
				*(volatile unsigned short *)0xB0C00000 = 0xff00;
				send_back(buf, len);
				*(volatile unsigned short *)0xB0C00000 = 0;
			}
		}
	}
}
#endif
