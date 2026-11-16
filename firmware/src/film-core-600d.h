/* 600D film UI/filter bridge, included by menu.c after native film drawing. */
#include "film-bridge.h"
#include "vram.h"
static struct film_hooks film_hooks;
static volatile int film_mode;
void photo_film_register(const struct film_hooks *h){if(h)film_hooks=*h;else memset(&film_hooks,0,sizeof(film_hooks));}
void photo_film_mode(int mode){film_mode=mode;if(mode)gui_open_menu();else menu_redraw();}
int photo_film_key(int key){int (*fn)(int)=film_hooks.key;return fn?fn(key):1;}
int photo_film_draw(void){int (*fn)(int)=film_hooks.draw;if(!film_mode || !fn){return 0;}int result=0;BMP_LOCK(result=fn(film_mode));return result;}
int photo_film_filter_active(void){
    int (*fn)(void)=film_hooks.filter_ready;
    return lv && !RECORDING && !gui_menu_shown() && !film_mode && vram_lv.width==720 && vram_lv.height==480 && vram_lv.pitch==1440 && fn && fn();
}
int photo_film_filter_update(void){
    void (*fn)(const uint32_t*,uint32_t*,unsigned)=film_hooks.filter;
    if(!fn || !photo_film_filter_active())return 0;
    uint32_t *src=NULL,*dst=NULL;display_filter_get_buffers(&src,&dst);
    if(!src || !dst || src==dst)return 0;
    fn(src,dst,720u*480u/2u);return 1;
}
static uint32_t film_saved_palette[160*2];
static int film_palette_live,film_palette_count;
void photo_film_palette_release(void){
    if(!film_palette_live)return;
    bmp_fill(COLOR_BLACK,0,0,720,480);
    for(int i=0;i<film_palette_count;i++)for(int bank=0;bank<2;bank++)EngDrvOut(LCD_Palette[(80+i)*3+bank*0x300],film_saved_palette[i*2+bank]);
    film_palette_live=0;
}
static void film_apply_palette(const unsigned char *palette,int count){
    if(film_palette_live && film_palette_count!=count)photo_film_palette_release();
    if(!film_palette_live){
        for(int i=0;i<count;i++) {
            int r=palette[3*i],g=palette[3*i+1],b=palette[3*i+2];
            int yy=(299*r+587*g+114*b)/1000,u=(b-yy)*564/1000,v=(r-yy)*713/1000;
            uint32_t entry=0x03000000u|((uint32_t)(yy*235/255)<<16)|((uint32_t)((u*235/255)&255)<<8)|((v*235/255)&255);
            for(int bank=0;bank<2;bank++){film_saved_palette[i*2+bank]=shamem_read(LCD_Palette[(80+i)*3+bank*0x300]);EngDrvOut(LCD_Palette[(80+i)*3+bank*0x300],entry);}
        }
        film_palette_live=1;film_palette_count=count;
    }
}
void photo_film_review_pixels(const unsigned char *pixels,const unsigned char *palette){
    if(!pixels){return;}film_apply_palette(palette,144);
    unsigned char *vram=bmp_vram();
    /* Keep top/bottom rows for the source/output caption and controls. */
    for(int y=0;y<400;y++)for(int x=0;x<600;x++)
        bmp_putpixel_fast(vram,60+x,38+y,pixels[(y*480/400)*720+x*720/600]);
}

void photo_film_art_pixels(const unsigned char *pixels,const unsigned char *palette,int x,int y,int w,int h){
    if(!pixels || w<=0 || h<=0 || x<0 || y<0 || x+w>720 || y+h>480)return;
    film_apply_palette(palette,160);unsigned char *vram=bmp_vram();
    for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++){
        unsigned char p=pixels[(yy*194/h)*176+xx*176/w];
        if(p)bmp_putpixel_fast(vram,x+xx,y+yy,p);
    }
}
