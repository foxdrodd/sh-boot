#include "defs.h"
#include "etherboot.h"

char    rfc1533_cookie[] = { RFC1533_COOKIE};
char    rfc1533_end[]={RFC1533_END };
static const char dhcpdiscover[]={
		RFC2132_MSG_TYPE,1,DHCPDISCOVER,
		RFC2132_MAX_SIZE,2,	/* request as much as we can */
		sizeof(struct bootpd_t) / 256, sizeof(struct bootpd_t) % 256,
		RFC2132_PARAM_LIST,4,
		RFC1533_NETMASK, RFC1533_GATEWAY,
		RFC1533_HOSTNAME, RFC1533_ROOTPATH
	};
static const char dhcprequest []={
		RFC2132_MSG_TYPE,1,DHCPREQUEST,
		RFC2132_SRV_ID,4,0,0,0,0,
		RFC2132_REQ_ADDR,4,0,0,0,0,
		RFC2132_MAX_SIZE,2,	/* request as much as we can */
		sizeof(struct bootpd_t) / 256, sizeof(struct bootpd_t) % 256,
		/* request parameters */
		RFC2132_PARAM_LIST, 6,
		/* Standard parameters */
		RFC1533_NETMASK, RFC1533_GATEWAY,
		RFC1533_HOSTNAME, RFC1533_ROOTPATH,
		/* Use VENDOR extentions */
		RFC1533_VENDOR_EXT,
		RFC1533_VENDOR_EXT+1,
	};

/**************************************************************************
BOOTP - Get my IP address and load information
**************************************************************************/
int bootp()
{
	int retry;
	int retry1;
	struct bootp_t bp;
	unsigned long  starttime;

	memset(&bp, 0, sizeof(struct bootp_t));
	bp.bp_op = BOOTP_REQUEST;
	bp.bp_htype = 1;
	bp.bp_hlen = ETHER_ADDR_SIZE;
	bp.bp_xid = xid = starttime = currticks();
	memcpy(bp.bp_hwaddr, arptable[ARP_CLIENT].node, ETHER_ADDR_SIZE);
	memcpy(bp.bp_vend, rfc1533_cookie, sizeof rfc1533_cookie); /* request RFC-style options */
	memcpy(bp.bp_vend+sizeof rfc1533_cookie, dhcpdiscover, sizeof dhcpdiscover);
	memcpy(bp.bp_vend+sizeof rfc1533_cookie +sizeof dhcpdiscover, rfc1533_end, sizeof rfc1533_end);

	for (retry = 0; retry < MAX_BOOTP_RETRIES; ) {
		/* Clear out the Rx queue first.  It contains nothing of
		 * interest, except possibly ARP requests from the DHCP/TFTP
		 * server.  We use polling throughout Etherboot, so some time
		 * may have passed since we last polled the receive queue,
		 * which may now be filled with broadcast packets.  This will
		 * cause the reply to the packets we are about to send to be
		 * lost immediately.  Not very clever.  */
		await_reply(AWAIT_QDRAIN, 0, NULL, 0);

		udp_transmit(IP_BROADCAST, BOOTP_CLIENT, BOOTP_SERVER,
			sizeof(struct bootp_t), &bp);
		if (await_reply(AWAIT_BOOTP, 0, NULL, TIMEOUT/10)){
			if (dhcp_reply==DHCPOFFER){
		dhcp_reply=0;
		memcpy(bp.bp_vend, rfc1533_cookie, sizeof rfc1533_cookie);
		memcpy(bp.bp_vend+sizeof rfc1533_cookie, dhcprequest, sizeof dhcprequest);
		memcpy(bp.bp_vend+sizeof rfc1533_cookie +sizeof dhcprequest, rfc1533_end, sizeof rfc1533_end);
		memcpy(bp.bp_vend+9, &dhcp_server, sizeof(in_addr));
		memcpy(bp.bp_vend+15, &dhcp_addr, sizeof(in_addr));
			for (retry1 = 0; retry1 < MAX_BOOTP_RETRIES;) {
			udp_transmit(IP_BROADCAST, BOOTP_CLIENT, BOOTP_SERVER,
				sizeof(struct bootp_t), &bp);
				dhcp_reply=0;
				if (await_reply(AWAIT_BOOTP, 0, NULL, TIMEOUT/10))
					if (dhcp_reply==DHCPACK)
						return(1);
					rfc951_sleep(++retry1);
				}
			} else
				return(1);
		}
		rfc951_sleep(++retry);
		bp.bp_secs = htons((currticks()-starttime)/TICKS_PER_SEC);
	}
	return(0);
}
