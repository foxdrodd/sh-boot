/* Taken from etherboot */
/**************************************************************************
ETHERBOOT -  BOOTP/TFTP Bootstrap Program

Author: Martin Renters
  Date: Dec/93

**************************************************************************/

#include "osdep.h"

#ifndef	MAX_BOOTP_RETRIES
#define MAX_BOOTP_RETRIES	20
#endif

#define MAX_BOOTP_EXTLEN	1024

#ifndef	MAX_ARP_RETRIES
#define MAX_ARP_RETRIES		20
#endif

#ifndef	MAX_RPC_RETRIES
#define MAX_RPC_RETRIES		20
#endif

#define	TICKS_PER_SEC		128

/* Inter-packet retry in ticks */
#define TIMEOUT			(10*TICKS_PER_SEC)

#ifndef	NULL
#define NULL	((void *)0)
#endif

#define TRUE		1
#define FALSE		0

#define ETHER_ADDR_SIZE		6	/* Size of Ethernet address */
#define ETHER_HDR_SIZE		14	/* Size of ethernet header */
#define ETH_MIN_PACKET		64
#define ETH_MAX_PACKET		1518

#define ARP_CLIENT	0
#define ARP_SERVER	1
#define ARP_GATEWAY	2
#define ARP_ROOTSERVER	3
#define ARP_SWAPSERVER	4
#define MAX_ARP		ARP_SWAPSERVER+1

#define IP		0x0800
#define ARP		0x0806

#define BOOTP_SERVER	67
#define BOOTP_CLIENT	68
#define SUNRPC_PORT	111

#define IP_UDP		17
/* Same after going through htonl */
#define IP_BROADCAST	0xFFFFFFFF

#define ARP_REQUEST	1
#define ARP_REPLY	2

#define BOOTP_REQUEST	1
#define BOOTP_REPLY	2

#define TAG_LEN(p)		(*((p)+1))
#define RFC1533_COOKIE		99, 130, 83, 99
#define RFC1533_PAD		0
#define RFC1533_NETMASK		1
#define RFC1533_TIMEOFFSET	2
#define RFC1533_GATEWAY		3
#define RFC1533_TIMESERVER	4
#define RFC1533_IEN116NS	5
#define RFC1533_DNS		6
#define RFC1533_LOGSERVER	7
#define RFC1533_COOKIESERVER	8
#define RFC1533_LPRSERVER	9
#define RFC1533_IMPRESSSERVER	10
#define RFC1533_RESOURCESERVER	11
#define RFC1533_HOSTNAME	12
#define RFC1533_BOOTFILESIZE	13
#define RFC1533_MERITDUMPFILE	14
#define RFC1533_DOMAINNAME	15
#define RFC1533_SWAPSERVER	16
#define RFC1533_ROOTPATH	17
#define RFC1533_EXTENSIONPATH	18
#define RFC1533_IPFORWARDING	19
#define RFC1533_IPSOURCEROUTING	20
#define RFC1533_IPPOLICYFILTER	21
#define RFC1533_IPMAXREASSEMBLY	22
#define RFC1533_IPTTL		23
#define RFC1533_IPMTU		24
#define RFC1533_IPMTUPLATEAU	25
#define RFC1533_INTMTU		26
#define RFC1533_INTLOCALSUBNETS	27
#define RFC1533_INTBROADCAST	28
#define RFC1533_INTICMPDISCOVER	29
#define RFC1533_INTICMPRESPOND	30
#define RFC1533_INTROUTEDISCOVER 31
#define RFC1533_INTROUTESOLICIT	32
#define RFC1533_INTSTATICROUTES	33
#define RFC1533_LLTRAILERENCAP	34
#define RFC1533_LLARPCACHETMO	35
#define RFC1533_LLETHERNETENCAP	36
#define RFC1533_TCPTTL		37
#define RFC1533_TCPKEEPALIVETMO	38
#define RFC1533_TCPKEEPALIVEGB	39
#define RFC1533_NISDOMAIN	40
#define RFC1533_NISSERVER	41
#define RFC1533_NTPSERVER	42
#define RFC1533_VENDOR		43
#define RFC1533_NBNS		44
#define RFC1533_NBDD		45
#define RFC1533_NBNT		46
#define RFC1533_NBSCOPE		47
#define RFC1533_XFS		48
#define RFC1533_XDM		49
#define RFC1533_VENDOR_EXT	128

#define RFC2132_REQ_ADDR	50
#define RFC2132_MSG_TYPE	53
#define RFC2132_SRV_ID		54
#define RFC2132_PARAM_LIST	55
#define RFC2132_MAX_SIZE	57

#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPACK			5

#define RFC1533_END		255
#define BOOTP_VENDOR_LEN	64
#define DHCP_OPT_LEN		312

#define AWAIT_ARP	0
#define AWAIT_BOOTP	1
#define AWAIT_RPC	4
#define AWAIT_QDRAIN	5	/* drain queue, process ARP requests */

/*
 *	Structure returned from eth_probe and passed to other driver
 *	functions.
 */

struct nic {
	char *packet;
	unsigned int packetlen;
	unsigned char *node_addr;
};

typedef struct {
	unsigned long s_addr;
} in_addr;

struct arptable_t {
	in_addr ipaddr;
	unsigned char node[6];
};

/*
 * A pity sipaddr and tipaddr are not longword aligned or we could use
 * in_addr. No, I don't want to use #pragma packed.
 */
struct arprequest {
	unsigned short hwtype;
	unsigned short protocol;
	char hwlen;
	char protolen;
	unsigned short opcode;
	char shwaddr[6];
	char sipaddr[4];
	char thwaddr[6];
	char tipaddr[4];
};

struct iphdr {
	char verhdrlen;
	char service;
	unsigned short len;
	unsigned short ident;
	unsigned short frags;
	char ttl;
	char protocol;
	unsigned short chksum;
	in_addr src;
	in_addr dest;
};

struct udphdr {
	unsigned short src;
	unsigned short dest;
	unsigned short len;
	unsigned short chksum;
};

struct bootp_t {
	struct iphdr ip;
	struct udphdr udp;
	char bp_op;
	char bp_htype;
	char bp_hlen;
	char bp_hops;
	unsigned long bp_xid;
	unsigned short bp_secs;
	unsigned short unused;
	in_addr bp_ciaddr;
	in_addr bp_yiaddr;
	in_addr bp_siaddr;
	in_addr bp_giaddr;
	char bp_hwaddr[16];
	char bp_sname[64];
	char bp_file[128];
	char bp_vend[BOOTP_VENDOR_LEN];
};

struct bootpd_t {
	struct bootp_t bootp_reply;
	unsigned char bootp_extension[MAX_BOOTP_EXTLEN];
};

struct rpc_t {
	struct iphdr ip;
	struct udphdr udp;
	union {
		char data[300];	/* longest RPC call must fit!!!! */
		struct {
			long id;
			long type;
			long rpcvers;
			long prog;
			long vers;
			long proc;
			long data[1];
		} call;
		struct {
			long id;
			long type;
			long rstatus;
			long verifier;
			long v2;
			long astatus;
			long data[1];
		} reply;
	} u;
};

#define PROG_PORTMAP	100000
#define PROG_NFS	100003
#define PROG_MOUNT	100005

#define MSG_CALL	0
#define MSG_REPLY	1

#define PORTMAP_GETPORT	3

#define MOUNT_ADDENTRY	1
#define MOUNT_UMOUNTALL	4

#define NFS_LOOKUP	4
#define NFS_READ	6

#define NFS_FHSIZE	32

#define NFSERR_PERM	1
#define NFSERR_NOENT	2
#define NFSERR_ACCES	13

/* Block size used for NFS read accesses.  A RPC reply packet (including  all
 * headers) must fit within a single Ethernet frame to avoid fragmentation.
 * Chosen to be a power of two, as most NFS servers are optimized for this.  */
#define NFS_READ_SIZE	1024

/***************************************************************************
External prototypes
***************************************************************************/
extern void rpc_init(void);
extern int nfs(const char *name, int (*)(unsigned char *, int, int, int));
extern void nfs_umountall(int);
extern int bootp(void);
extern int udp_transmit(unsigned long destip, unsigned int srcsock,
			unsigned int destsock, int len, const void *buf);
extern int await_reply(int type, int ival, void *ptr, int timeout);
extern int decode_rfc1533(unsigned char *, int, int, int);
extern unsigned short ipchksum(unsigned short *, int len);
extern void rfc951_sleep(int);
extern void cleanup_net(void);
extern void cleanup(void);

extern int eth_probe(void);
extern int eth_poll(void);
extern void eth_transmit(const char *, unsigned int, unsigned int,
			 const void *p);
extern void eth_disable(void);

extern unsigned long currticks(void);
extern void exit(int status);

/***************************************************************************
External variables
***************************************************************************/
extern char filename[128];
extern char *hostname;
extern int hostnamelen;
extern char *rootpath;
extern int rootpathlen;
extern char *commandline;
extern int commandlinelen;
extern int vendorext_isvalid;
extern unsigned long netmask;
extern struct arptable_t arptable[MAX_ARP];

#define	BOOTP_DATA_ADDR	(&bootp_data)
extern struct bootpd_t bootp_data;
extern unsigned char *end_of_rfc1533;

extern struct nic nic;

extern unsigned long xid;
extern char rfc1533_cookie[];
extern char rfc1533_end[];
extern int dhcp_reply;
extern in_addr dhcp_server;
extern in_addr dhcp_addr;

/*
 * Local variables:
 *  c-basic-offset: 8
 * End:
 */
