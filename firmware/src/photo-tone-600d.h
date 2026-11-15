/* Independent native JPEG picker. GPL-2.0-or-later.
 * Recipe values credited in photo-tone-recipes-600d.h; graphics/UI written for this fork.
 * Included after photo_text; all UI state is protected by menu_sem. */
#include "photo-tone-engine.h"
static int photo_tone_opened,photo_tone_group,photo_tone_selected[4],photo_tone_confirm;
static int photo_tone_working,photo_tone_step,photo_tone_steps;
static const char *photo_tone_message="JPEG / RAW+JPEG in Canon";
static struct photo_tone_session photo_tone_session={.active=-1};
static char photo_tone_read_detail[96];
#ifndef PHOTO_TONE_TEST_ADAPTER
static volatile int photo_tone_wb[6],photo_tone_wb_seen,photo_tone_iso_seen;
PROP_HANDLER(PROP_ISO) { (void)buf;if(len==4)photo_tone_iso_seen=1; }
#define TONE_WB_HANDLER(prop,index) \
    PROP_HANDLER(prop) { if(len==4){photo_tone_wb[index]=index>=4?*(int8_t*)buf:*(int*)buf;photo_tone_wb_seen|=1<<(index);} }
TONE_WB_HANDLER(PROP_WB_MODE_PH,0)
TONE_WB_HANDLER(PROP_WB_MODE_LV,1)
TONE_WB_HANDLER(PROP_WB_KELVIN_PH,2)
TONE_WB_HANDLER(PROP_WB_KELVIN_LV,3)
TONE_WB_HANDLER(PROP_WBS_GM,4)
TONE_WB_HANDLER(PROP_WBS_BA,5)
#undef TONE_WB_HANDLER
static int photo_tone_platform_ready(void *ctx) {
    (void)ctx;
    unsigned jpeg=(unsigned)pic_quality>>24;
    #ifdef FEATURE_EXPO_OVERRIDE
    if(CONTROL_BV)return 0; /* Native recipes must not drive an ML exposure override. */
    #endif
    return !RECORDING && !is_movie_mode() && shooting_mode>=0 && shooting_mode<=3 &&
        job_state_ready_to_take_pic() && (jpeg==2 || jpeg==3);
}
static int photo_tone_platform_read(void *ctx,struct photo_tone_state *state) {
    (void)ctx;
    memset(state,0,sizeof(*state));
    state->active=lens_info.raw_picstyle;state->raw_iso=lens_info.raw_iso;
    unsigned styles=0;
    const int ids[]={0x81,0x83,0x86};
    for(int i=0;i<3;i++) {
        if(photo_jpeg_style_read(ids[i],&state->style[i]))styles|=1<<i;
        else state->style[i].sharpness=-10;
    }
    for(int i=0;i<6;i++)state->wb[i]=photo_tone_wb[i];
    snprintf(photo_tone_read_detail,sizeof(photo_tone_read_detail),"WB %02X PS %X M%d/%d K%d/%d ISO %d:%d",photo_tone_wb_seen,styles,state->wb[0],state->wb[1],state->wb[2],state->wb[3],photo_tone_iso_seen,state->raw_iso);
    /* Modes and shifts must be observed. Unused Kelvin and unrelated styles need not be. */
    if((photo_tone_wb_seen & 0x33)!=0x33 || !photo_tone_iso_seen)return 0;
    if(state->raw_iso && (state->raw_iso<72 || state->raw_iso>128))return 0;
    if(!((state->active>=0x81 && state->active<=0x87) || (state->active>=0x21 && state->active<=0x23)))return 0;
    return state->wb[0]>=0 && state->wb[0]<=9 && state->wb[1]>=0 && state->wb[1]<=9 &&
        (state->wb[0]!=9 || ((photo_tone_wb_seen & 4) && photo_tone_kelvin_known(state->wb[2]))) &&
        (state->wb[1]!=9 || ((photo_tone_wb_seen & 8) && photo_tone_kelvin_known(state->wb[3]))) &&
        state->wb[4]>=-9 && state->wb[4]<=9 && state->wb[5]>=-9 && state->wb[5]<=9;
}
static int photo_tone_platform_write(void *ctx,int op,const struct photo_tone_state *state) {
    (void)ctx;
    if(op==13) {
        int value=state->raw_iso;
        prop_request_change_wait(PROP_ISO,&value,4,350);
        return lens_info.raw_iso==value;
    }
    if(op==0 || (op>=10 && op<=12)) {
        const int ids[]={0x81,0x83,0x86};
        int value=op==0?state->active:ids[op-10];
        prop_request_change_wait(PROP_PICTURE_STYLE,&value,4,350);
        return lens_info.raw_picstyle==value;
    }
    if(op>=1 && op<=3) {
        const int ids[]={0x81,0x83,0x86};
        return photo_jpeg_style_write(ids[op-1],&state->style[op-1]);
    }
    if(op>=4 && op<=9) {
        const unsigned props[]={PROP_WB_MODE_PH,PROP_WB_MODE_LV,PROP_WB_KELVIN_PH,PROP_WB_KELVIN_LV,PROP_WBS_GM,PROP_WBS_BA};
        int value=state->wb[op-4];
        prop_request_change_wait(props[op-4],&value,4,350);
        return photo_tone_wb[op-4]==value;
    }
    return 0;
}
#endif
static int photo_tone_ids(int group,int out[PHOTO_TONE_COUNT]) {
    int n=0;for(int i=0;i<PHOTO_TONE_COUNT;i++)if(photo_tone_recipes[i].group==group)out[n++]=i;return n;
}
static int photo_tone_current(void) {
    int ids[PHOTO_TONE_COUNT],n=photo_tone_ids(photo_tone_group,ids);
    photo_tone_selected[photo_tone_group]=MOD(photo_tone_selected[photo_tone_group],n);
    return ids[photo_tone_selected[photo_tone_group]];
}
static void photo_tone_progress(void *ctx,int step,int total) {
    (void)ctx;take_semaphore(menu_sem,0);
    photo_tone_step=step;photo_tone_steps=total;
    give_semaphore(menu_sem);menu_redraw();
}
static const struct photo_tone_io photo_tone_io={NULL,photo_tone_platform_ready,photo_tone_platform_read,photo_tone_platform_write,photo_tone_progress};
static void photo_tone_perform(int action) {
    /* Property callbacks may acquire other locks; never execute with menu_sem held. */
    take_semaphore(menu_sem,0);
    struct photo_tone_session local=photo_tone_session;int index=photo_tone_current();
    give_semaphore(menu_sem);
    int result=action==1?photo_tone_apply(&local,&photo_tone_io,index):photo_tone_restore(&local,&photo_tone_io);
    take_semaphore(menu_sem,0);
    photo_tone_session=local;photo_tone_working=0;photo_tone_confirm=0;
    photo_tone_message=result==1?(action==1?"Tone ready - take a JPEG":"Original settings restored"):
        result==-1?"Failed - changes rolled back":result==-2?"Recovery needed - Q Restore":photo_tone_platform_ready(NULL)?"Settings not ready - retry":"Use P/Tv/Av/M and enable JPEG";
    give_semaphore(menu_sem);menu_redraw();
}
#include "photo-film-art-data.h"
/* Four cached sprites: three shelf positions and the active-film thumbnail.
 * File DMA buffers are temporary; hot pixel reads use cached malloc memory. */
static unsigned char *photo_film_art;
static FILE *photo_film_art_file;
static int photo_film_art_attempted,photo_film_palette_live,photo_film_art_error;
static uint32_t photo_film_palette_probe;
static struct {int id;unsigned used;} photo_film_slots[4];
static unsigned photo_film_art_clock;
static unsigned photo_film_crc(const unsigned char *b,unsigned n) {
    unsigned crc=~0u;while(n--){crc^=*b++;for(int k=0;k<8;k++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}
    return ~crc;
}
static uint32_t photo_film_word(unsigned off) {uint32_t v;memcpy(&v,photo_film_art+off,4);return v;}
static void photo_film_palette_restore(void) {
    if(!photo_film_palette_live || !photo_film_art)return;
    bmp_fill(COLOR_BLACK,0,0,720,480);
    uint32_t *saved=(uint32_t*)(photo_film_art+PHOTO_FILM_ART_META_BYTES);
    for(int i=0;i<160;i++)for(int bank=0;bank<2;bank++)EngDrvOut(LCD_Palette[(i+80)*3+bank*0x300],saved[i*2+bank]);
    photo_film_palette_live=0;
}
static void photo_film_palette_apply(void) {
    if(photo_film_palette_live && shamem_read(LCD_Palette[200*3])==photo_film_palette_probe)return;
    uint32_t *saved=(uint32_t*)(photo_film_art+PHOTO_FILM_ART_META_BYTES);
    uint32_t white=LCD_Palette[3*COLOR_WHITE+2],opaque=white&0xff000000;
    if((opaque>>24)<2)opaque=0x03000000;
    int wy=(white>>16)&255;if(wy<180)wy=235;
    for(int i=0;i<160;i++) {
        if(!photo_film_palette_live)for(int bank=0;bank<2;bank++)saved[i*2+bank]=shamem_read(LCD_Palette[(i+80)*3+bank*0x300]);
        uint32_t c=photo_film_word(16+i*4);int r=(c>>16)&255,g=(c>>8)&255,b=c&255;
        int yf=(299*r+587*g+114*b)/1000,y=yf*wy/255,u=(b-yf)*564/1000*wy/255,v=(r-yf)*713/1000*wy/255;
        if(u<-127)u=-127;
        if(u>127)u=127;
        if(v<-127)v=-127;
        if(v>127)v=127;
        uint32_t entry=opaque|((uint32_t)y<<16)|((uint32_t)(u&255)<<8)|(v&255);
        EngDrvOut(LCD_Palette[(i+80)*3],entry);EngDrvOut(LCD_Palette[(i+80)*3+0x300],entry);
        if(i+80==200)photo_film_palette_probe=entry;
    }
    photo_film_palette_live=1;
}
static int photo_film_art_prepare(void) {
    if(photo_film_art_attempted)return photo_film_art!=NULL;
    photo_film_art_attempted=1;uint32_t size=0;
    if(FIO_GetFileSize("ML/data/FILMART.BIN",&size) || size!=PHOTO_FILM_ART_BYTES)goto fail;
    photo_film_art=malloc(PHOTO_FILM_ART_META_BYTES+1280+4*PHOTO_FILM_ART_CELL_BYTES);
    if(!photo_film_art)goto fail;
    unsigned char *dma=fio_malloc(PHOTO_FILM_ART_META_BYTES);if(!dma)goto bad;
    photo_film_art_file=FIO_OpenFile("ML/data/FILMART.BIN",O_RDONLY|O_SYNC);
    int n=photo_film_art_file?FIO_ReadFile(photo_film_art_file,dma,PHOTO_FILM_ART_META_BYTES):0;
    if(n==PHOTO_FILM_ART_META_BYTES)memcpy(photo_film_art,dma,PHOTO_FILM_ART_META_BYTES);
    fio_free(dma);
    if(n!=PHOTO_FILM_ART_META_BYTES || memcmp(photo_film_art,"FAR7",4) || photo_film_crc(photo_film_art,PHOTO_FILM_ART_META_BYTES)!=PHOTO_FILM_ART_META_CRC)goto bad;
    for(int i=0;i<4;i++){photo_film_slots[i].id=-1;photo_film_slots[i].used=0;}
    return 1;
 bad:
    if(photo_film_art_file)FIO_CloseFile(photo_film_art_file);
    photo_film_art_file=NULL;free(photo_film_art);photo_film_art=NULL;
 fail:
    photo_film_art_error=1;return 0;
}
static const unsigned char *photo_film_sprite(int index) {
    if(index<0 || index>=PHOTO_TONE_COUNT || !photo_film_art_prepare())return NULL;
    int slot=0;
    for(int i=0;i<4;i++) {
        if(photo_film_slots[i].id==index){photo_film_slots[i].used=++photo_film_art_clock;photo_film_palette_apply();return photo_film_art+PHOTO_FILM_ART_META_BYTES+1280+i*PHOTO_FILM_ART_CELL_BYTES;}
        if(photo_film_slots[i].id==-index-2)return NULL;
        if(photo_film_slots[i].used<photo_film_slots[slot].used)slot=i;
    }
    unsigned char *cell=photo_film_art+PHOTO_FILM_ART_META_BYTES+1280+slot*PHOTO_FILM_ART_CELL_BYTES;
    unsigned char *dma=fio_malloc(PHOTO_FILM_ART_CELL_BYTES);if(!dma)return NULL;
    unsigned off=PHOTO_FILM_ART_META_BYTES+index*PHOTO_FILM_ART_CELL_BYTES;
    int n=0;
    if(FIO_SeekSkipFile(photo_film_art_file,off,SEEK_SET)==off)n=FIO_ReadFile(photo_film_art_file,dma,PHOTO_FILM_ART_CELL_BYTES);
    if(n==PHOTO_FILM_ART_CELL_BYTES)memcpy(cell,dma,PHOTO_FILM_ART_CELL_BYTES);
    fio_free(dma);photo_film_slots[slot].used=++photo_film_art_clock;
    photo_film_slots[slot].id=-index-2;
    if(n!=PHOTO_FILM_ART_CELL_BYTES || photo_film_crc(cell,PHOTO_FILM_ART_CELL_BYTES)!=photo_film_word(656+index*4)){photo_film_art_error=1;return NULL;}
    photo_film_slots[slot].id=index;photo_film_palette_apply();return cell;
}
static int photo_tone_anim_step=2;
static void photo_tone_art_release(void) {
    photo_film_palette_restore();
    if(photo_film_art_file)FIO_CloseFile(photo_film_art_file);
    if(photo_film_art)free(photo_film_art);
    photo_film_art_file=NULL;photo_film_art=NULL;photo_film_art_attempted=0;photo_film_art_error=0;
}
static int photo_tone_animation_pending(void) {
    take_semaphore(menu_sem,0);
    int pending=photo_tone_opened && !photo_tone_confirm && !photo_tone_working && photo_tone_anim_step<2;
    give_semaphore(menu_sem);return pending;
}
static int photo_film_scaled(int index,int dx,int dy,int dw,int dh,int clear_bg) {
    const unsigned char *sprite=photo_film_sprite(index);if(!sprite)return 0;
    unsigned short map[262];if(dw<1 || dw>262 || dh<1)return 0;
    for(int x=0;x<dw;x++)map[x]=x*176/dw;
    unsigned char *v=bmp_vram();
    for(int y=0;y<dh;y++) {
        const unsigned char *row=sprite+(y*194/dh)*176;
        for(int x=0;x<dw;x++){int c=row[map[x]];if(c)bmp_putpixel_fast(v,dx+x,dy+y,c);else if(clear_bg>=0)bmp_putpixel_fast(v,dx+x,dy+y,clear_bg);}
    }
    return 1;
}
/* Readable low-memory/missing-art fallback; no palette lease is required. */
static void photo_tone_canister_flat(int x,int index,int selected) {
    (void)selected;const struct photo_tone_recipe *r=&photo_tone_recipes[index];
    bmp_fill(COLOR_GRAY(24),x+54,166,92,151);bmp_draw_rect(COLOR_GRAY(65),x+54,166,92,151);
    bmp_fill(COLOR_BLACK,x+87,146,26,18);
    int w=bmp_string_width(FONT(FONT_MED,COLOR_WHITE,COLOR_GRAY(24)),r->name);
    if(w<=176)bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_GRAY(24)),x+100-w/2,221,"%s",r->name);
}

static int photo_tone_matches(const struct photo_tone_recipe *r) {
    struct photo_tone_state actual;int slot=photo_tone_style_slot(r->style);
    return photo_tone_platform_read(NULL,&actual) && actual.active==r->style && actual.raw_iso==r->raw_iso &&
        actual.style[slot].contrast==r->contrast && actual.style[slot].sharpness==r->sharpness &&
        (r->style==0x86 || (actual.style[slot].saturation==r->saturation && actual.style[slot].color_tone==r->color_tone)) &&
        actual.wb[0]==9 && actual.wb[1]==9 && actual.wb[2]==r->kelvin && actual.wb[3]==r->kelvin && actual.wb[4]==r->gm && actual.wb[5]==r->ba;
}

static int photo_film_inset(int y,int h,int r) {
    int q=y<r?r-y:y>=h-r?y-(h-r-1):0;
    int n=r;while(n>0 && n*n+q*q>r*r)n--;
    return q?r-n:0;
}
static void photo_film_round(int x,int y,int w,int h,int r,int fill,int edge) {
    bmp_fill(COLOR_BLACK,x,y,w,h);
    for(int j=0;j<h;j++) {
        int k=photo_film_inset(j,h,r);
        bmp_fill(edge,x+k,y+j,w-2*k,1);
        if(j>0 && j<h-1 && w-2*k>2)bmp_fill(fill,x+k+1,y+j,w-2*k-2,1);
    }
}
static void photo_film_center_title(int font,int x,int y,int width,const char *text,int fg,int bg) {
    int f=font,w=bmp_string_width(FONT(f,fg,bg),text);
    if(w>width){f=FONT_MED;w=bmp_string_width(FONT(f,fg,bg),text);}
    photo_text(FONT(f,fg,bg),x+(width-w)/2,y,width,text);
}
static void photo_film_card(int x,int index,int chosen,int scale) {
    int bg=chosen?COLOR_GRAY(13):COLOR_GRAY(3);
    photo_film_round(x,102,212,238,14,bg,chosen?COLOR_GRAY(42):COLOR_GRAY(22));
    if(!photo_film_scaled(index,x+100-73*scale/100,334-194*scale/100,176*scale/100,194*scale/100,-1))
        photo_tone_canister_flat(x,index,0);
    photo_film_center_title(FONT_MED_LARGE,x+1,108,210,photo_tone_recipes[index].name,chosen?COLOR_WHITE:COLOR_GRAY(76),bg);
}
#include "photo-film-details-data.h"
static unsigned char *photo_film_info;
static int photo_film_info_attempted;
static int photo_film_info_prepare(void) {
    if(photo_film_info_attempted)return photo_film_info!=NULL;
    photo_film_info_attempted=1;uint32_t size=0;
    if(FIO_GetFileSize("ML/data/TONEINFO.BIN",&size) || size!=PHOTO_FILM_INFO_BYTES || size>256*1024)return 0;
    unsigned char *b=fio_malloc(size);if(!b)return 0;
    FILE *f=FIO_OpenFile("ML/data/TONEINFO.BIN",O_RDONLY|O_SYNC);
    if(!f){fio_free(b);return 0;}
    int n=FIO_ReadFile(f,b,size);FIO_CloseFile(f);
    if(n!=(int)size){fio_free(b);return 0;}
    unsigned crc=~0u;
    for(unsigned i=0;i<size;i++){crc^=b[i];for(int j=0;j<8;j++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}
    if(~crc!=PHOTO_FILM_INFO_CRC || memcmp(b,"FTD6",4)){fio_free(b);return 0;}
    photo_film_info=b;return 1;
}
static int photo_film_info_draw(int index,int x,int y,int color) {
    if(!photo_thai_enabled || !photo_film_info_prepare() || index<0 || index>=PHOTO_FILM_INFO_COUNT)return 0;
    struct {unsigned short w,h;unsigned off,bytes;} r;
    memcpy(&r,photo_film_info+16+index*12,12);
    if(r.w>368 || r.h>80 || x+r.w>720 || y+r.h>480 || r.off>PHOTO_FILM_INFO_BYTES || r.bytes>PHOTO_FILM_INFO_BYTES-r.off)return 0;
    unsigned stride=(r.w+7)/8,total=stride*r.h,out=0,pos=r.off,end=r.off+r.bytes;
    unsigned char *v=bmp_vram();
    while(out<total && pos<end) {
        unsigned tag=photo_film_info[pos++],run=(tag&127)+1;
        if(run>total-out || (!(tag&128) && run>end-pos))return 0;
        if(tag&128){out+=run;continue;}
        while(run--) {
            unsigned bits=photo_film_info[pos++],xx=(out%stride)*8,yy=out/stride;
            for(unsigned k=0;k<8 && xx+k<r.w;k++)if(bits & (128>>k))bmp_putpixel_fast(v,x+xx+k,y+yy,color);
            out++;
        }
    }
    if(out!=total || pos!=end)return 0;
    return 1;
}
static void photo_film_english_desc(int index) {
    char line[64];
    if(photo_film_info_prepare())snprintf(line,sizeof(line),"%.63s",photo_film_info+PHOTO_FILM_EN_OFFSET+index*64);
    else snprintf(line,sizeof(line),"Native JPEG preset");
    int cut=strlen(line);
    while(cut && bmp_string_width(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),line)>368){
        char *p=strrchr(line,' ');if(!p)break;cut=p-line;*p=0;
    }
    photo_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),320,175,368,line);
    if(photo_film_info && cut<(int)strlen((char*)photo_film_info+PHOTO_FILM_EN_OFFSET+index*64))
        photo_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),320,206,368,(char*)photo_film_info+PHOTO_FILM_EN_OFFSET+index*64+cut+1);
}
static void photo_tone_detail(void) {
    int i=photo_tone_current();const struct photo_tone_recipe *r=&photo_tone_recipes[i];
    if(!photo_film_info_draw(75,320,56,COLOR_CYAN))photo_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),320,56,380,"FILM DETAILS");
    photo_film_round(14,94,286,330,18,COLOR_GRAY(3),COLOR_GRAY(20));
    if(!photo_film_scaled(i,22,110,262,289,-1))photo_tone_canister_flat(24,i,0);
    photo_text(FONT(FONT_LARGE,COLOR_WHITE,COLOR_BLACK),320,96,380,r->name);
    photo_text(FONT(FONT_MED,COLOR_YELLOW,COLOR_BLACK),320,132,380,r->brand);
    if(!photo_film_info_draw(i,320,175,COLOR_WHITE))photo_film_english_desc(i);
    bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),320,258,"ISO%d  %dK  %s",r->effective_iso,r->kelvin,r->process);
    bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),320,287,"%s C%+d S%d",r->style==0x86?"Mono":r->style==0x83?"Landscape":"Standard",r->contrast,r->sharpness);
    if(r->style==0x86)bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),320,317,"GM%+d BA%+d Filter/Tone kept",r->gm,r->ba);
    else bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),320,317,"Sat%+d Tone%+d GM%+d BA%+d",r->saturation,r->color_tone,r->gm,r->ba);
    if(photo_tone_working)photo_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),16,442,688,"Setting tone - please wait");
    else if(!photo_film_info_draw(73,16,442,COLOR_CYAN))photo_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),16,442,688,"SET Apply   MENU Back");
}
static void photo_film_pill(const struct photo_tone_recipe *r,int index,int active) {
    photo_text(FONT(FONT_MED,active?COLOR_CYAN:COLOR_WHITE,COLOR_BLACK),420,350,288,active?"Active JPEG tone":"FLIM TONE");
    photo_film_round(422,393,230,43,12,COLOR_GRAY(10),COLOR_GRAY(7));
    photo_film_round(420,390,230,43,12,COLOR_YELLOW,COLOR_ORANGE);
    for(int y=2;y<41;y++) {
        int k=photo_film_inset(y,43,12);
        for(int x=k+1;x<230-k-1;x++)bmp_putpixel_fast(bmp_vram(),420+x,390+y,((x+y)&3)<(y*4/43)?COLOR_ORANGE:COLOR_YELLOW);
    }
    int f=bmp_string_width(FONT(FONT_LARGE,COLOR_BLACK,COLOR_YELLOW),r->name)<=210?FONT_LARGE:FONT_MED;
    int w=bmp_string_width(FONT(f,COLOR_BLACK,COLOR_YELLOW),r->name);
    photo_text(FONT(f,COLOR_BLACK,NO_BG_ERASE),430+(210-w)/2,395,w,r->name);
    photo_film_scaled(index,656,376,50,56,-1);
}

static int photo_tone_draw(void) {
    photo_thai_prepare();
    bmp_fill(COLOR_BLACK,0,0,720,480);bmp_fill(COLOR_GRAY(5),0,0,720,48);
    photo_text(FONT(FONT_LARGE,COLOR_WHITE,COLOR_GRAY(5)),16,2,570,"FLIM TONE");
    if(photo_tone_confirm==1){photo_tone_detail();return 1;}
    for(int g=0;g<4;g++) {
        int bg=g==photo_tone_group?COLOR_BLUE:COLOR_BLACK;
        int x=8+g*158,width=g==3?230:150;
        bmp_fill(bg,x,54,width,40);
        photo_text(FONT(FONT_MED,g==photo_tone_group?COLOR_WHITE:COLOR_CYAN,bg),x+(width-bmp_string_width(FONT(FONT_MED,COLOR_WHITE,bg),photo_tone_groups[g]))/2,60,width-12,photo_tone_groups[g]);
    }
    int ids[PHOTO_TONE_COUNT],n=photo_tone_ids(photo_tone_group,ids),selected=photo_tone_selected[photo_tone_group];
    char count[20];snprintf(count,sizeof(count),"%d/%d",selected+1,n);
    int cw=bmp_string_width(FONT(FONT_MED_LARGE,COLOR_WHITE,COLOR_GRAY(5)),count);
    bmp_printf(FONT(FONT_MED_LARGE,COLOR_WHITE,COLOR_GRAY(5)),704-cw,8,"%s",count);
    int first=selected/3*3;
    for(int i=0;i<3 && first+i<n;i++)photo_film_card(24+i*234,ids[first+i],first+i==selected,first+i==selected?96+photo_tone_anim_step*2:84);
    if(photo_tone_anim_step<2)photo_tone_anim_step++;
    if(selected>0)for(int i=0;i<8;i++)bmp_fill(COLOR_CYAN,5+i,230-i,1,2*i+1);
    if(selected<n-1)for(int i=0;i<8;i++)bmp_fill(COLOR_CYAN,714-i,230-i,1,2*i+1);
    const struct photo_tone_recipe *r=&photo_tone_recipes[photo_tone_current()];
    bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,350,"ISO%d K%d",r->effective_iso,r->kelvin);
    bmp_printf(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,376,"GM%+d BA%+d",r->gm,r->ba);
    const char *message=photo_tone_working?"Setting tone - please wait":photo_tone_confirm==1?"Apply tone? SET confirms":photo_tone_confirm==2?"Restore session settings? SET":photo_tone_message;
    const struct photo_tone_recipe *badge=r;
    int active=0;
    if(!photo_tone_working && photo_tone_session.active>=0) {
        const struct photo_tone_recipe *current=&photo_tone_recipes[photo_tone_session.active];
        if(photo_tone_matches(current)){badge=current;active=1;}
        else if(streq(message,"Tone ready - take a JPEG"))message="Settings changed in Canon";
    }
    if(photo_film_art_error && streq(message,"JPEG / RAW+JPEG in Canon"))message="Copy ML/data/FILMART.BIN";
    photo_text(FONT(FONT_MED_LARGE,COLOR_CYAN,COLOR_BLACK),16,403,392,message);
    photo_film_pill(badge,active?photo_tone_session.active:photo_tone_current(),active);
    if(!photo_tone_working && !photo_tone_confirm && streq(photo_tone_message,"Settings not ready - retry")) {
        char first[33];snprintf(first,sizeof(first),"%.32s",photo_tone_read_detail);
        bmp_fill(COLOR_BLACK,16,347,392,54);
        photo_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,350,392,first);
        photo_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,376,392,strlen(photo_tone_read_detail)>32?photo_tone_read_detail+32:"");
    }
    photo_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),16,442,688,"L/R Tone U/D Group SET Details Q Restore MENU Back");
    return 1;
}
/* Called with menu_sem held. Returns a pending action, dispatched outside it. */
static int photo_tone_key(int key) {
    if(photo_tone_working)return 0;
    if(key==BGMT_MENU){if(photo_tone_confirm)photo_tone_confirm=0;else {photo_tone_opened=0;photo_tone_art_release();}return 0;}
    if(photo_tone_confirm==1 && key!=BGMT_PRESS_SET)return 0;
    if(key==BGMT_PRESS_LEFT || key==BGMT_WHEEL_LEFT || key==BGMT_PRESS_RIGHT || key==BGMT_WHEEL_RIGHT) {
        int ids[PHOTO_TONE_COUNT],n=photo_tone_ids(photo_tone_group,ids);
        int d=key==BGMT_PRESS_LEFT || key==BGMT_WHEEL_LEFT?-1:1;
        int next=photo_tone_selected[photo_tone_group]+d;
        photo_tone_selected[photo_tone_group]=next<0?0:next>=n?n-1:next;photo_tone_confirm=0;photo_tone_anim_step=0;return 0;
    }
    if(key==BGMT_PRESS_UP || key==BGMT_WHEEL_UP || key==BGMT_PRESS_DOWN || key==BGMT_WHEEL_DOWN) {
        photo_tone_group=MOD(photo_tone_group+(key==BGMT_PRESS_UP || key==BGMT_WHEEL_UP?-1:1),4);photo_tone_confirm=0;photo_tone_anim_step=0;return 0;
    }
    if(key==BGMT_Q){photo_tone_confirm=photo_tone_session.saved?2:0;photo_tone_message="No session backup";return 0;}
    if(key==BGMT_PRESS_SET) {
        if(!photo_tone_confirm){photo_tone_confirm=1;return 0;}
        int action=photo_tone_confirm;photo_tone_confirm=action==1?1:0;photo_tone_anim_step=2;photo_tone_working=1;photo_tone_step=0;photo_tone_steps=9;return action;
    }
    return 0;
}
