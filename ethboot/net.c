/* Taken from etherboot */
/**************************************************************************
ETHERBOOT -  BOOTP/TFTP Bootstrap Program

Author: Martin Renters
  Date: Dec/93

Literature dealing with the network protocols:
	ARP - RFC826
	RARP - RFC903
	UDP - RFC768
	BOOTP - RFC951, RFC2132 (vendor extensions)
	DHCP - RFC2131, RFC2132 (options)
	TFTP - RFC1350, RFC2347 (options), RFC2348 (blocksize), RFC2349 (tsize)
	RPC - RFC1831, RFC1832 (XDR), RFC1833 (rpcbind/portmapper)
	NFS - RFC1094, RFC1813 (v3, useful for clarifications, not implemented)

**************************************************************************/

#include "defs.h"
#include "etherboot.h"

struct arptable_t arptable[MAX_ARP];
struct bootpd_t bootp_data;

char filename[128];

int vendorext_isvalid;
unsigned long netmask;
char *hostname;
int hostnamelen;
char *commandline;
int commandlinelen;
char *rootpath;
int rootpathlen;

unsigned long xid;
unsigned char *end_of_rfc1533 = NULL;
int dhcp_reply;
in_addr dhcp_server = { 0L };
in_addr dhcp_addr = { 0L };

unsigned char vendorext_magic[] = "DODES";	/* Our magic string */
static const char broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/**************************************************************************
DEFAULT_NETMASK - Return default netmask for IP address
**************************************************************************/
static inline unsigned long default_netmask(void)
{
	int net = ntohl(arptable[ARP_CLIENT].ipaddr.s_addr) >> 24;
	if (net <= 127)
		return (htonl(0xff000000));
	else if (net < 192)
		return (htonl(0xffff0000));
	else
		return (htonl(0xffffff00));
}

/**************************************************************************
UDP_TRANSMIT - Send a UDP datagram
**************************************************************************/
int udp_transmit(unsigned long destip, unsigned int srcsock,
		 unsigned int destsock, int len, const void *buf)
{
	struct iphdr *ip;
	struct udphdr *udp;
	struct arprequest arpreq;
	int arpentry, i;
	int retry;

	ip = (struct iphdr *)buf;
	udp = (struct udphdr *)((long)buf + sizeof(struct iphdr));
	ip->verhdrlen = 0x45;
	ip->service = 0;
	ip->len = htons(len);
	ip->ident = 0;
	ip->frags = 0;
	ip->ttl = 60;
	ip->protocol = IP_UDP;
	ip->chksum = 0;
	ip->src.s_addr = arptable[ARP_CLIENT].ipaddr.s_addr;
	ip->dest.s_addr = destip;
	ip->chksum = ipchksum((unsigned short *)buf, sizeof(struct iphdr));
	udp->src = htons(srcsock);
	udp->dest = htons(destsock);
	udp->len = htons(len - sizeof(struct iphdr));
	udp->chksum = 0;
	if (destip == IP_BROADCAST) {
		eth_transmit(broadcast, IP, len, buf);
	} else {
		if (((destip & netmask) !=
		     (arptable[ARP_CLIENT].ipaddr.s_addr & netmask)) &&
		    arptable[ARP_GATEWAY].ipaddr.s_addr)
			destip = arptable[ARP_GATEWAY].ipaddr.s_addr;
		for (arpentry = 0; arpentry < MAX_ARP; arpentry++)
			if (arptable[arpentry].ipaddr.s_addr == destip)
				break;
		if (arpentry == MAX_ARP) {
			printf("%I is not in my arp table!\n", destip);
			return (0);
		}
		for (i = 0; i < ETHER_ADDR_SIZE; i++)
			if (arptable[arpentry].node[i])
				break;
		if (i == ETHER_ADDR_SIZE) {	/* Need to do arp request */
			arpreq.hwtype = htons(1);
			arpreq.protocol = htons(IP);
			arpreq.hwlen = ETHER_ADDR_SIZE;
			arpreq.protolen = 4;
			arpreq.opcode = htons(ARP_REQUEST);
			memcpy(arpreq.shwaddr, arptable[ARP_CLIENT].node,
			       ETHER_ADDR_SIZE);
			memcpy(arpreq.sipaddr, &arptable[ARP_CLIENT].ipaddr,
			       sizeof(in_addr));
			memset(arpreq.thwaddr, 0, ETHER_ADDR_SIZE);
			memcpy(arpreq.tipaddr, &destip, sizeof(in_addr));
			for (retry = 1; retry <= MAX_ARP_RETRIES; retry++) {
				eth_transmit(broadcast, ARP, sizeof(arpreq),
					     &arpreq);
				if (await_reply(AWAIT_ARP, arpentry,
						arpreq.tipaddr, TIMEOUT))
					goto xmit;
				rfc951_sleep(retry);
				/* We have slept for a while - the packet may
				 * have arrived by now.  If not, we have at
				 * least some room in the Rx buffer for the
				 * next reply.  */
				if (await_reply(AWAIT_ARP, arpentry,
						arpreq.tipaddr, 0))
					goto xmit;
			}
			return (0);
		}
	      xmit:
		eth_transmit(arptable[arpentry].node, IP, len, buf);
	}
	return (1);
}

/**************************************************************************
AWAIT_REPLY - Wait until we get a response for our request
**************************************************************************/
int await_reply(int type, int ival, void *ptr, int timeout)
{
	unsigned long time;
	struct iphdr *ip;
	struct udphdr *udp;
	struct arprequest *arpreply;
	struct bootp_t *bootpreply;
	struct rpc_t *rpc;
	unsigned short ptype;

	unsigned int protohdrlen = ETHER_HDR_SIZE + sizeof(struct iphdr) +
	    sizeof(struct udphdr);
	time = timeout + currticks();
	/* The timeout check is done below.  The timeout is only checked if
	 * there is no packet in the Rx queue.  This assumes that eth_poll()
	 * needs a negligible amount of time.  */
	for (;;) {
		if (eth_poll()) {	/* We have something! */
			/* Check for ARP - No IP hdr */
			if (nic.packetlen >= ETHER_HDR_SIZE) {
				ptype = ((unsigned short)nic.packet[12]) << 8
				    | ((unsigned short)nic.packet[13]);
			} else
				continue;	/* what else could we do with it? */
			if ((nic.packetlen >= ETHER_HDR_SIZE +
			     sizeof(struct arprequest)) && (ptype == ARP)) {
				unsigned long tmp;

				arpreply = (struct arprequest *)
				    &nic.packet[ETHER_HDR_SIZE];
				if ((arpreply->opcode == ntohs(ARP_REPLY)) &&
				    !memcmp(arpreply->sipaddr, ptr,
					    sizeof(in_addr))
				    && (type == AWAIT_ARP)) {
					memcpy(arptable[ival].node,
					       arpreply->shwaddr,
					       ETHER_ADDR_SIZE);
					return (1);
				}
				memcpy(&tmp, arpreply->tipaddr,
				       sizeof(in_addr));
				if ((arpreply->opcode == ntohs(ARP_REQUEST))
				    && (tmp ==
					arptable[ARP_CLIENT].ipaddr.s_addr)) {
					arpreply->opcode = htons(ARP_REPLY);
					memcpy(arpreply->tipaddr,
					       arpreply->sipaddr,
					       sizeof(in_addr));
					memcpy(arpreply->thwaddr,
					       arpreply->shwaddr,
					       ETHER_ADDR_SIZE);
					memcpy(arpreply->sipaddr,
					       &arptable[ARP_CLIENT].ipaddr,
					       sizeof(in_addr));
					memcpy(arpreply->shwaddr,
					       arptable[ARP_CLIENT].node,
					       ETHER_ADDR_SIZE);
					eth_transmit(arpreply->thwaddr, ARP,
						     sizeof(struct arprequest),
						     arpreply);
				}
				continue;
			}

			if (type == AWAIT_QDRAIN) {
				continue;
			}

			/* Anything else has IP header */
			if ((nic.packetlen < protohdrlen) || (ptype != IP))
				continue;
			ip = (struct iphdr *)&nic.packet[ETHER_HDR_SIZE];
			if ((ip->verhdrlen != 0x45) ||
			    ipchksum((unsigned short *)ip, sizeof(struct iphdr))
			    || (ip->protocol != IP_UDP))
				continue;
			udp = (struct udphdr *)&nic.packet[ETHER_HDR_SIZE +
							   sizeof(struct
								  iphdr)];

			/* BOOTP ? */
			bootpreply =
			    (struct bootp_t *)&nic.packet[ETHER_HDR_SIZE];
			if ((type == AWAIT_BOOTP)
			    && (nic.packetlen >=
				(ETHER_HDR_SIZE + sizeof(struct bootp_t)) -
				DHCP_OPT_LEN)
			    && (ntohs(udp->dest) == BOOTP_CLIENT)
			    && (bootpreply->bp_op == BOOTP_REPLY)
			    && (bootpreply->bp_xid == xid)) {
				arptable[ARP_CLIENT].ipaddr.s_addr =
				    bootpreply->bp_yiaddr.s_addr;
				dhcp_addr.s_addr = bootpreply->bp_yiaddr.s_addr;
				netmask = default_netmask();
				arptable[ARP_SERVER].ipaddr.s_addr =
				    bootpreply->bp_siaddr.s_addr;
				memset(arptable[ARP_SERVER].node, 0, ETHER_ADDR_SIZE);	/* Kill arp */
				arptable[ARP_GATEWAY].ipaddr.s_addr =
				    bootpreply->bp_giaddr.s_addr;
				memset(arptable[ARP_GATEWAY].node, 0, ETHER_ADDR_SIZE);	/* Kill arp */
				if (bootpreply->bp_file[0]) {
					memcpy(filename, bootpreply->bp_file,
					       128);
				}
				memcpy((char *)BOOTP_DATA_ADDR,
				       (char *)bootpreply,
				       sizeof(struct bootpd_t));
				decode_rfc1533(BOOTP_DATA_ADDR->bootp_reply.
					       bp_vend, 0, DHCP_OPT_LEN, 1);
				return (1);
			}

			/* RPC ? */
			rpc = (struct rpc_t *)&nic.packet[ETHER_HDR_SIZE];
			if ((type == AWAIT_RPC) &&
			    (ntohs(udp->dest) == ival) &&
			    (*(unsigned long *)ptr == ntohl(rpc->u.reply.id)) &&
			    (ntohl(rpc->u.reply.type) == MSG_REPLY)) {
				return (1);
			}
		} else {
			/* Do the timeout after at least a full queue walk.  */
			if ((timeout == 0) || (currticks() > time)) {
				break;
			}
		}
	}
	return (0);
}

/**************************************************************************
DECODE_RFC1533 - Decodes RFC1533 header
**************************************************************************/
int decode_rfc1533(p, block, len, eof)
register unsigned char *p;
int block, len, eof;
{
	static unsigned char *extdata = NULL, *extend = NULL;
	unsigned char *endp;

	if (block == 0) {
		end_of_rfc1533 = NULL;
		vendorext_isvalid = 0;
		if (memcmp(p, rfc1533_cookie, 4))
			return (0);	/* no RFC 1533 header found */
		p += 4;
		endp = p + len;
	} else {
		if (block == 1) {
			if (memcmp(p, rfc1533_cookie, 4))
				return (0);	/* no RFC 1533 header found */
			p += 4;
			len -= 4;
		}
		if (extend + len <=
		    (unsigned char *)&(BOOTP_DATA_ADDR->
				       bootp_extension[MAX_BOOTP_EXTLEN])) {
			memcpy(extend, p, len);
			extend += len;
		} else {
			printf("Overflow in vendor data buffer! Aborting...\n");
			*extdata = RFC1533_END;
			return (0);
		}
		p = extdata;
		endp = extend;
	}
	if (eof) {
		while (p < endp) {
			unsigned char c = *p;
			if (c == RFC1533_PAD) {
				p++;
				continue;
			} else if (c == RFC1533_END) {
				end_of_rfc1533 = endp = p;
				continue;
			} else if (c == RFC1533_NETMASK) {
				memcpy(&netmask, p + 2, sizeof(in_addr));
			}

			else if (c == RFC1533_GATEWAY) {
				/* This is a little simplistic, but it will
				   usually be sufficient.
				   Take only the first entry */
				if (TAG_LEN(p) >= sizeof(in_addr))
					memcpy(&arptable[ARP_GATEWAY].ipaddr,
					       p + 2, sizeof(in_addr));
			} else if (c == RFC2132_MSG_TYPE) {
				dhcp_reply = *(p + 2);
			} else if (c == RFC2132_SRV_ID) {
				memcpy(&dhcp_server, p + 2, sizeof(in_addr));
			} else if (c == RFC1533_HOSTNAME) {
				hostname = p + 2;
				hostnamelen = *(p + 1);
			} else if (c == RFC1533_ROOTPATH) {
				rootpath = p + 2;
				rootpathlen = *(p + 1);
			} else if (c == RFC1533_VENDOR_EXT
				   && TAG_LEN(p) >= 5
				   && memcmp(p + 2, vendorext_magic, 5) == 0)
				vendorext_isvalid++;
			else if (c == RFC1533_VENDOR_EXT + 1) {
				commandline = p + 2;
				commandlinelen = *(p + 1);
			}
			p += TAG_LEN(p) + 2;
		}
		extdata = extend = endp;
	}
	return (-1);		/* proceed with next block */
}

/**************************************************************************
IPCHKSUM - Checksum IP Header
**************************************************************************/
unsigned short ipchksum(ip, len)
register unsigned short *ip;
register int len;
{
	unsigned long sum = 0;
	len >>= 1;
	while (len--) {
		sum += *(ip++);
		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}
	return ((~sum) & 0x0000FFFF);
}

/**************************************************************************
RFC951_SLEEP - sleep for expotentially longer times
**************************************************************************/
void rfc951_sleep(exp)
int exp;
{
	static long seed = 0;
	long q;
	unsigned long tmo;

#ifdef BACKOFF_LIMIT
	if (exp > BACKOFF_LIMIT)
		exp = BACKOFF_LIMIT;
#endif
	if (!seed)		/* Initialize linear congruential generator */
		seed = currticks() + *(long *)&arptable[ARP_CLIENT].node
		    + ((short *)arptable[ARP_CLIENT].node)[2];
	/* simplified version of the LCG given in Bruce Scheier's
	   "Applied Cryptography" */
	q = seed / 53668;
	if ((seed = 40014 * (seed - 53668 * q) - 12211 * q) < 0)
		seed += 2147483563l;
	/* compute mask */
	for (tmo = 63; tmo <= 60 * TICKS_PER_SEC && --exp > 0;
	     tmo = 2 * tmo + 1) ;
	/* sleep */
	printf("<sleep>\n");
	for (tmo = (tmo & seed) + currticks(); currticks() < tmo;) ;
	return;
}

/**************************************************************************
CLEANUP_NET - shut down networking
**************************************************************************/
void cleanup_net(void)
{
	nfs_umountall(ARP_SERVER);
	eth_disable();
}

/**************************************************************************
CLEANUP - shut down etherboot so that the OS may be called right away
**************************************************************************/
void cleanup(void)
{
}

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */
