/*------------------------------------------
	SH-2000 Hardware depend program
------------------------------------------*/

#include "io.h"

#define	PORT_PCDR	0xa4000124
#define	PORT_PDDR	0xa4000126
#define	PORT_PFDR	0xa400012a

int sh2000_read_dipsw(void)
{
	unsigned char dsw;

	dsw = (p4_inb(PORT_PCDR) >> 6) & 3;
	dsw |= (p4_inb(PORT_PFDR) & 0xf) << 2;
	if (p4_inb(PORT_PDDR) & 2)
		dsw |= 0x40;
	if (p4_inb(PORT_PDDR) & 1)
		dsw |= 0x80;
	return ~dsw & 0xff;
}
