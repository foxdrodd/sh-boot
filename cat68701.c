/*------------------------------------------
	CAT-68701 Hardware depend program
  (C) 2001 Yutarou Ebihara ebihara@si-linux.com
------------------------------------------*/

int cat68701_read_dipsw()
{
	unsigned short i;
	i = *(volatile unsigned short *)0xb4007000;
	return (i >> 12) & 0xf;
}

#define MCR_ADDR 0xffffff68

int cat68701_read_mem_size()
{
	int d;
	d = *(volatile unsigned short *)MCR_ADDR;

	if (d == 0x116c)
		return 32 << 20;
	else
		return 16 << 20;
}
