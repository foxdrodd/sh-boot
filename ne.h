#define D8390_REMOTE_IO	0x0010
#define NE_RESET	0x001f
#define START_PG	0x40	/* First page of TX buffer */
#define STOP_PG		0x80	/* Last page +1 of RX ring */

#define ISA_OFFSET	CONFIG_IO_BASE

static inline volatile unsigned char *
port2adr(unsigned short port)
{
  /* 0x0300 is only valid for my card */
  return (volatile unsigned char *)(ISA_OFFSET+0x0300+port);
}

static void
delay (void)
{
  volatile unsigned short trash;
  trash = *(volatile unsigned short *) 0xa0000000;
}

static inline unsigned long
inb (unsigned short port)
{
  unsigned long v = *port2adr(port);
  delay ();
  return v;
}

static inline void
outb (unsigned long value, unsigned short port)
{
  *port2adr(port) = value;
  delay ();
}

static inline unsigned long 
inw (unsigned short port)
{
  unsigned long v = *(unsigned short *)port2adr(port);
  delay ();
  return v;
}

static inline void
outw (unsigned long value, unsigned short port)
{
  *(unsigned short *)port2adr(port) = value;
  delay ();
}

static void
nic_reset (void)
{
  outb (inb (NE_RESET), NE_RESET);
}
