
/*
 * Initialization/setup code for the SH7751 PCI Controller (PCIC).
 * Code sequence taken largely from arch/sh/kernel/pci-bigsur.c,
 * put together by Dustin McIntire (dustin@sensoria.com). License
 * is GPL.
 */

#include "config.h"
#include "defs.h"
#include "io.h"

/* Register addresses and such */
#define SH7751_BCR1	(volatile unsigned long *)0xFF800000
  #define BCR1_BREQEN	0x00080000
#define SH7751_BCR2	(volatile unsigned short*)0xFF800004
#define SH7751_WCR1	(volatile unsigned long *)0xFF800008
#define SH7751_WCR2	(volatile unsigned long *)0xFF80000C
#define SH7751_WCR3	(volatile unsigned long *)0xFF800010
#define SH7751_MCR	(volatile unsigned long *)0xFF800014

#define SH7751_PCICONF0	(volatile unsigned long *)0xFE200000
#define PCI_SH7751_ID_MASK	(0x35051054 & 0x350e1054) /* SH7751: 0x35051054
							     SH7751R:0x350e1054 */
#define SH7751_PCICONF1	(volatile unsigned long *)0xFE200004
  #define SH7751_PCICONF1_WCC 0x00000080
  #define SH7751_PCICONF1_PER 0x00000040
  #define SH7751_PCICONF1_BUM 0x00000004
  #define SH7751_PCICONF1_MES 0x00000002
  #define SH7751_PCICONF1_IOS 0x00000001
  #define SH7751_PCICONF1_CMDS 0x000000C7
#define SH7751_PCICONF2	(volatile unsigned long *)0xFE200008
  #define SH7751_PCI_HOST_BRIDGE 0x6
#define SH7751_PCICONF3	(volatile unsigned long *)0xFE20000C
#define SH7751_PCICONF4	(volatile unsigned long *)0xFE200010
  #define SH7751_PCICONF4_IOBASE 0xab000001
#define SH7751_PCICONF5	(volatile unsigned long *)0xFE200014
#define SH7751_PCICONF6	(volatile unsigned long *)0xFE200018

#define SH7751_PCICR	(volatile unsigned long *)0xFE200100
  #define SH7751_PCICR_PREFIX 0xa5000000	     
  #define SH7751_PCICR_PRST    0x00000002
  #define SH7751_PCICR_CFIN    0x00000001
#define SH7751_PCILSR0	(volatile unsigned long *)0xFE200104
#define SH7751_PCILSR1	(volatile unsigned long *)0xFE200108
#define SH7751_PCILAR0	(volatile unsigned long *)0xFE20010C
#define SH7751_PCILAR1	(volatile unsigned long *)0xFE200110
#define SH7751_PCIMBR	(volatile unsigned long *)0xFE2001C4
#define SH7751_PCIIOBR	(volatile unsigned long *)0xFE2001C8
#define SH7751_PCIPINT	(volatile unsigned long *)0xFE2001CC
  #define SH7751_PCIPINT_D3  0x00000002
  #define SH7751_PCIPINT_D0  0x00000001
#define SH7751_PCIPINTM	(volatile unsigned long *)0xFE2001D0
#define SH7751_PCICLKR	(volatile unsigned long *)0xFE2001D4
  #define SH7751_PCICLKR_PREFIX 0xa5000000
#define SH7751_PCIBCR1	(volatile unsigned long *)0xFE2001E0
#define SH7751_PCIBCR2	(volatile unsigned long *)0xFE2001E4
#define SH7751_PCIWCR1	(volatile unsigned long *)0xFE2001E8
#define SH7751_PCIWCR2	(volatile unsigned long *)0xFE2001EC
#define SH7751_PCIWCR3	(volatile unsigned long *)0xFE2001F0
#define SH7751_PCIMCR	(volatile unsigned long *)0xFE2001F4

#define SH7751_PCI_MEM_BASE 0xFD000000
#define SH7751_PCI_MEM_SIZE 0x01000000
#if defined(CONFIG_CPU_SUBTYPE_SH_R)
/* on SE7751R, SH7751_PCI_IO_BASE must be 0 otherwise Ether-chip doesn't work
 * on SE7751, I don't know.
 */
#define SH7751_PCI_IO_BASE  0
#else
#define SH7751_PCI_IO_BASE  0xFE240000
#endif
#define SH7751_PCI_IO_SIZE  0x00040000

#define SH7751_CS3_BASE_ADDR   0x0C000000
#define SH7751_P2CS3_BASE_ADDR 0xAC000000

#define SH7751_PCIPAR	(volatile unsigned long *)0xFE2001C0
#define SH7751_PCIPDR	(volatile unsigned long *)0xFE200220

unsigned long pci_nextio;
unsigned long pci_nextmem;

/* Return 0 if ok, else err code */
int init_pcic(void)
{
  /* Double-check that we're a 7751 chip */
  if ((p4_in(SH7751_PCICONF0) & PCI_SH7751_ID_MASK) != PCI_SH7751_ID_MASK)
    return 1;

  /* Double-check some BSC config settings */
  /* (Area 3 non-MPX 32-bit, PCI bus pins) */
  if ((p4_in(SH7751_BCR1) & 0x20008) == 0x20000)
    return 2;
  if ((p4_in(SH7751_BCR2) & 0xC0) != 0xC0)
    return 3;
  if (p4_in(SH7751_BCR2) & 0x01)
    return 4;

  /* Force BREQEN in BCR1 to allow PCIC access */
  p4_out(SH7751_BCR1, (p4_in(SH7751_BCR1) | BCR1_BREQEN));

  /* Toggle PCI reset pin */
  p4_out(SH7751_PCICR, (SH7751_PCICR_PREFIX | SH7751_PCICR_PRST));
  sleep128(32);
  p4_out(SH7751_PCICR, SH7751_PCICR_PREFIX);
	
  /* Set cmd bits: WCC, PER, BUM, MES */
  /* (Addr/Data stepping, Parity enabled, Bus Master, Memory enabled) */
  p4_out(SH7751_PCICONF1, SH7751_PCICONF1_CMDS);

  /* Define this host as the host bridge */
  p4_out(SH7751_PCICONF2, (SH7751_PCI_HOST_BRIDGE << 24));

  p4_out(SH7751_PCICONF4, SH7751_PCICONF4_IOBASE);

  /* Force PCI clock(s) on */
  p4_out(SH7751_PCICLKR, SH7751_PCICLKR_PREFIX);

  /* Clear powerdown IRQs, also mask them (unused) */
  p4_out(SH7751_PCIPINT, (SH7751_PCIPINT_D0 | SH7751_PCIPINT_D3)); 
  p4_out(SH7751_PCIPINTM, 0);

  /* Set up target memory mappings (for external DMA access) */
  /* Map both P0 and P2 range to Area 3 RAM for ease of use */

  p4_out(SH7751_PCILSR0, CONFIG_MEMORY_SIZE-1);
  p4_out(SH7751_PCILAR0, SH7751_CS3_BASE_ADDR);
  p4_out(SH7751_PCILSR1, CONFIG_MEMORY_SIZE-1);
  p4_out(SH7751_PCILAR1, SH7751_CS3_BASE_ADDR);

  p4_out(SH7751_PCICONF5, SH7751_CS3_BASE_ADDR);
  p4_out(SH7751_PCICONF6, SH7751_P2CS3_BASE_ADDR);

  /* Map memory window to same address on PCI bus */
  p4_out(SH7751_PCIMBR, SH7751_PCI_MEM_BASE);
  pci_nextmem = SH7751_PCI_MEM_BASE;
  
  /* Map IO window to same address on PCI bus */
  p4_out(SH7751_PCIIOBR, SH7751_PCI_IO_BASE);
  pci_nextio = SH7751_PCI_IO_BASE;

  /* Copy BSC registers into PCI BSC */
  p4_out(SH7751_PCIBCR1, p4_in(SH7751_BCR1));
  p4_out(SH7751_PCIBCR2, p4_in(SH7751_BCR2));
  p4_out(SH7751_PCIWCR1, p4_in(SH7751_WCR1));
  p4_out(SH7751_PCIWCR2, p4_in(SH7751_WCR2));
  p4_out(SH7751_PCIWCR3, p4_in(SH7751_WCR3));
  p4_out(SH7751_PCIMCR , p4_in(SH7751_MCR));

  /* Finally, set central function init complete */
  p4_out(SH7751_PCICR, (SH7751_PCICR_PREFIX | SH7751_PCICR_CFIN));
  return 0;
}

unsigned long pci_read_config_dword(unsigned long offset)
{
  p4_out(SH7751_PCIPAR, offset);
  return p4_in(SH7751_PCIPDR);
}

void pci_write_config_dword(unsigned long offset, unsigned long data)
{
  p4_out(SH7751_PCIPAR, offset);
  p4_out(SH7751_PCIPDR, data);
}  

