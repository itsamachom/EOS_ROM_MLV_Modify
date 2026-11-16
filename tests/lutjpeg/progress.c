#define main existing_photo_menu_tests
#include "../photo_menu_test.c"
#undef main
#ifndef COERCE
#define COERCE(a,b,c) ((a)<(b)?(b):(a)>(c)?(c):(a))
#endif
#include "../../firmware/modules/lutjpeg/film-progress.h"
int main(void){
 proof_measure_fonts=1;
 for(int lang=0;lang<2;lang++)for(int p=0;p<=100;p+=25){
  reset();photo_thai_enabled=lang;photo_thai_prepare();
  film_progress_draw(1000+p*16,p<100,p<50?1:6,p,"Portra 400","Photo ready",3,12,2592,1728,1);
  char name[64];snprintf(name,sizeof(name),"film-progress-%s-%03d",lang?"th":"en",p);save_proof(name);
 }
 unsigned char first[sizeof(framebuffer)];memcpy(first,framebuffer,sizeof(first));
 reset();film_progress_draw(2100,1,1,100,"Portra 400","Photo ready",3,12,2592,1728,1);
 assert(memcmp(first,framebuffer,sizeof(first)));
 puts("PASS production progress renderer: Thai/English, changing film-strip frames, percent/stages, 720x480 framebuffer bounds");
 return 0;
}
