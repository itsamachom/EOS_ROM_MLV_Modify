/* One-level seven-canister picker and on-camera result viewer. */
static volatile int ui_mode,pending_preview=1,pending_view;
static int browsing;
static uint32_t *live_table;
static volatile int live_ready;
static char live_message[80]="Live View: loading colours";
static void *live_sem;
static unsigned char *review_pixels;
static unsigned char review_palette[144*3];
static char last_preview[128],last_saved[128],last_source[128];
static int last_saved_film;
#include "trial-art.h"
static unsigned char *trial_art;
static unsigned char trial_palette[160*3];
static void load_trial_art(void){
    if(trial_art)return;
    uint32_t size=0;if(FIO_GetFileSize("ML/LUTDATA/FILM7.BIN",&size) || size!=FILM7_BYTES)return;
    unsigned char *next=malloc(size),*dma=fio_malloc(8192);FILE *f=NULL;int ok=0;
    if(!next || !dma)goto done;
    f=FIO_OpenFile("ML/LUTDATA/FILM7.BIN",O_RDONLY|O_SYNC);if(!f)goto done;
    for(unsigned off=0;off<size;){unsigned n=MIN(size-off,8192);if(FIO_ReadFile(f,dma,n)!=(int)n)goto done;memcpy(next+off,dma,n);off+=n;}
    if(~film_crc(~0u,next,FILM7_META_BYTES)!=FILM7_META_CRC)goto done;
    for(int i=0;i<7;i++){uint32_t expected;memcpy(&expected,next+16+640+i*4,4);if(~film_crc(~0u,next+FILM7_META_BYTES+i*FILM7_CELL_BYTES,FILM7_CELL_BYTES)!=expected)goto done;}
    for(int i=0;i<160;i++){uint32_t rgb;memcpy(&rgb,next+16+i*4,4);trial_palette[i*3]=rgb>>16;trial_palette[i*3+1]=rgb>>8;trial_palette[i*3+2]=rgb;}
    trial_art=next;next=NULL;ok=1;
 done:
    if(f){FIO_CloseFile(f);}if(dma){fio_free(dma);}if(next){free(next);}(void)ok;
}
static const char *leaf(const char *p){const char *q=strrchr(p,'/');return q?q+1:p;}
static void load_live_preview(void){
    if(!pending_preview){return;}
    int index=current_index();pending_preview=0;
    unsigned char *dma=fio_malloc(8192);uint32_t *next=malloc(32768*4);FILE *f=NULL;int ok=0;
    char path[64];snprintf(path,sizeof(path),"ML/LUTDATA/LV%03d.FLV",films[index]);
    if(!dma || !next)goto done;
    f=FIO_OpenFile(path,O_RDONLY|O_SYNC);if(!f || FIO_ReadFile(f,dma,16)!=16)goto done;
    uint32_t h[4];memcpy(h,dma,16);if(h[0]!=0x36564c46u || h[1]!=(unsigned)films[index] || h[2]!=32768*4)goto done;
    unsigned crc=~0u;
    for(unsigned off=0;off<32768*4;off+=8192){if(FIO_ReadFile(f,dma,8192)!=8192)goto done;memcpy((unsigned char*)next+off,dma,8192);crc=film_crc(crc,dma,8192);}
    if(~crc!=h[3])goto done;
    if(index!=current_index()){pending_preview=1;goto done;}
    take_semaphore(live_sem,0);uint32_t *old=live_table;live_table=next;next=NULL;live_ready=1;snprintf(live_message,sizeof(live_message),"Live View colours ready");give_semaphore(live_sem);if(old)free(old);ok=1;
 done:
    if(f){FIO_CloseFile(f);}if(dma){fio_free(dma);}if(next){free(next);}
    if(!ok)snprintf(live_message,sizeof(live_message),"Live View unavailable - check preview table files");
}
static int film_lv_ready(void){return live_ready && !stopping && !is_movie_mode();}
static void film_lv_filter(const uint32_t *src,uint32_t *dst,unsigned words){
    take_semaphore(live_sem,0);
    if(live_ready && live_table && words<=720u*480u/2u)film_apply_uyvy(src,dst,words,live_table);
    else if(words<=720u*480u/2u)memcpy(dst,src,words*4);
    give_semaphore(live_sem);
}
#include "../../src/film-bridge.h"
static int film_ui_key(int key);
static void close_film_ui(int stop_menu){
    photo_film_palette_release();ui_mode=0;photo_film_mode(0);if(stop_menu)gui_stop_menu();
}
#ifndef LUTJPEG_HOST_TEST
#include "film-picker.h"
static int film_ui_draw(int mode){
    bmp_fill(COLOR_BLACK,0,0,720,480);
    if(mode==1){
        film_picker_draw(browsing,current_index());
    }else if(mode==2){
        if(review_pixels){photo_film_review_pixels(review_pixels,review_palette);photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),12,4,696,leaf(last_saved));}
        else photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,100,688,last_saved[0]?"JPEG saved; display preview unavailable":"No converted photo yet");
        photo_film_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),12,444,696,"Film result   MENU: back   PLAY: Canon playback");
    }else{
        photo_film_text(FONT(FONT_LARGE,COLOR_WHITE,COLOR_BLACK),16,8,688,"Photo conversion status");
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,62,688,current_job.path[0]?leaf(current_job.path):leaf(last_source));
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,108,688,status);
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,164,200,"Waiting");count_text(240,164,qcount);
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,210,200,"Saved this session");count_text(300,210,completed);
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,256,200,"Failed");count_text(240,256,failed);
        photo_film_text(FONT(FONT_MED,COLOR_YELLOW,COLOR_BLACK),16,310,688,last_saved);
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,356,688,live_message);
        photo_film_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),16,444,688,"PLAY: view film photo   MENU: back");
    }return 1;
}
static void register_film_ui(void){struct film_hooks hooks={film_ui_key,film_ui_draw,film_lv_ready,film_lv_filter};photo_film_register(&hooks);}
#else
static void register_film_ui(void){}
#endif

#include "film-picker-key.h"
