/* $Id: flash.c,v 1.1 2001/06/07 04:47:07 sugioka Exp $

   Flash ROM Writing routine for TC58FVB800FT-85

   Based on the code in CQ RISC Evaluatin Kit for SH-4 "tool/flash/flash.c"
   Copyright (C) 1999 CQ Publishing.

   sh-stub/flash.c

   Distributed under the permission of CQ Publishing.
   Absolutely NO Warranty.
 */

#include "io.h"

#define LED_ADDRESS  0xB4000000

#define FLASH_ADDRESS 0xA0000000

static void
flash_reset (void)
{
	p4_outw(FLASH_ADDRESS+0x5555*2,0x00AA);
	p4_outw(FLASH_ADDRESS+0x2AAA*2,0x0055);
	p4_outw(FLASH_ADDRESS+0x5555*2,0x00F0);
	(void)p4_inw(FLASH_ADDRESS);
}


static inline unsigned short
swap(unsigned short w)
{
  unsigned short r;

  r = ((w&0xff)<<8)|(w>>8);
  return r;
}

static int
flash_write_word(unsigned long address, unsigned short data)
{
	unsigned short status0, status1;

	p4_outw(FLASH_ADDRESS+0x5555*2,0x00AA);
	p4_outw(FLASH_ADDRESS+0x2AAA*2,0x0055);
	p4_outw(FLASH_ADDRESS+0x5555*2,0x00A0);

#ifdef SWAP
	p4_outw(address,swap(data));
#else
	p4_outw(address,data);
#endif

	for(;;) {
		status0 = p4_inw(FLASH_ADDRESS);
		status1 = p4_inw(FLASH_ADDRESS);

		if ((status0 & 0x40) == (status1 & 0x40))
			return 0;
		if (status0 & 0x20)
			return 1;
	}

	return 0;
}

static int
flash_erase_all(void)
{
	unsigned short status0, status1;

	p4_outw(FLASH_ADDRESS+0x5555*2,0x00AA);
	p4_outw(FLASH_ADDRESS+0x2AAA*2,0x0055);
	p4_outw(FLASH_ADDRESS+0x5555*2,0x0080);
	p4_outw(FLASH_ADDRESS+0x5555*2,0x00AA);
	p4_outw(FLASH_ADDRESS+0x2AAA*2,0x0055);
	p4_outw(FLASH_ADDRESS+0x5555*2,0x0010);

	for(;;) {
		status0 = p4_inw(FLASH_ADDRESS);
		status1 = p4_inw(FLASH_ADDRESS);

		if ((status0 & 0x40) == (status1 & 0x40))
			return 0;
		if (status0 & 0x20)
			return 1;
	}

	return 0;
}

static int
flash_write(unsigned long address, unsigned long size)
{
	int result, offset;
	unsigned short data;

	flash_reset();

	p4_outb(LED_ADDRESS,5);

	if (flash_erase_all() != 0)
		return -2;
        else
		for (offset = 0; offset < size; offset += 2) {
			data = readw(address+offset);
			result = flash_write_word(FLASH_ADDRESS+offset, data);
			if (result)
				return -1;
		}

	return 0;
}

void
do_flash()
{
	unsigned long address;
	unsigned long size;

	address = 0x88100000;
	size = 8192*4;

	p4_outb(LED_ADDRESS,6);

	if (flash_write(address, size) == 0)
		p4_outb(LED_ADDRESS,0);
	else
		p4_outb(LED_ADDRESS,1);
	for(;;);
}
