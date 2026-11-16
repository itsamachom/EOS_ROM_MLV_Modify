#define main existing_photo_menu_tests
#include "../photo_menu_test.c"
#undef main
#undef COUNT
#include "../../firmware/src/vram.h"
int lv;
struct vram_info vram_lv={0,720,1440,480};
static int core_menu,filter_calls,draw_calls;
void gui_open_menu(void){core_menu=1;}
void gui_stop_menu(void){core_menu=0;}
int gui_menu_shown(void){return core_menu;}
#define BMP_LOCK(body) do {body;}while(0)
static uint32_t filter_source[720*480/2],filter_destination[720*480/2];
void display_filter_get_buffers(uint32_t **src,uint32_t **dst){*src=filter_source;*dst=filter_destination;}
#include "../../firmware/src/film-core-600d.h"
static int mock_key(int key){return key==BGMT_PRESS_SET?0:1;}
static int mock_draw(int mode){draw_calls++;return mode>0;}
static int mock_ready(void){return 1;}
static void mock_filter(const uint32_t *src,uint32_t *dst,unsigned n){assert(n==720*480/2);for(unsigned i=0;i<n;i++)dst[i]=src[i]^0x00ff00ff;filter_calls++;}
#include "../../firmware/modules/lutjpeg/film-colour.h"
#include "../../firmware/modules/lutjpeg/seven-films.h"
#include "../../firmware/modules/lutjpeg/trial-art.h"
static unsigned char *trial_art;
static unsigned char trial_palette[480];
#include "../../firmware/modules/lutjpeg/film-picker.h"
int main(void){
 struct film_hooks hooks={mock_key,mock_draw,mock_ready,mock_filter};photo_film_register(&hooks);
 assert(!photo_film_filter_active());lv=1;assert(photo_film_filter_active());
 assert(photo_film_filter_update() && filter_calls==1 && filter_destination[0]==0x00ff00ff);
 recording=1;assert(!photo_film_filter_update());recording=0;vram_lv.pitch=1424;assert(!photo_film_filter_active());vram_lv.pitch=1440;
 photo_film_mode(1);assert(core_menu && photo_film_draw() && draw_calls==1 && !photo_film_filter_active());assert(photo_film_key(BGMT_PRESS_SET)==0);
 unsigned char *pixels=malloc(720*480);memset(pixels,80,720*480);unsigned char palette[480];memset(palette,100,sizeof(palette));
 init_palette();uint32_t before[512];memcpy(before,palette_registers,sizeof(before));
 photo_film_review_pixels(pixels,palette);assert(film_palette_live && film_palette_count==144);
 photo_film_palette_release();assert(!memcmp(before,palette_registers,sizeof(before)));
 photo_film_register(NULL);assert(!photo_film_draw() && !photo_film_filter_update());free(pixels);
 const char *art_path=getenv("PHOTO_TRIAL_ART");
 if(art_path){
  FILE *f=fopen(art_path,"rb");assert(f);trial_art=malloc(FILM7_BYTES);assert(fread(trial_art,1,FILM7_BYTES,f)==FILM7_BYTES);fclose(f);
  assert(~film_crc(~0u,trial_art,FILM7_META_BYTES)==FILM7_META_CRC && films[0]==8 && film_notes[0]);
  for(int i=0;i<160;i++){uint32_t rgb;memcpy(&rgb,trial_art+16+i*4,4);trial_palette[3*i]=rgb>>16;trial_palette[3*i+1]=rgb>>8;trial_palette[3*i+2]=rgb;}
  proof_measure_fonts=1;
  for(int lang=0;lang<2;lang++)for(int i=0;i<7;i++){
   photo_film_palette_release();reset();photo_thai_enabled=lang;photo_thai_prepare();
   film_picker_draw(i,i);char name[64];snprintf(name,sizeof(name),"trial-picker-%s-%d",lang?"th":"en",i);save_proof(name);
  }
  photo_film_palette_release();free(trial_art);trial_art=NULL;
 }
 puts("PASS core film hooks: safe LV geometry/recording/menu guards, key/draw dispatch, review bounds and palette restore");
}
