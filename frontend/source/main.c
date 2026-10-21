/* SPDX-License-Identifier: MIT */
#include <switch.h>
#include "frontend.h"
#include <stdio.h>
#include <sys/stat.h>
int main(void) {
    padConfigureInput(1,HidNpadStyleSet_NpadStandard);
    PadState pad; padInitializeDefault(&pad); hidInitializeTouchScreen();
    Framebuffer fb;
    Result rc=framebufferCreate(&fb,nwindowGetDefault(),1280,720,PIXEL_FORMAT_RGBA_8888,2);
    if (R_FAILED(rc)) return 1;
    rc=framebufferMakeLinear(&fb);
    if (R_FAILED(rc)) {framebufferClose(&fb);return 1;}
    Scan scan; scan_files(GAME_ROOT,&scan);
    char notice[80]=""; int page=0,touching=0;
    while(appletMainLoop()) {
        padUpdate(&pad); u64 down=padGetButtonsDown(&pad);
        if(down&HidNpadButton_Plus)break;
        if(down&(HidNpadButton_R|HidNpadButton_Down|HidNpadButton_Right)) {page=(page+1)%4;notice[0]=0;}
        if(down&(HidNpadButton_L|HidNpadButton_Up|HidNpadButton_Left)) {page=(page+3)%4;notice[0]=0;}
        if(down&HidNpadButton_B) {page=0;notice[0]=0;}
        if((down&HidNpadButton_A)&&page==0) {page=1;notice[0]=0;}
        if(down&HidNpadButton_Y) {
            scan_files(GAME_ROOT,&scan);
            snprintf(notice,sizeof notice,"Scan complete. Game compatibility remains unverified.");
        }
        if(down&HidNpadButton_X) {
            /* Create only the designated frontend report directory when requested. */
            mkdir("sdmc:/switch",0777); mkdir(GAME_ROOT,0777);
            int ok=save_report(GAME_ROOT,&scan)==0;
            snprintf(notice,sizeof notice,"%s",ok?"Saved: switch/shadowguardian_nx/frontend.log":"Report could not be saved. Check SD card access.");
        }
        HidTouchScreenState ts={0};hidGetTouchScreenStates(&ts,1);
        if(ts.count&&!touching) {
            unsigned x=ts.touches[0].x,y=ts.touches[0].y;
            if(x>=22&&x<=258&&y>=273&&y<273+4*64) {page=(y-273)/64;notice[0]=0;}
        }
        touching=ts.count>0;
        u32 stride;u32 *pixels=framebufferBegin(&fb,&stride);
        draw_frontend(pixels,stride/4,page,&scan,notice);
        framebufferEnd(&fb);
    }
    framebufferClose(&fb);return 0;
}
