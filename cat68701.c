/*------------------------------------------
	CAT-68701 Hardware depend program
  (C) 2001 Yutarou Ebihara ebihara@si-linux.com
------------------------------------------*/


int cat68701_read_dipsw(){
	unsigned short i;
	i = *(volatile unsigned short *)0xb4007000;
	return (i >> 12)&0xf;
}
