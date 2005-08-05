/*
 * Program for building ROM image of CQSH3
 * Written by Yutarou Ebihara
 * This file may be distributed under GNU GPL
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	unsigned char buff[64 * 1024], *p = buff;
	int i;

	if (argc < 3)
		exit();
	printf("input file=%s\n", argv[1]);
	printf("output file=%s\n", argv[2]);

	fp = fopen(argv[1], "r");

	while (!feof(fp)) {
		*p++ = fgetc(fp);
	}
	fclose(fp);
	printf("file size= %d\n", p - buff);

	fp = fopen(argv[2], "w");

	for (i = 0; i < 16 * 1024; i++) {
		fputc(buff[i * 2], fp);
	}
	for (i = 0; i < 16 * 1024; i++) {
		fputc(buff[i * 2 + 1], fp);
	}
	fclose(fp);
}
