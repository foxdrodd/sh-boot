/* $Id: ide.c,v 1.14 2001/06/06 12:29:07 sugioka Exp $
 *
 * sh-ipl+g/ide.c
 *
 * Support for IDE disk drive
 *
 *  Copyright (C) 2000  Niibe Yutaka
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file "COPYING.LIB" in the main
 * directory of this archive for more details.
 *
 */
#include "config.h"
#include "defs.h"

/* Machine dependent part */

#include "io.h"

#if defined(CONFIG_SOLUTION_ENGINE)
#if defined(CONFIG_MRSHPC)
#define PA_MRSHPC	0xb83fffe0	/* MR-SHPC-01 PCMCIA controler */
#define PA_MRSHPC_MW1	0xb8400000	/* MR-SHPC-01 memory window base */
#define PA_MRSHPC_MW2	0xb8500000	/* MR-SHPC-01 attribute window base */
#define PA_MRSHPC_IO	0xb8600000	/* MR-SHPC-01 I/O window base */
#define MRSHPC_OPTION   (PA_MRSHPC + 6)
#define MRSHPC_CSR      (PA_MRSHPC + 8)
#define MRSHPC_ISR      (PA_MRSHPC + 10)
#define MRSHPC_ICR      (PA_MRSHPC + 12)
#define MRSHPC_CPWCR    (PA_MRSHPC + 14)
#define MRSHPC_MW0CR1   (PA_MRSHPC + 16)
#define MRSHPC_MW1CR1   (PA_MRSHPC + 18)
#define MRSHPC_IOWCR1   (PA_MRSHPC + 20)
#define MRSHPC_MW0CR2   (PA_MRSHPC + 22)
#define MRSHPC_MW1CR2   (PA_MRSHPC + 24)
#define MRSHPC_IOWCR2   (PA_MRSHPC + 26)
#define MRSHPC_CDCR     (PA_MRSHPC + 28)
#define MRSHPC_PCIC_INFO (PA_MRSHPC + 30)

#else /* !CONFIG_MRSHPC */

#define PA_SUPERIO	0xb0400000 /* SMC37C935A super io chip */
#define BCR_ILCRA	0xb1400000 /* Interrupt Controler */

/* Configuration port and key */
#define CONFIG_PORT		0x03f0
#define INDEX_PORT		CONFIG_PORT
#define DATA_PORT		0x03f1
#define CONFIG_ENTER		0x55
#define CONFIG_EXIT		0xaa

/* Configuration index */
#define CURRENT_LDN_INDEX	0x07
#define ACTIVATE_INDEX		0x30
#define IRQ_SELECT_INDEX	0x70
#define GPIO46_INDEX		0xc6
#define GPIO47_INDEX		0xc7

/* Logical device number */
#define LDN_IDE1		1
#define LDN_AUXIO		8

static inline void outb_p(unsigned long value, unsigned short port)
{
  unsigned long addr = PA_SUPERIO + (port << 1);

  *(volatile unsigned short *)addr = value << 8;
  delay();
}

static inline void smsc_config(int index, int data)
{
  outb_p(index, INDEX_PORT);
  outb_p(data, DATA_PORT);
}
#endif
#endif /* CONFIG_SOLUTION_ENGINE */

void delay (void);
static void delay256 (void);
static void delay10000 (void);

static unsigned long ide_offset;

/* Initialize hardware for IDE support */
void
init_ide (void)
{
#if defined(CONFIG_DIRECT_COMPACT_FLASH)
  ide_offset = CONFIG_IO_BASE;
#elif defined(CONFIG_SOLUTION_ENGINE)
#if defined(CONFIG_MRSHPC)
  ide_offset = PA_MRSHPC_IO;
  if ((p4_inw (MRSHPC_CSR) & 0x000c) == 0)
    { /* if card detect is true */
      if ((p4_inw (MRSHPC_CSR) & 0x0080) == 0)
	p4_outw (MRSHPC_CPWCR, 0x0674); /* Card Vcc is 3.3v? */
      else
	p4_outw (MRSHPC_CPWCR, 0x0678); /* Card Vcc is 5V */
      delay10000 ();		/* wait for power on */
    }
  else
    return;
  /*
    PC-Card window open 
    flag == COMMON/ATTRIBUTE/IO
   */
  /* common window open */
  p4_outw (MRSHPC_MW0CR1, 0x8a84);/* window 0xb8400000 */
  if ((p4_inw (MRSHPC_CSR) & 0x4000) != 0)
    p4_outw (MRSHPC_MW0CR2, 0x0b00); /* common mode & bus width 16bit SWAP = 1*/
  else
    p4_outw (MRSHPC_MW0CR2, 0x0300); /* common mode & bus width 16bit SWAP = 0*/
  /* attribute window open */
  p4_outw (MRSHPC_MW1CR1, 0x8a85);/* window 0xb8500000 */
  if ((p4_inw (MRSHPC_CSR) & 0x4000) != 0)
    p4_outw (MRSHPC_MW1CR2, 0x0a00); /* attribute mode & bus width 16bit SWAP = 1*/
  else
    p4_outw (MRSHPC_MW1CR2, 0x0200); /* attribute mode & bus width 16bit SWAP = 0*/
  /* I/O window open */
  p4_outw (MRSHPC_IOWCR1, 0x8a86);/* I/O window 0xb8600000 */
  p4_outw (MRSHPC_CDCR, 0x0008);	/* I/O card mode */
  if ((p4_inw(MRSHPC_CSR) & 0x4000) != 0)
    p4_outw (MRSHPC_IOWCR2, 0x0a00); /* bus width 16bit SWAP = 1*/
  else
    p4_outw (MRSHPC_IOWCR2, 0x0200); /* bus width 16bit SWAP = 0*/
  p4_outw (MRSHPC_ISR, 0x0000);
  p4_outw (MRSHPC_ICR, 0x2000);
  p4_outb ((PA_MRSHPC_MW2 + 0x206), 0x00);
  p4_outb ((PA_MRSHPC_MW2 + 0x200), 0x42);
#else
  ide_offset = PA_SUPERIO;

  outb_p(CONFIG_ENTER, CONFIG_PORT);
  outb_p(CONFIG_ENTER, CONFIG_PORT);

  /* Configure IDE1 */
  smsc_config(CURRENT_LDN_INDEX, LDN_IDE1);
  smsc_config(ACTIVATE_INDEX, 0x01);
  smsc_config(IRQ_SELECT_INDEX, 14); /* IRQ14 */

  /* Configure AUXIO (GPIO): to use IDE1 */
  smsc_config(CURRENT_LDN_INDEX, LDN_AUXIO);
  smsc_config(ACTIVATE_INDEX, 0x01);
  smsc_config(GPIO46_INDEX, 0x00); /* nIOROP */
  smsc_config(GPIO47_INDEX, 0x00); /* nIOWOP */

  outb_p(CONFIG_EXIT, CONFIG_PORT);
#endif
#elif defined(CONFIG_CQ_BRIDGE)
#define BRIDGE_IDE_OFFSET	0xA4000000

#define BRIDGE_ENABLE		0x0000
#define BRIDGE_FEATURE		0x0002

#define BRIDGE_IDE_CTRL		0x0018
#define BRIDGE_IDE_INTR_LVL    	0x001A
#define BRIDGE_IDE_INTR_MASK	0x001C
#define BRIDGE_IDE_INTR_STAT	0x001E

  ide_offset = BRIDGE_IDE_OFFSET;

  if ((ide_inw (BRIDGE_FEATURE) & 1))
    {				/* We have IDE interface */
      int i;

      ide_outw (0, BRIDGE_IDE_INTR_LVL);
      ide_outw (0, BRIDGE_IDE_INTR_MASK);

      for (i=0; i<10000; i++)
	ide_outw (0, BRIDGE_IDE_CTRL);
      for (i=0; i<10000; i++)
	ide_outw (0x8000, BRIDGE_IDE_CTRL);

      ide_outw (0x0f-14, BRIDGE_IDE_INTR_LVL); /* Use 14 IPR */
      ide_outw (1, BRIDGE_IDE_INTR_MASK); /* Enable interrupt */
      ide_outw (0xffff, BRIDGE_IDE_INTR_STAT); /* Clear interrupt status */
    }
#endif
}

void delay (void)
{
  volatile unsigned short trash;
  trash = *(volatile unsigned short *) 0xa0000000;
}

static void delay256 (void)
{
  int i;
  for (i=0; i<256; i++)
    delay ();
}

static void delay10000 (void)
{
  int i;
  for (i=0; i<10000; i++)
    delay ();
}

/* Machine independent part */

#define IDE_BSY 0x80
#define IDE_RDY 0x40
#define IDE_DSC 0x10
#define IDE_DRQ 0x08
#define IDE_ERR 0x01

#define IDE_DATA			0x01f0 /* 16-bit */

#define IDE_ERROR			0x01f1
#define IDE_FEATURES			0x01f1 /* Write */

#define IDE_SECTOR_COUNT		0x01f2
#define IDE_SECTOR_NUMBER		0x01f3
#define IDE_SECTOR_CYLINDER_LOW		0x01f4
#define IDE_SECTOR_CYLINDER_HIGH	0x01f5
#define IDE_DEVICE_HEAD			0x01f6

#define IDE_STATUS			0x01f7
#define IDE_COMMAND			0x01f7 /* Write */

#define IDE_ALTERNATE_STATUS		0x03f6
#define IDE_DEVICE_CONTROL		0x03f6 /* Write */

#define IDE_LBA 0x40

#define IDE_COMMAND_READ_SECTORS				0x20
#define IDE_COMMAND_IDLE					0xE3
#define IDE_COMMAND_IDENTIFY					0xEC
#define IDE_COMMAND_SET_FEATURES				0xEF
#define IDE_COMMAND_INITIALIZE_DEVICE_PARAMETERS		0x91

/* Don't loop forever */
#define TIMEOUT 1000

static int
wait_ready()
{
  unsigned long status;
  int i;
  for (i=0; i<TIMEOUT; i++)
    {
      status = ide_inb (IDE_ALTERNATE_STATUS);
      if ((status & IDE_RDY) == IDE_RDY)
        return 0;
      delay256 ();
    }
  putString("wait_ready() TIMEOUT\n");
  putString("status "); printouthex(status);
  status=ide_inb(IDE_ERROR);
  putString("error  "); printouthex(status);
  return -1;
}

static int
ide_register_check (void)
{
  ide_outb (0x55, IDE_SECTOR_CYLINDER_LOW);
  ide_outb (0xaa, IDE_SECTOR_CYLINDER_HIGH);
  if (ide_inb (IDE_SECTOR_CYLINDER_LOW) != 0x55)
    return -1;
  if (ide_inb (IDE_SECTOR_CYLINDER_HIGH) != 0xaa)
    return -1;
  return 0;
}

/* Reset the bus, set it polling mode. */
static int
ide_reset (void)
{
  unsigned long status;
  int i;

  ide_outb (0x04|0x02, IDE_DEVICE_CONTROL);
  delay10000 (); delay10000 (); delay10000 (); delay10000 ();

  /* Polling mode (nIEN = 0x02)*/
  ide_outb (0x02, IDE_DEVICE_CONTROL);
  delay10000 (); delay10000 (); delay10000 (); delay10000 ();

  for (i=0; i<TIMEOUT; i++)
    {
      status = ide_inb (IDE_ALTERNATE_STATUS);
      if ((status & IDE_BSY) == 0)
	break;
      delay256 ();
    }

  if (i == TIMEOUT)
    return -1;
  else if (ide_inb (IDE_ERROR) != 1)
    return -1;
  return 0;
}

/* Device selection protocol */
static int
ide_device_selection (int dev)
{
  unsigned long status;
  int i;

  for (i=0; i<TIMEOUT; i++)
    {
      status = ide_inb (IDE_STATUS);
      if ((status & IDE_BSY) == 0)
	break;
      delay256 ();
    }
  if (i == TIMEOUT)
    return -1;

  /* Polling */
  ide_outb (0x02, IDE_DEVICE_CONTROL);
  ide_outb (dev << 4, IDE_DEVICE_HEAD);
  delay256 ();

  return 0;
}

static int
ide_get_data (unsigned char *buf, int count)
{
  unsigned long status;
  unsigned short data;
  int i;

  while (count--)
    {
      /* Dummy read */
      status = ide_inb (IDE_ALTERNATE_STATUS);

      for (i=0; i<TIMEOUT; i++)
	{
	  status = ide_inb (IDE_ALTERNATE_STATUS);
	  if ((status & (IDE_DRQ | IDE_ERR)) != 0)
	    break;
	  delay256 ();
	}
      if (i == TIMEOUT)
	{
	  putString ("Timeout: ");
	  printouthex (status);
	  putString ("\n");
	  return -1;
	}

      if ((status & IDE_ERR))
	{				/* Error occurred */
	  status = ide_inb (IDE_STATUS);
	  putString ("Error: ");
	  printouthex (status);
	  putString ("\n");
	  return -1;
	}

      /* Read a data */
      for (i=0; i<256; i++)
	{
	  data = ide_inw (IDE_DATA);
#if defined(__LITTLE_ENDIAN__)
	  *buf++ = data & 0xff;
	  *buf++ = data >> 8;
#else
	  *buf++ = data >> 8;
	  *buf++ = data & 0xff;
#endif
	}
      for (i=0; i<TIMEOUT; i++)
        {
	  status = ide_inb (IDE_STATUS);
	  if (status == IDE_RDY | IDE_DSC)
	    break;
	  delay256 ();
	}
    }

  return 0;
}

static void
ide_fix_string (unsigned char *dest, unsigned char *src, int num)
{
  int i;

  for (i=0; i<num; i+=2)
    {
      *dest++ = *(src+1);
      *dest++ = *src;
      src += 2;
    }
  while (*--dest == ' ')
    /* do nothing */;
  ++dest;
  *dest++ = ' ';
  *dest++ = '\0';
}

/* Only for Device 0 (for now) */
/* XXX: Should be struct to support multiple devices.. */
static char ide_device_data;
static char ide_transfer_mode;
static short ide_sectors_per_track;
static short ide_max_head;

static int
ide_identify_device (dev)
{
  unsigned short buf[256];	/* No stack overflow? Cross fingered.. */
  unsigned char name[42];

  if (ide_device_selection (dev) < 0)
    return -1;

  wait_ready();
  ide_outb (IDE_COMMAND_IDENTIFY, IDE_COMMAND);
  delay256 ();

  if (ide_get_data ((unsigned char *)buf, 1) < 0)
    return -1;
  if (*buf & 0x04)		/* Incomplete */
    {
      /* XXX: Read the word #2, and printout the result */
      return -1;
    }

  /* XXX: check buf[64] for supported PIO mode */
  ide_transfer_mode = 3;

  putString ("Disk drive detected: ");

  ide_fix_string (name, (unsigned char *)&buf[27], 40);
  putString (name);
  ide_fix_string (name, (unsigned char *)&buf[23], 8);
  putString (name);
  ide_fix_string (name, (unsigned char *)&buf[10], 20);
  putString (name);
  putString ("\n");

  ide_sectors_per_track = buf[6];
  ide_max_head = buf[3] - 1;

  ide_device_data = 0;

  return 0;
}

#define IDE_SUBCOMMAND_SET_TRANSFER_MODE 0x03
static int
ide_set_transfer_mode (int dev, int mode)
{
  unsigned long status;
  int i;

  /* XXX: Don't set the mode, but use the default... for now */

  if (ide_device_selection (dev) < 0)
    return -1;

  wait_ready();
  ide_outb (IDE_SUBCOMMAND_SET_TRANSFER_MODE, IDE_FEATURES);
  ide_outb (0x00, IDE_SECTOR_COUNT); /* PIO Default mode */
  ide_outb (IDE_COMMAND_SET_FEATURES, IDE_COMMAND);
  delay256 ();

  for (i=0; i<TIMEOUT; i++)
    {
      status = ide_inb (IDE_ALTERNATE_STATUS);
      if ((status & IDE_BSY) == 0)
	break;
      delay256 ();
    }
  if (i == TIMEOUT)
    return -1;

  status = ide_inb (IDE_STATUS);
  putString ("Set Transfer Mode result: ");
  printouthex (status);

  return 0;
}

static int
ide_set_device_params (int dev, int sectors_per_track, int max_head)
{
  unsigned long status;
  int i;

  if (ide_device_selection (dev) < 0)
    return -1;

  wait_ready();
  ide_outb (sectors_per_track, IDE_SECTOR_COUNT);
  ide_outb (max_head | (dev << 4), IDE_DEVICE_HEAD);
  ide_outb (IDE_COMMAND_INITIALIZE_DEVICE_PARAMETERS, IDE_COMMAND);
  delay256 ();

  for (i=0; i<TIMEOUT; i++)
    {
      status = ide_inb (IDE_ALTERNATE_STATUS);
      if ((status & IDE_BSY) == 0)
	break;
      delay256 ();
    }
  if (i == TIMEOUT)
    return -1;

  status = ide_inb (IDE_STATUS);
  putString ("Initialize Device Parameters result: ");
  printouthex (status);

  return 0;
}

static int
ide_idle (int dev)
{
  unsigned long status;
  int i;

  if (ide_device_selection (dev) < 0)
    return -1;

  wait_ready();
  ide_outb (0x00, IDE_SECTOR_COUNT);
  ide_outb (IDE_COMMAND_IDLE, IDE_COMMAND);
  delay256 ();

  for (i=0; i<TIMEOUT; i++)
    {
      status = ide_inb (IDE_ALTERNATE_STATUS);
      if ((status & IDE_BSY) == 0)
	break;
      delay256 ();
    }
  if (i == TIMEOUT)
    return -1;

  status = ide_inb (IDE_STATUS);
  putString ("IDLE result: ");
  printouthex (status);

  return 0;
}

/* Read sectors at the address specified by LBA (linear block address),
   and copy the data into the memory at BUF.  The number of sectors are
   specified by COUNT (Should be 1-256).
 */
int
ide_read_sectors (int dev, unsigned long lba, unsigned char *buf, int count)
{
  /* Select the device */
  if (ide_device_selection (dev) < 0)
    return -1;

  /* Set the parameters */
  ide_outb (count, IDE_SECTOR_COUNT);
  ide_outb (lba & 0xff, IDE_SECTOR_NUMBER);
  ide_outb ((lba & 0xff00)>>8, IDE_SECTOR_CYLINDER_LOW);
  ide_outb ((lba & 0xff0000)>>16, IDE_SECTOR_CYLINDER_HIGH);
  ide_outb (((lba & 0x0f000000)>>24)|IDE_LBA, IDE_DEVICE_HEAD);

  /* Issue the command */
  ide_outb (IDE_COMMAND_READ_SECTORS, IDE_COMMAND);
  delay256 ();

  if (ide_get_data (buf, count) < 0)
    return -1;

  return 0;
}

static char ide_detected;
static char ide_started;

int
ide_detect_devices (void)
{
  ide_detected = 1;

  /* Clear device information */
  ide_device_data = -1;

  /* Only for Device 0 (for now) */

  if (ide_reset () < 0)		/* Failed on resetting bus */
    return -1;

  if (ide_device_selection (0) < 0)
    return -1;
  if (ide_register_check () < 0)
    return -1;
  if (ide_identify_device (0) < 0)
    return -1;

  return 0;
}

void
ide_startup_devices (void)
{
  if (ide_device_data != 0)
    return;

  /* Only for Device 0 (for now) */

  /* SET FEATURES (set transfer mode) */
  /* NOTE: This gets error with Compact Flash */
  ide_set_transfer_mode (0, ide_transfer_mode);

  /* INITIALIZE DEVICE PARAMETERS */
  ide_set_device_params (0, ide_sectors_per_track, ide_max_head);

  /* IDLE */
  ide_idle (0);

  ide_started = 1;
}
