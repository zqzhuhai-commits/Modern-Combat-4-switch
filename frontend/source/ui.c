/* SPDX-License-Identifier: MIT */
#include "frontend.h"
#include "font.h"
#include <stdio.h>
#include <string.h>
#define RGB(r,g,b) ((uint32_t)(r)|((uint32_t)(g)<<8)|((uint32_t)(b)<<16)|0xff000000u)
#define BG RGB(10,17,27)
#define CARD RGB(18,31,46)
#define GOLD RGB(223,185,116)
#define TEXT RGB(230,236,243)
#define MUTED RGB(148,167,185)
#define GREEN RGB(113,206,165)
static uint32_t *fb; static unsigned pitch;
static void box(int x,int y,int w,int h,uint32_t c) {
 for(int j=y;j<y+h&&j<720;j++) for(int i=x;i<x+w&&i<1280;i++) if(i>=0&&j>=0)fb[j*pitch+i]=c;
}
static void text(int x,int y,const char *s,uint32_t color,int scale) {
 for(;*s;s++,x+=13*scale) {
  unsigned c=(unsigned char)*s;
  if(c<32||c>126)c='?';
  for(int j=0;j<26;j++) for(int i=0;i<13;i++)
   if(glyphs[c-32][j]&(1<<i))box(x+i*scale,y+j*scale,scale,scale,color);
 }
}
static void line(int y,const char *a,const char *b,int good) {
 text(352,y,a,MUTED,1); text(835,y,b,good?GREEN:GOLD,1);
 box(352,y+38,800,1,RGB(35,49,64));
}
void draw_frontend(uint32_t *pixels,unsigned stride,int page,const Scan *s,const char *notice) {
 fb=pixels;pitch=stride; box(0,0,1280,720,BG);
 box(0,0,280,720,CARD); box(34,42,48,3,GOLD);
 text(34,66,"SHADOW",TEXT,2); text(34,118,"GUARDIAN",TEXT,2);
 text(36,186,"SWITCH FRONTEND",GOLD,1);
 const char *tabs[]={"Overview","Game files","Setup guide","Diagnostics"};
 for(int i=0;i<4;i++) {
  int y=280+i*64;
  if(page==i) {box(22,y-7,236,49,RGB(37,52,66));box(22,y-7,3,49,GOLD);}
  text(40,y,tabs[i],page==i?GOLD:MUTED,1);
 }
 text(35,639,"PREVIEW " FRONTEND_VERSION,MUTED,1);
 text(332,42,tabs[page],TEXT,2);
 text(334,105,"Preparation build / gameplay runtime not integrated",GOLD,1);
 box(332,157,890,3,GOLD);
 char a[120];
 if(page==0) {
  box(332,190,890,170,CARD);
  text(354,209,"YOUR EXPEDITION STARTS HERE",GOLD,1);
  text(354,254,"Prepare your original PortMaster game files.",TEXT,1);
  text(354,292,"This build does not start the game yet.",MUTED,1);
  line(397,"Original ARMv7 library",s->library_verified?"SHA256 MATCH":s->library_present?"UNKNOWN BUILD":"MISSING",s->library_verified);
  snprintf(a,sizeof a,"%d file(s)",s->data_files); line(457,"Gameloft data",a,s->data_files>0);
  line(517,"Game runtime","NOT READY",0);
  text(352,586,"A  Inspect files       Y  Rescan",TEXT,1);
 } else if(page==1) {
  text(352,194,"sd:/switch/shadowguardian_nx/",GOLD,1);
  line(252,"Original library SHA256",s->library_verified?"MATCH":s->library_present?"UNKNOWN":"MISSING",s->library_verified);
  snprintf(a,sizeof a,"%.1f MiB",s->data_bytes/1048576.0);line(312,"Nonempty game data",a,s->data_files>0);
  const char *name=strrchr(s->apk,'/'); name=name?name+1:s->apk;
  text(352,378,"Optional APK candidate:",MUTED,1);
  snprintf(a,sizeof a,"%.61s",*name?name:"Not required for PortMaster data");text(352,414,a,TEXT,1);
  text(352,474,"Known library: SHA256; data: file count/size.",MUTED,1);
  text(352,510,"Resource completeness/gameplay: unverified.",GOLD,1);
  text(352,562,s->limited?"Scan limit reached; result is incomplete.":"Y  Rescan files     X  Export diagnostic log",TEXT,1);
 } else if(page==2) {
  text(352,200,"01  Copy libshadowguardian.so to:",GOLD,1);
  text(352,236,"    switch/shadowguardian_nx/",TEXT,1);
  text(352,292,"02  Copy original PortMaster game data under:",GOLD,1);
  text(352,328,"    gameloft/games/GloftSGHP/",TEXT,1);
  text(352,384,"03  Press Y to scan; X to save a report.",GOLD,1);
  text(352,438,"Also accepts the original library at:",MUTED,1);
  text(352,474,"lib/armeabi-v7a/libshadowguardian.so",TEXT,1);
  text(352,530,"Both Gameloft/Games and gameloft/games work.",MUTED,1);
  text(352,566,"The frontend never alters APKs, data or saves.",MUTED,1);
 } else {
  line(200,"Frontend","AARCH64 NATIVE",1);
  line(260,"Gameplay integration","PENDING",0);
  snprintf(a,sizeof a,"%d",s->errors);line(320,"File scan I/O errors",a,s->errors==0);
  text(352,390,"X  Write frontend.log in the game folder",TEXT,1);
  text(352,432,"Report includes paths and scan results only.",MUTED,1);
  text(352,484,"Hardware install/display/input: unverified.",GOLD,1);
  text(352,532,"No original game files are included.",MUTED,1);
  text(352,568,"Source: zqzhuhai-commits/shadowguardian-switch",MUTED,1);
 }
 box(332,625,890,1,RGB(35,49,64));
 text(334,642,notice&&*notice?notice:"L/R  Change page    Y  Rescan    X  Report    +  Exit",TEXT,1);
 text(334,680,"Touch a tab to navigate.  B returns to Overview.",MUTED,1);
}
