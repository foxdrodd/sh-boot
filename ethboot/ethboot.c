/* $Id: ethboot.c,v 1.3 2001/07/02 04:20:20 sugioka Exp $
 *
 * Ethernet boot loader
 *
 *  lilo/ethboot.c
 *
 *  Copyright (C) 2000  Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.
 *
 */

#include "defs.h"
#include "etherboot.h"

static void cache_flush (void);
static void halt (void);
static void load_file (char *, char *, unsigned long);
static void setup_parms (void);

static unsigned char *load_ptr;

/* This routine shoule be here, at the beginning */
void
etherboot (void)
{
  char *p;
  extern long __ethboot_bss_start, __ethboot_bss_end;
#ifdef ROM
  char *q;
  extern long __ethboot_data_rom_start;
  extern long __ethboot_data_start, __ethboot_data_end;
#endif

  /* Zero BSS */
  for (p=(char *)&__ethboot_bss_start; p<(char *)&__ethboot_bss_end; p++)
    *p = 0;

#ifdef ROM
  /* Initialize DATA segment */
  q = (char *)&__ethboot_data_rom_start;
  for (p=(char *)&__ethboot_data_start; p<(char *)&__ethboot_data_end; p++)
    *p = *q++;
#endif

  /* Enable the cache */
  cache_flush ();

  printf ("\nBooting from network!\n");

  if (!eth_probe ())
    {
      printf ("No adapter found\n");
      exit (0);
    }

  printf ("Searching for server (BOOTP/DHCP)...\n");

  if (!bootp ())
    {
      printf ("No Server found for BOOTP/DHCP\n");
      exit (0);
    }

  printf ("IP Address: %I\n", arptable[ARP_CLIENT].ipaddr.s_addr);

  if (arptable[ARP_SERVER].ipaddr.s_addr == 0)
    {
      printf ("No Server found to download the file\n");
      sleep (2);
      exit (0);
    }
  printf ("Server: %I", arptable[ARP_SERVER].ipaddr.s_addr);

  if (BOOTP_DATA_ADDR->bootp_reply.bp_giaddr.s_addr)
    printf(", Relay: %I", BOOTP_DATA_ADDR->bootp_reply.bp_giaddr.s_addr);
  if (arptable[ARP_GATEWAY].ipaddr.s_addr)
    printf(", Gateway %I", arptable[ARP_GATEWAY].ipaddr.s_addr);
  putchar('\n');

  if (filename[0] == 0)
    {
      printf ("No FILENAME is available\n");
      sleep (2);
      exit (0);
    }
  printf ("Kernel to load: \"%s\"\n", filename);

  /* My (client's) name */
  if (hostname == 0)
    {
      printf ("No HOSTNAME is available\n");
      sleep (2);
      exit (0);
    }
  hostname[hostnamelen]= '\0';
  printf ("HOSTNAME: %s\n", hostname);

  if (rootpath == 0)
    {
      printf ("No ROOT PATH is available\n");
      sleep (2);
      exit (0);
    }
  rootpath[rootpathlen]= '\0';
  printf ("ROOT PATH: %s\n", rootpath);

  if (vendorext_isvalid && commandline)
    {
      commandline[commandlinelen]= '\0';
      printf ("COMMANDLINE: %s\n", commandline);
    }
  else
    commandline = 0;

  setup_parms ();

  rpc_init ();

  load_file ("Kernel", filename, MEMORY_ADDR_KERNEL);

  cleanup_net ();
  cache_flush ();
  asm volatile ("jmp @r0; nop"
		: /* no output */
		: "z" (MEMORY_ADDR_KERNEL));

  /* on failure */
  exit (0);
}

static inline char *string_set (char *dest, const char *str)
{
  int len = strlen (str);
  memcpy (dest, str, len);
  return dest + len;
}

static int
machine_type (void)
{
  register long __sc0 __asm__ ("r0") = 3; /* FEATURE QUERY */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0)
		: "memory");

  return (__sc0 >> 8);
}

static int
serial_type (void)
{
  register long __sc0 __asm__ ("r0") = 3; /* FEATURE QUERY */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0)
		: "memory");

  return (__sc0 & 0x07);
}

static int
memory_size (void)
{
  register long __sc0 __asm__ ("r0") = 4; /* MEMORY SIZE */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0)
		: "memory");

  return (__sc0);
}

static int
io_base (void)
{
  register long __sc0 __asm__ ("r0") = 5; /* IO BASE */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0)
		: "memory");

  return (__sc0);
}

static const char hexchars[] = "0123456789abcdef";
#define digits hexchars		/* 10base is same for 16base (up to 10) */
static inline char highhex (int x) {  return hexchars[(x >> 4) & 0xf];  }
static inline char lowhex (int x) {  return hexchars[x & 0xf];  }

static void
setup_parms (void)
{
  char *p;
  unsigned long mem_size;

  /* Set up commandline at MEMORY_ADDR_CONFIG */
  memset ((void *)MEMORY_ADDR_CONFIG, 0, 4*1024);
  p = (char *)MEMORY_ADDR_CONFIG + 256;

  /* Query to BIOS and build the command line string */
  /* Build string "mem=XXM" */
  mem_size = memory_size ();    
  mem_size >>= 20; /* In Mega-byte */
  p = string_set (p, "mem=");
  if (mem_size >= 100)
    {
      *p++ = digits[mem_size/100];
      mem_size = mem_size % 100;
    }
  if (mem_size >= 10)
    {
      *p++ = digits[mem_size/10];
      mem_size = mem_size % 10;
    }
  *p++ = digits[mem_size];
  *p++ = 'M';
  *p++ = ' ';

  switch (machine_type ())
    {
    case 0: /* Unknown board */
      {			/* Build string "sh_mv=unknown,0xXXXXXX,1" */
	unsigned int io = io_base ();
	int b31_24, b23_16, b15_08, b07_00;

	b31_24 = (io>>24)&0xff;
	b23_16 = (io>>16)&0xff;
	b15_08 = (io>>8)&0xff;
	b07_00 = (io>>0)&0xff;

	p = string_set (p, "sh_mv=unknown,0x");
	*p++ = highhex (b31_24); *p++ = lowhex (b31_24);
	*p++ = highhex (b23_16); *p++ = lowhex (b23_16);
	*p++ = highhex (b15_08); *p++ = lowhex (b15_08);
	*p++ = highhex (b07_00); *p++ = lowhex (b07_00);
	p = string_set (p, ",1 ");
	break;
      }

    case 1:
      p = string_set (p, "sh_mv=CqREEK ");
      break;

    case 3:
      p = string_set (p, "sh_mv=SolutionEngine ");
      break;
    }

  if (serial_type () == 0)
    p = string_set (p, "console=ttySC0,115200 ");
  else
    p = string_set (p, "console=ttySC1,115200 ");

  p = sprintf (p, "root=/dev/nfs rw nfsroot=%I:%s ip=%I::%I:%I:%s::off", 
	       arptable[ARP_SERVER].ipaddr.s_addr,
	       rootpath,
	       arptable[ARP_CLIENT].ipaddr.s_addr,
	       arptable[ARP_GATEWAY].ipaddr.s_addr,
	       netmask,
	       hostname);

  if (commandline)
    {
      *p++ = ' ';
      p = string_set (p, commandline);
    }
  *p++ = '\0';
}

static int
binaryload(unsigned char *data, int block, int len, int eof)
{
  static long sum;

  if (block == 1)
    sum = 0;

  if (len != 0)
    {
      int i;

      memcpy (load_ptr, data, len);
      for (i=0; i<len; i++)
	sum += data[i];
      load_ptr += len;
    }

  if (eof)
    printf ("SUM: %x\n", sum);
  else
    putchar ('.');

  return -1; /* there is more data */
}

static void
load_file (char *name, char *file_name, unsigned long addr)
{
  for (;;)
    {
      load_ptr = (unsigned char *)addr;

      printf ("Loading %s: %s ", name , file_name);
      if (nfs (file_name, binaryload))
	break;

      printf ("Unable to load %s: %s .\n", name, file_name);
      sleep (2);	/* lay off server for a while */
    }
  printf ("done\n");
}

static void
cache_flush (void)
{
  register long __sc0 __asm__ ("r0") = 6; /* CACHE_CONTROL */
  register long __sc4 __asm__ ("r4") = 0; /* ENABLE */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4)
		: "memory");
}

static void
halt (void)
{
  register long __sc0 __asm__ ("r0") = 11; /* SHUTDOWN */
  register long __sc4 __asm__ ("r4") = (long) 0;

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4)
		: "memory");
}

void
exit (int status)
{
  halt ();
}

struct nic nic;
static unsigned char packet[ETH_MAX_PACKET+2]; /* 2 for alignment */
static unsigned char node_addr[ETHER_ADDR_SIZE];

static int
eth_reset (int func)
{
  register long __sc0 __asm__ ("r0") = 7;        /* ETH_RESET */
  register long __sc4 __asm__ ("r4") = (long) func; /* START or STOP */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4)
		: "memory");
  return 0;
}

static int
eth_get_node_addr (unsigned char *node_addr)
{
  register long __sc0 __asm__ ("r0") = 10;        /* ETH_NODE_ADDR */
  register long __sc4 __asm__ ("r4") = (long) 0;  /* GET */
  register long __sc5 __asm__ ("r5") = (long) node_addr;

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4), "r" (__sc5)
		: "memory");
  return 0;
}

static void
reset_tick (void)
{
  register long __sc0 __asm__ ("r0") = 12; /* RTC */
  register long __sc4 __asm__ ("r4") = (long) 5; /* Reset TICK */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4)
		: "memory");
  return;
}

int
eth_probe (void)
{
  nic.packet = packet+2;	/* Alignment */
  nic.packetlen = 0;
  nic.node_addr = node_addr;

  reset_tick ();
  eth_get_node_addr (node_addr);
  memcpy (arptable[ARP_CLIENT].node, node_addr, ETHER_ADDR_SIZE);
  eth_reset (0);		/* START */
  return 1;
}

static int
eth_receive (void)
{
  register long __sc4 __asm__ ("r4") = (long) nic.packet;
  register long __sc5 __asm__ ("r5") = (long) &nic.packetlen;
  register long __sc0 __asm__ ("r0") = 8; /* ETH_RECEIVE */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4), "r" (__sc5)
		: "memory");

  return __sc0;
}

int
eth_poll (void)
{
  return eth_receive ();
}

void
eth_disable (void)
{
  eth_reset (1);		/* STOP */
}

unsigned long
currticks (void)
{
  register long __sc0 __asm__ ("r0") = 12; /* RTC */
  register long __sc4 __asm__ ("r4") = (long) 6; /* Get TICK */

  asm volatile ("trapa	#0x3F"
		: "=z" (__sc0)
		: "0" (__sc0), "r" (__sc4)
		: "memory");

  return __sc0;
}
