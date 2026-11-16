#define main previous_photo_menu_tests
#include "../photo_menu_test.c"
#undef main
#define BMP_LOCK(body) do {body;}while(0)
#include "../../firmware/src/offline-core-600d.h"
#include "../../firmware/modules/lutjpeg/film-colour.h"
#include "../../firmware/modules/lutjpeg/trial-art.h"
static const int offline_art[]={0,1,2,4,5};
static const char *offline_names[]={"Velvia","Astia","Classic Neg","Gold 200","Ektachrome"};
static int mode=1,stopping,film,job_film,total=4,done=2,failed,warnings,percent,raw_count=9,cursor=2,busy;
static int marks[128]={1,0,1};
static char raw_files[128][128],status[96]="Choose film",last_file[128]="A:/ML/FILMOUT/IMG_0001_ClassicNeg.JPG";
static unsigned char *art;
static int selected_count(void){return 2;}
#include "../../firmware/modules/lutjpeg/offline-screen.h"
int main(void){
 const char *path=getenv("PHOTO_TRIAL_ART");assert(path);FILE *f=fopen(path,"rb");assert(f);art=malloc(FILM7_BYTES);assert(fread(art,1,FILM7_BYTES,f)==FILM7_BYTES);fclose(f);
 for(unsigned i=FILM7_META_BYTES;i<FILM7_BYTES;i++){
  unsigned index=art[i];uint32_t rgb=0;if(index>=80 && index<240)memcpy(&rgb,art+16+(index-80)*4,4);
  unsigned luma=(((rgb>>16)&255)*299+((rgb>>8)&255)*587+(rgb&255)*114)/1000;art[i]=index?COLOR_GRAY(luma*100/255):COLOR_BLACK;
 }
 for(int i=0;i<9;i++)snprintf(raw_files[i],128,"A:/DCIM/100CANON/IMG_%04d.CR2",i+1);
 proof_measure_fonts=1;
 for(int lang=0;lang<2;lang++){
  reset();photo_thai_enabled=lang;photo_thai_prepare();
  for(int i=0;i<5;i++){mode=1;film=i;assert(offline_draw());char n[64];snprintf(n,64,"offline-picker-%s-%d",lang?"th":"en",i);save_proof(n);}
  mode=2;offline_draw();save_proof(lang?"offline-files-th":"offline-files-en");
  for(int p=0;p<=100;p+=25){mode=3;job_film=2;percent=p;snprintf(status,sizeof(status),"IMG_0003.CR2");offline_draw();char n[64];snprintf(n,64,"offline-progress-%s-%03d",lang?"th":"en",p);save_proof(n);}
 }
 free(art);puts("PASS offline production screens: five film cards, multi-selection, vertical 0..100 progress, Thai/English framebuffer bounds; no palette register writes");
}
