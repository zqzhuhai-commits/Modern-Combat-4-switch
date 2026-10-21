/* SPDX-License-Identifier: MIT */
#include "frontend.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv) {
 if(argc<2)return 1;
 uint32_t *fb=calloc(1280*720,sizeof *fb); if(!fb)return 1;
 Scan s={0};if(argc>2)scan_files(argv[2],&s);
 for(int page=0;page<4;page++) {
  draw_frontend(fb,1280,page,&s,"");
  char name[1024];snprintf(name,sizeof name,"%s/page-%d.ppm",argv[1],page+1);
  FILE *f=fopen(name,"wb");if(!f)return 1;fprintf(f,"P6\n1280 720\n255\n");
  for(int i=0;i<1280*720;i++){unsigned char px[]={fb[i]&255,(fb[i]>>8)&255,(fb[i]>>16)&255};fwrite(px,1,3,f);}
  fclose(f);
 }
 free(fb);return 0;
}
