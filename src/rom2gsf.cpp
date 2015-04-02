/*
** Simple utility to convert a SNES rom to SNSF
** Based on EXE2PSF code, written by Neill Corlett
** Released under the terms of the GNU General Public License
**
** You need zlib to compile this.
** It's available at http://www.gzip.org/zlib/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"

Byte   compbuf[0x2000000];
Byte uncompbuf[0x2000000];

uLong entrypoint;
uLong load_offset;
uLong rom_size;

unsigned long endianflip(unsigned long value)
{
	return ((value & 0xFF) << 24) + ((value & 0xFF00) << 8) + ((value & 0xFF0000) >> 8) + ((value & 0xFF000000) >> 24);
}

unsigned char hexdigit(unsigned char digit)
{
	if(digit>='0'&&digit<='9')
		return digit-'0';
	if(digit>='A'&&digit<='F')
		return digit-'A'+10;
    if(digit>='a'&&digit<='f')
		return digit-'a'+10;
	return 0;
}

unsigned long ahextoi(const char *buffer)
{
	char *buf = (char*) buffer;
	int i=0;
	unsigned long integer=0;
	while(*buf)
	{
		integer<<=4;
		integer+=hexdigit(*buf);
		buf++;
		
	}

	return integer;
}

int doexe2snsf(const char *from, const char *to) {
  FILE *f;
  uLong ucl;
  uLong cl;
  uLong ccrc;
  int r;



  fprintf(stderr, "%s->%s: ", from, to);

  f=fopen(from,"rb");if(!f){perror(from);return 1;}
  ucl=fread(uncompbuf+8,1,sizeof(uncompbuf)-8,f);
  fclose(f);
  
  entrypoint = load_offset;
  rom_size = ucl;
  memcpy(uncompbuf+0, &load_offset, sizeof(load_offset));
  memcpy(uncompbuf+4, &rom_size, sizeof(rom_size));
  
//  fprintf(stdout,"uncompressed: %ld bytes\n",ucl);fflush(stdout);

  cl = sizeof(compbuf);
  r=compress2(compbuf,&cl,uncompbuf,ucl+12,9);
  if(r!=Z_OK){fprintf(stderr,"zlib compress2() failed (%d)\n", r);return 1;}

//  fprintf(stdout,"compressed: %ld bytes\n",cl);fflush(stdout);

  f=fopen(to,"wb");if(!f){perror(to);return 1;}
  fputc('P',f);fputc('S',f);fputc('F',f);fputc(0x23,f);
  fputc(0,f);fputc(0,f);fputc(0,f);fputc(0,f);
  fputc(cl  >> 0,f);
  fputc(cl  >> 8,f);
  fputc(cl  >>16,f);
  fputc(cl  >>24,f);
  ccrc=crc32(crc32(0L, Z_NULL, 0), compbuf, cl);
  fputc(ccrc>> 0,f);
  fputc(ccrc>> 8,f);
  fputc(ccrc>>16,f);
  fputc(ccrc>>24,f);
  fwrite(compbuf,1,cl,f);
  fclose(f);
  fprintf(stderr,"ok\n");
  return 0;
}



int main(int argc, char **argv) {
  char s[1000];
  int i;
  int errors = 0;
  if(argc<2){
    fprintf(stderr,"usage: %s [-offset <hexadecimal offset>] <snes-rom-files>\n", argv[0]);
    return 1;
  }
  load_offset = 0;
  for(i = 1; i < argc; i++) {
    if(!stricmp(argv[i],"-offset"))
	{
		i++;
		load_offset = ahextoi(argv[i]);
		i++;
	}
    strncpy(s, argv[i], 900);
    s[900] = 0;
    { char *e = s + strlen(s) - 4;
      if(!strcmp(e, ".smc")) { *e = 0; }
    }

	

    strcat(s, ".snsf");
    errors += doexe2snsf(argv[i], s);

  }


  fprintf(stderr, "%d error(s)\n", errors);
  return 0;
}
