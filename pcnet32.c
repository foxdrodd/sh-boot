
/*
 * Based on pcnet32.c from linux kernel, vastly pared down to act
 * as a standalone driver for the AMD79C973 on the Hitachi SH7751
 * Solution Engine.
 */

#include "config.h"
#include "string.h"
#include "defs.h"
#include "io.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned long  u32;

typedef signed char  s8;
typedef signed short s16;
typedef signed long  s32;

static char hexbuf[10];
static const int pcnet32_debug = 0;
#define DBPRINT(str) if (pcnet32_debug) putString(str)

static int sync_tx;
static const int init_sync_tx = 0;

#define PORT_AUI      0x00
#define PORT_10BT     0x01
#define PORT_GPSI     0x02
#define PORT_MII      0x03

#define PORT_PORTSEL  0x03
#define PORT_ASEL     0x04
#define PORT_100      0x40
#define PORT_FD	      0x80

#define PCNET32_DMA_MASK 0xffffffff

/*
 * Set buffer counts w/Log_2(count), i.e. encoding used in the
 * init block.  Boot apps generally need only one Tx buffer, but
 * allow 4 (2^2) here (what the heck); receive needs more to deal
 * with unwanted packets (e.g. other broadcasts), use 16 (2^4).
 */
#define PCNET32_LOG_TX_BUFFERS 2  /* Use 4 tx buffers */
#define PCNET32_LOG_RX_BUFFERS 4  /* Use 16 rx buffers */

#define TX_RING_SIZE			(1 << (PCNET32_LOG_TX_BUFFERS))
#define TX_RING_MOD_MASK		(TX_RING_SIZE - 1)
#define TX_RING_LEN_BITS		((PCNET32_LOG_TX_BUFFERS) << 12)

#define RX_RING_SIZE			(1 << (PCNET32_LOG_RX_BUFFERS))
#define RX_RING_MOD_MASK		(RX_RING_SIZE - 1)
#define RX_RING_LEN_BITS		((PCNET32_LOG_RX_BUFFERS) << 4)

/*
 * Use PKT_RESERVE as placeholder in case alignment slop is needed.
 * (Allocated memory accounts for max packet size plus reserve, so
 * alignment can skip up to reserve # of bytes if needed and still
 * have max pkt size available for the chip to receive into.)
 */

#define PKT_BUF_SZ		1544
#define PKT_RESERVE		0
#define PKT_MEM_SIZE		(PKT_BUF_SZ + PKT_RESERVE)

/* Offsets from base I/O address. */
#define PCNET32_WIO_RDP		0x10
#define PCNET32_WIO_RAP		0x12
#define PCNET32_WIO_RESET	0x14
#define PCNET32_WIO_BDP		0x16

#define PCNET32_DWIO_RDP	0x10
#define PCNET32_DWIO_RAP	0x14
#define PCNET32_DWIO_RESET	0x18
#define PCNET32_DWIO_BDP	0x1C

#define PCI_VENDOR_ID_AMD	      0x1022
#define PCI_DEVICE_ID_AMD_LANCE	      0x2000

#define ETHER_ADDR_SIZE	    6
#define ETHER_HDR_SIZE     14
#define ETHER_MIN_PSIZE    64

/* Local defs for the address, like stnic.c */
static int node_addr_valid;
unsigned char node_addr[ETHER_ADDR_SIZE];
static const unsigned char local_dummy[ETHER_ADDR_SIZE]
 = CONFIG_ETHER_ADDRESS_ARRAY;

/* The PCNET32 Rx and Tx ring descriptors. */
struct pcnet32_rx_head {
  u32 base;
  s16 buf_length;
  s16 status;	   
  u32 msg_length;
  u32 reserved;
};
	
struct pcnet32_tx_head {
  u32 base;
  s16 length;
  s16 status;
  u32 misc;
  u32 reserved;
};

/* The PCNET32 32-Bit initialization block, described in databook. */
struct pcnet32_init_block {
  u16 mode;
  u16 tlen_rlen;
  u8  phys_addr[6];
  u16 reserved;
  u32 filter[2];
  /* Receive and transmit ring base, along with extra bits. */    
  u32 rx_ring;
  u32 tx_ring;
};

/* PCnet32 access functions */
struct pcnet32_access {
  u16  (*read_csr) (unsigned long, int);
  void (*write_csr)(unsigned long, int, u16);
  u16  (*read_bcr) (unsigned long, int);
  void (*write_bcr)(unsigned long, int, u16);
  u16  (*read_rap) (unsigned long);
  void (*write_rap)(unsigned long, u16);
  void (*reset)    (unsigned long);
};

/*
 * The first three fields of pcnet32_private are read by the ethernet device 
 * so we allocate the structure should be allocated by pci_alloc_consistent().
 */

/*
 * No such thing as pci_alloc_consistent() here, will just use any available
 * memory (default based on CONFIG definition, should probably allow caller
 * to pass an address before or during eth_reset).  Also, put everything in
 * the structure, including the Tx/Rx buffers themselves and the base address
 * for I/O access to the chip.  (Thus change the structure name.)  Put all
 * chip-accessible structures (like buffers) near the beginning, since the
 * init block and rings already were there.
 */
struct pcnet32_data {
  /* Ring entries must be 16-byte aligned in 32bit mode. */
  struct pcnet32_rx_head    rx_ring[RX_RING_SIZE];
  struct pcnet32_tx_head    tx_ring[TX_RING_SIZE];
  struct pcnet32_init_block init_block;

  /* Now the buffers */
  char rx_bufs[RX_RING_SIZE][PKT_MEM_SIZE];
  char tx_bufs[TX_RING_SIZE][PKT_MEM_SIZE];

  /* Remaining fields are SW (not chip) data */
  unsigned long ioaddr;
  const struct pcnet32_access *a;
  unsigned int cur_rx, cur_tx;		/* The next free ring entry */
};

/* Goes in BSS space, init at runtime */
struct pcnet32_data *pcdata;

/* Forward decls */
static int eth_startup(void);
static void pcnet32_init_ring(struct pcnet32_data *lp);

/* Return pointer to low-order (width) digits of num */
/* Pointer is to static array, copy before next call */
static char *itox(unsigned int num, int width)
{
  static const char hchars[] = "0123456789ABCDEF";
  static char hexbuf[9];
  char *dptr = &hexbuf[7];

  if (width > 8 || width < 1)
    width = 8;

  while (width--) {
    *--dptr = hchars[num & 0x0f];
    num = (num >> 4);
  }
  
  return dptr;
}

/*
 * Access functions to read/write CSR and BCR registers in the
 * chip; two sets needed since DWIO and WIO mode have different
 * IO-space offsets for the various access registers.
 */
static u16 pcnet32_wio_read_csr (unsigned long addr, int index)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RAP) = index;
  return *(volatile unsigned short*)(addr+PCNET32_WIO_RDP);
}
static void pcnet32_wio_write_csr (unsigned long addr, int index, u16 val)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RAP) = index;
  *(volatile unsigned short*)(addr+PCNET32_WIO_RDP) = val;
}
static u16 pcnet32_wio_read_bcr (unsigned long addr, int index)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RAP) = index;
  return *(volatile unsigned short*)(addr+PCNET32_WIO_BDP);
}
static void pcnet32_wio_write_bcr (unsigned long addr, int index, u16 val)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RAP) = index;
  *(volatile unsigned short*)(addr+PCNET32_WIO_BDP) = val;
}
static u16 pcnet32_wio_read_rap (unsigned long addr)
{
  return *(volatile unsigned short *)(addr+PCNET32_WIO_RAP);
}
static void pcnet32_wio_write_rap (unsigned long addr, u16 val)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RAP) = val;
}
static void pcnet32_wio_reset (unsigned long addr)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RESET);
}
static int pcnet32_wio_check (unsigned long addr)
{
  *(volatile unsigned short *)(addr+PCNET32_WIO_RAP) = 88;
  return (*(volatile unsigned short *)(addr+PCNET32_WIO_RAP) == 88);
}
static const struct pcnet32_access pcnet32_wio = {
  pcnet32_wio_read_csr,
  pcnet32_wio_write_csr,
  pcnet32_wio_read_bcr,
  pcnet32_wio_write_bcr,
  pcnet32_wio_read_rap,
  pcnet32_wio_write_rap,
  pcnet32_wio_reset
};

static u16 pcnet32_dwio_read_csr (unsigned long addr, int index)
{
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RAP) = index;
  return *(volatile unsigned long *)(addr+PCNET32_DWIO_RDP);
}
static void pcnet32_dwio_write_csr (unsigned long addr, int index, u16 val)
{
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RAP) = index;
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RDP) = val;
}
static u16 pcnet32_dwio_read_bcr (unsigned long addr, int index)
{
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RAP) = index;
  return *(volatile unsigned long *)(addr+PCNET32_DWIO_BDP);
}
static void pcnet32_dwio_write_bcr (unsigned long addr, int index, u16 val)
{
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RAP) = index;
  *(volatile unsigned long *)(addr+PCNET32_DWIO_BDP) = val;
}
static u16 pcnet32_dwio_read_rap (unsigned long addr)
{
  return *(volatile unsigned long *)(addr+PCNET32_DWIO_RAP);
}
static void pcnet32_dwio_write_rap (unsigned long addr, u16 val)
{
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RAP) = val;
}
static void pcnet32_dwio_reset (unsigned long addr)
{
  *(volatile unsigned long *)(addr+PCNET32_DWIO_RESET);
}
static int pcnet32_dwio_check (unsigned long addr)
{
  pcnet32_dwio_write_rap(addr, 88);
  return (pcnet32_dwio_read_rap(addr) == 88);
}
static const struct pcnet32_access pcnet32_dwio = {
  pcnet32_dwio_read_csr,
  pcnet32_dwio_write_csr,
  pcnet32_dwio_read_bcr,
  pcnet32_dwio_write_bcr,
  pcnet32_dwio_read_rap,
  pcnet32_dwio_write_rap,
  pcnet32_dwio_reset
};


/*
 * Name:      pcnet32_setup_pci
 * Function:  Set AMD PCI regs, reset chip.
 *            Clear main data structure.
 * Arguments: None.
 * Returns:   0 if ok, neg on error
 */
static int pcnet32_setup_pci(void)
{
  unsigned long rval;

  /* First check the ID value */
  rval = pci_read_config_dword(0x00);
  if (rval != ((PCI_DEVICE_ID_AMD_LANCE << 16) | PCI_VENDOR_ID_AMD)) {
    return -2;
  }

  /* Disable most PCI access -- turn off BMEN, MEMEN, IOEN */
  rval = pci_read_config_dword(0x04);
  pci_write_config_dword(0x04, (rval & ~0x0007));

  /* Clear data structure */
  if (!pcdata) return -3;
  memset(pcdata, 0, sizeof(*pcdata));

  /* Program IO address to next available */
  pcdata->ioaddr = pci_nextio;
  pci_nextio += 32;
  pci_write_config_dword(0x10, pcdata->ioaddr);

  /* Enable IOEN first to allow IO access */
  rval = pci_read_config_dword(0x04);
  pci_write_config_dword(0x04, (rval | 0x0001));

  /* Force a chip reset (using IO space) */
  pcnet32_dwio_reset(pcdata->ioaddr);
  pcnet32_wio_reset(pcdata->ioaddr);

  /* Now enable bus master (BMEN) */
  rval = pci_read_config_dword(0x04);
  pci_write_config_dword(0x04, (rval | 0x0004));

  return 0;
}

/*
 * Name:      eth_reset
 * Function:  Called thru BIOS for LAN control.
 * Arguments: 0 to reset/start, 1 to stop.
 * Returns:   0 if ok, else an error code.
 */
int eth_reset(unsigned int start_or_stop)
{
  int rc;
  unsigned long ioaddr;

  DBPRINT("IN RESET\n");

  /* Set up PCI if first time */
  if (!pcdata) {
    sync_tx = init_sync_tx;
    pcdata = (struct pcnet32_data *)CONFIG_PCNET_MEMORY;
    rc = pcnet32_setup_pci();
    if (rc != 0) {
      DBPRINT("In eth_reset("); DBPRINT(itox(start_or_stop, 8));
      DBPRINT("): pcnet32_setup_pci() returns"); DBPRINT(itox(rc, 4));
      DBPRINT("!\n");
      return rc;
    }
  }
  /* Special -1 means just init PCI */
  if (start_or_stop == -1)
    return 0;

  /* Must have ioaddr set up now */
  ioaddr = pcdata->ioaddr;

  /* Handle stop first -- no access fns means never started */
  if (start_or_stop == 1) {
    if (pcdata->a != 0)
      pcdata->a->write_csr(ioaddr, 0, 0x0004);
    return 0;
  }

  /* Handle start: call real init function */
  rc = eth_startup();
  return rc;
}  

/* Startup: meld pcnet32_probe1(), pcnet32_open() as needed. */
/* Return 0 if init completes, else error code */
int eth_startup(void)
{
  struct pcnet32_data *lp = pcdata;
  unsigned long ioaddr = lp->ioaddr;
  const struct pcnet32_access *a = 0;

  int i;
  int chip_version;

  u16 val;

  DBPRINT("ENTER: eth_startup()...\n");

  /* Force a chip reset */
  pcnet32_dwio_reset(ioaddr);
  pcnet32_wio_reset(ioaddr);

  /* Important to do the check for dwio mode first. */
  if (pcnet32_dwio_read_csr(ioaddr, 0) == 4 && pcnet32_dwio_check(ioaddr)) {
    lp->a = a = &pcnet32_dwio;
  } else if (pcnet32_wio_read_csr(ioaddr, 0) == 4 && pcnet32_wio_check(ioaddr)) {
    lp->a = a = &pcnet32_wio;
  } else {
    DBPRINT("No access functions!\n");
    return 1;
  }

  /* Read the chip version and confirm it */
  chip_version = a->read_csr(ioaddr, 88) | (a->read_csr(ioaddr,89) << 16);
  if (((chip_version & 0xfff) != 0x003) ||
      (((chip_version >> 12) & 0xffff) != 0x2625)) {
    DBPRINT("Chip version mismatch!!\n");
    return 2;
  }

  DBPRINT("  ...setting registers\n");

  /* switch pcnet32 to 32bit mode */
  a->write_bcr(ioaddr, 20, 2);

  /* Force autoselect bit (??) on */
  val = a->read_bcr (ioaddr, 2);
  a->write_bcr (ioaddr, 2, (val | 2));
    
  /* Don't force full-duplex */
  val = a->read_bcr (ioaddr, 9);
  a->write_bcr (ioaddr, 9, (val & ~0x0003));
    
  /* Autopad, mask counter overflows, mask txstart */
  /* (Sets 965 jabber mask, undefined for 973) */
  a->write_csr (ioaddr, 4, 0x0915);

  /* Disable Tx Stop On Underflow */
  val = a->read_csr (ioaddr, 3);
  a->write_csr (ioaddr, 3, (val | 0x0040));
  
  /* Force PCI bus bursting, and set NOUFLO bit */
  a->write_bcr(ioaddr, 18, ((a->read_bcr(ioaddr, 18)) | 0x860));

  /* Force tx start point to full packet (w/NOUFLO set...) */
  a->write_csr(ioaddr, 80, (a->read_csr(ioaddr, 80) & 0x0C00) | 0x0c00);

  /* Use full SRAM, split Tx/Rx evenly */
  a->write_bcr(ioaddr, 25, 0x16);
  a->write_bcr(ioaddr, 26, 0x08);
    
  DBPRINT("  ...filling in init block\n");

  /* Set up some init-block fields */
  lp->init_block.mode = le16_to_cpu(0x0000);
  lp->init_block.filter[0] = 0x00000000;
  lp->init_block.filter[1] = 0x00000000;

  /* Initialize the tx/rx rings */
  pcnet32_init_ring(lp);
    
  /* Generate address for later if needed */
  if (!node_addr_valid) {
    memcpy(node_addr, local_dummy, ETHER_ADDR_SIZE);
    node_addr[5] = *(unsigned short*)CONFIG_ETHER_CONFIG_WORD;
    node_addr_valid = 1;
  }

  /* Fill in the local address */
  for (i = 0; i < 6; i++)
    lp->init_block.phys_addr[i] = node_addr[i];

  /******************************************************/
  /* Re-initialize the PCNET32, and start it when done. */
  /******************************************************/

  DBPRINT("  ...issuing INIT command\n");

  /* Put init-block address in CSR1/CSR2 */
  a->write_csr (ioaddr, 1, ((u32)&lp->init_block & 0xffff));
  a->write_csr (ioaddr, 2, (((u32)&lp->init_block >> 16) & 0xffff));

  /* Turn on INIT, wait for IDON */
  a->write_csr (ioaddr, 0, 0x0001);
  sleep128(64); /* Wait half a sec */
  for (i = 0; i < 64; sleep128(1), i++)
    if (a->read_csr(ioaddr, 0) & 0x0100)
      break;

  if ((a->read_csr(ioaddr, 0) & 0x0100) == 0) {
    DBPRINT("Init timed out!!\n");
    return 4;
  }

  putString("Initialization done, ");

  /* Turn on STRT, but NOT IENA (polling only) */
  a->write_csr (ioaddr, 0, 0x0002);

  putString("LANCE Enabled.\n");

  return 0;
}

/* Initialize the PCNET32 Rx and Tx rings. */
/* Use 1-1 mapping rings to buffers in structure */
/* Don't bother with reserving space for alignment */
void pcnet32_init_ring(struct pcnet32_data *lp)
{
  int i;

  lp->cur_rx = lp->cur_tx = 0;

  for (i = 0; i < RX_RING_SIZE; i++) {
    lp->rx_ring[i].base = (u32)le32_to_cpu(&lp->rx_bufs[i][0] + PKT_RESERVE);
    lp->rx_ring[i].buf_length = le16_to_cpu(-PKT_BUF_SZ);
    lp->rx_ring[i].status = le16_to_cpu(0x8000);
  }

  /* Tx entries are normally "as needed", only status is cleared here */
  /* HOWEVER, since we know the mapping in advance might as well do it... */
  for (i = 0; i < TX_RING_SIZE; i++) {
    lp->tx_ring[i].base = (u32)le32_to_cpu(&lp->tx_bufs[i][0]);
    lp->tx_ring[i].status = 0;
  }

  /* Ok, fill in the relevant portions of the init_block */
  lp->init_block.tlen_rlen = le16_to_cpu(TX_RING_LEN_BITS | RX_RING_LEN_BITS);
  lp->init_block.rx_ring = (u32)le32_to_cpu(&lp->rx_ring[0]);
  lp->init_block.tx_ring = (u32)le32_to_cpu(&lp->tx_ring[0]);
}



/* Based on pcnet32_start_xmit, polling only */
/* Returns 0 on success, error code otherwise */
int eth_transmit(const char *dst, unsigned int type,
		 unsigned int size, const char *p)
{
  struct pcnet32_data *lp = pcdata;
  unsigned int ioaddr = lp->ioaddr;
  const struct pcnet32_access *a = lp->a;

  u16 status;
  int entry;
  char *buf;
  int dtime;

  /* Confirm packet length is ok */
  if ((size + ETHER_HDR_SIZE) > PKT_MEM_SIZE) {
    DBPRINT("Transmit packet exceeds maximum length!\n");
    return 1;
  }

  /* Locate current ring entry, check it */
  entry = lp->cur_tx & TX_RING_MOD_MASK;
  status = (u16)le16_to_cpu(lp->tx_ring[entry].status);
  if (status & 0x8000) {
    DBPRINT("Transmit ring is full!\n");
    return 2;
  }
  else if (status & 0x4000) {
    int err = le32_to_cpu(lp->tx_ring[entry].misc);
    DBPRINT("Found old tx error: status = 0x");
    DBPRINT(itox(status, 4)); DBPRINT(", err_status = 0x");
    DBPRINT(itox(err, 8)); DBPRINT("\n");
  }

  /* Check base address, fix if necessary */
  if (lp->tx_ring[entry].base != (u32)le32_to_cpu(&lp->tx_bufs[entry][0])) {
    DBPRINT("NEED TO FIX UP BASE ADDRESS???\n");
    lp->tx_ring[entry].base = le32_to_cpu(&lp->tx_bufs[entry][0]);
  }

  /* Prepare packet in transmit buffer */
  buf = &lp->tx_bufs[entry][0];
  memcpy(buf, dst, ETHER_ADDR_SIZE);
  buf += ETHER_ADDR_SIZE;
  memcpy(buf, node_addr, ETHER_ADDR_SIZE);
  buf += ETHER_ADDR_SIZE;
  *buf++ = (type >> 8); *buf++ = type;
  memcpy(buf, p, size);
  
  /* Fill in other descriptor fields (length, misc) */
  lp->tx_ring[entry].length = le16_to_cpu(-(size + ETHER_HDR_SIZE));
  lp->tx_ring[entry].misc = 0x00000000;

  /* Pass to chip (OWN, STP, ENP), move to next */
  lp->tx_ring[entry].status = le16_to_cpu(0x8300);
  lp->cur_tx++;

  /* Trigger an immediate tx poll. */
  a->write_csr (ioaddr, 0, 0x0008);

  /* If we're not synchronous, return now... */
  if (!sync_tx) {
    return 0;
  }

  /* ...otherwise, wait (half a sec) for completion */
  for (dtime = 0; dtime++ < 64; sleep128(1)) {
    status = le16_to_cpu(lp->tx_ring[entry].status);
    if ((status & 0x8000) == 0) break;
  }

  if (status & 0x8000) {
    DBPRINT("Excessive transmit time!!\n");
    return 3;
  } else if (status & 0x4000) {
    int err = le32_to_cpu(lp->tx_ring[entry].misc);
    DBPRINT("Got tx error: status = 0x");
    DBPRINT(itox(status, 4)); DBPRINT(", err_status = 0x");
    DBPRINT(itox(err, 8)); DBPRINT("\n");
    lp->tx_ring[entry].status = 0;
    return 4;
  }

  return 0;
}

/* Based on pcnet32_rx, poll for receipt */
/* Returns true/false (got packet or not) */
int eth_receive (char *p, unsigned int *len_p)
{
  short rxok = 0;
  struct pcnet32_data *lp = pcdata;
  int entry = lp->cur_rx & RX_RING_MOD_MASK;

  /* Loop to find good packet (rxok) or end of ring (chip owns entry) */
  while (!rxok && ((short)le16_to_cpu(lp->rx_ring[entry].status) >= 0)) {
    int status = (short)le16_to_cpu(lp->rx_ring[entry].status) >> 8;
    short pkt_len = (le32_to_cpu(lp->rx_ring[entry].msg_length) & 0xfff)-4;

    if (status == 3) {
      /* Good if non-runt */
      if (pkt_len >= 60) {
	memcpy(p, &lp->rx_bufs[entry][0], pkt_len);
	if (len_p != 0) *len_p = pkt_len;
	rxok = 1;
      } else {
	DBPRINT("Runt packet!\n");
      }
    } else {
      /* Error indication */
      DBPRINT("RXERR: status = 0x");
      DBPRINT(itox(status, 4)); DBPRINT("\n");
    }

    /* Recycle ring entry to chip, move up to next */

    /*
     * The docs say that the buffer length isn't touched, but Andrew Boyd
     * of QNX reports that some revs of the 79C965 clear it.
     */
    lp->rx_ring[entry].buf_length = le16_to_cpu(-PKT_BUF_SZ);
    lp->rx_ring[entry].status = le16_to_cpu(0x8000);
    entry = (++lp->cur_rx) & RX_RING_MOD_MASK;
  }

  return rxok;
}

/* Set or get MAC address: copied from stnic.c */
/* Arg 0 gets address, 1 sets it, others rejected */
int eth_node_addr(unsigned int func, char *p)
{
  if (func > 1)
    return -1; /* Unknown function */

  if (!node_addr_valid) {
    memcpy(node_addr, local_dummy, ETHER_ADDR_SIZE);
    node_addr[5] = *(unsigned short*)CONFIG_ETHER_CONFIG_WORD;
    node_addr_valid = 1;
  }

  if (func)
    memcpy(node_addr, p, ETHER_ADDR_SIZE);
  else
    memcpy(p, node_addr, ETHER_ADDR_SIZE);

  return 0;
}
