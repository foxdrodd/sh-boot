/*
 * Address of Physical Access (Area 4)
 *
 */
#define PA_83902	0xB0000000	/* DP83902A                 */
#define PA_83902_IF	0xB0040000	/* DP83902A Remote I/O Port */
#define PA_83902_RST	0xB0080000	/* DP83902A Reset Port      */

/*
 * PORT:
 *
 * 0x0000 --- 0x000F : DP83902A Register
 * 0x1000:             DP83902A Remote I/O port
 * 0x2000:             DP83902A Reset port
 *
 */

#define D83902A_REMOTE_IO	0x1000
#define D83902A_RESET		0x2000

#define D8390_REMOTE_IO	D83902A_REMOTE_IO

#define START_PG	0	/* First page of TX buffer */
#define STOP_PG		128	/* Last page +1 of RX ring */

static inline volatile unsigned short *
port2adr(unsigned short port)
{
  if (port <= 0x0010)
    return (volatile unsigned short *) (PA_83902 + (port << 1));
  else if (port == 0x1000)
    return (volatile unsigned short *) PA_83902_IF;
  else if (port == 0x2000)
    return (volatile unsigned short *) PA_83902_RST;

  /* Just in case... */
  return 0;
}

static void
delay (void)
{
  volatile unsigned short *a=(volatile unsigned short *)0xa0000000;
  unsigned short w;

  w = *a;
  asm volatile ("" : : : "memory" );
  w = *a;
  asm volatile ("" : : : "memory" );
  w = *a;
}

static inline unsigned long
inb (unsigned short port)
{
  unsigned long v = (*port2adr(port) >> 8);
  delay ();
  return v;
}

static inline void
outb (unsigned long value, unsigned short port)
{
  *port2adr(port) = value << 8;
  delay ();
}

static inline unsigned long 
inw (unsigned short port)
{
  unsigned long v = *port2adr(port);
  delay ();
  return v;
}

static inline void
outw (unsigned long value, unsigned short port)
{
  *port2adr(port) = value;
  delay ();
}

static inline void
nic_reset (void)
{
  outw (0, D83902A_RESET);
  sleep128 (1);
  outw (0xffff, D83902A_RESET);
  sleep128 (1);
}
