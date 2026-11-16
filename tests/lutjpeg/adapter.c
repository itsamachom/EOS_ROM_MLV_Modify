#define LUTJPEG_HOST_TEST
#include "../../firmware/modules/lutjpeg/lutjpeg.c"
static int cancel_on_read,fail_writes;
static int FIO_ReadFile(FILE *f,void *p,int n){if(cancel_on_read)cancel=1;assert(test_is_dma(p,n) && n<=8192);return fread(p,1,n,f);}
static int FIO_WriteFile(FILE *f,const void *p,int n){assert(test_is_dma(p,n) && n<=8192);return fail_writes?-1:(int)fwrite(p,1,n,f);}
static unsigned file_hash(const char *path){FILE *f=fopen(path,"rb");assert(f);unsigned h=~0u;unsigned char b[8192];int n;while((n=fread(b,1,sizeof(b),f)))h=film_crc(h,b,n);fclose(f);return ~h;}
static void drain(void){service_tick();mock_now+=600;service_tick();}
int main(void){
 assert(lutjpeg_bootstrap()==0 && registered && auto_enabled && qcount==0 && COUNT(films)==7);
 open_film_picker(NULL,0);assert(ui_mode==1 && test_film_mode==1);
 int selected_before=within;film_ui_key(BGMT_PRESS_RIGHT);assert(within==selected_before && browsing==1);
 film_ui_key(BGMT_PRESS_SET);assert(within==1 && !ui_mode && !test_film_mode && pending_preview);
 within=0;open_film_picker(NULL,0);film_ui_key(BGMT_PRESS_LEFT);assert(browsing==6);film_ui_key(BGMT_MENU);assert(within==0);
 pic_quality=0x3010000;quality_requested=0;service_tick();assert(quality_requests==1 && pic_quality==PICQ_RAW_JPG_LARGE_FINE);
 assert(live_ready && live_table);
 uint32_t raw_pixels[8]={0x80008000,0x7f407f40,0x00ff00ff,0x00800080,0x40402040,0xa060d060,0x10601060,0xf0e0f0e0},original_pixels[8],filtered[8];memcpy(original_pixels,raw_pixels,sizeof(raw_pixels));
 unsigned previous=0;
 for(int i=0;i<7;i++){within=i;live_ready=0;pending_preview=1;load_live_preview();assert(live_ready);film_lv_filter(raw_pixels,filtered,8);assert(!memcmp(original_pixels,raw_pixels,sizeof(raw_pixels)));unsigned checksum=~film_crc(~0u,(unsigned char*)filtered,sizeof(filtered));if(i)assert(checksum!=previous);previous=checksum;}
 output_size=FOS_SIZE_S;
 for(int i=0;i<7;i++){
  within=i;start_convert(NULL,0);drain();assert(!running && percent==100 && completed==i+1);
  assert(jpeg_valid(output) && jpeg_valid("A:/DCIM/100CANON/IMG_0001.JPG"));
  assert(!strcmp(last_saved,"A:/DCIM/100CANON/IMG_0001.JPG") && review_pixels && is_file(last_preview));
  assert(strstr(status,"DCIM"));
 }
 /* A four-letter temporary extension is rejected by the modeled Canon FIO. */
 assert(!FIO_CreateFile("A:/ML/LUTOUT/BAD.PART"));
 within=0;fail_optional_print=1;start_convert(NULL,0);drain();fail_optional_print=0;
 assert(completed==8 && jpeg_valid(output)); /* The camera-reported error cannot discard JPEG. */
 fail_preview_file=1;start_convert(NULL,0);drain();fail_preview_file=0;
 assert(completed==9 && jpeg_valid(output) && review_pixels); /* In-memory review survives optional file failure. */
 cancel_on_read=1;start_convert(NULL,0);drain();cancel_on_read=0;
 assert(!running && !percent);
 fail_writes=1;start_convert(NULL,0);drain();fail_writes=0;assert(!running && !percent);
 fail_rename=1;start_convert(NULL,0);drain();fail_rename=0;assert(!running && !percent);
 assert(is_file("A:/DCIM/100CANON/IMG_0001.CR2"));
 /* DCIM promotion failure leaves the previous Canon JPEG and the film copy. */
 assert(jpeg_valid("A:/DCIM/100CANON/IMG_0001.JPG"));
 unsigned before=file_hash("A:/DCIM/100CANON/IMG_0001.JPG");
 fail_companion_rename=1;within=2;start_convert(NULL,0);drain();fail_companion_rename=0;
 assert(jpeg_valid(output) && file_hash("A:/DCIM/100CANON/IMG_0001.JPG")==before && strstr(status,"DCIM update failed"));
 /* Last result and its palette preview reopen after the RAM cache is released. */
 within=1;start_convert(NULL,0);drain();assert(completed==11);
 char saved[128];snprintf(saved,sizeof(saved),"%s",last_saved);
 free(review_pixels);review_pixels=NULL;last_saved[0]=last_preview[0]=last_source[0]=0;last_record(0);
 assert(!strcmp(last_saved,saved) && load_review() && review_pixels);
 /* A missing new preview must never display the previous photo with a new filename. */
 assert(!store_review(NULL,NULL,"unused.FPV") && !review_pixels && !last_preview[0]);
 ui_mode=0;pending_view=0;assert(film_ui_key(BGMT_PLAY)==1 && !pending_view);
 last_record(0);assert(load_review());
 /* No old image on boot; a fresh capture is queued with a film+size snapshot. */
 assert(qcount==0);observe_shot();assert(qcount==0);card.file_number=2;observe_shot();assert(qcount==1);
 int captured=queue[qhead].film;within=6;assert(queue[qhead].film==captured);cancel_convert(NULL,0);assert(!qcount);
 observed_number=0;observed_busy=0;card.file_number=1;observe_shot();assert(qcount==1);cancel_convert(NULL,0);
 card.file_number=2;observe_shot();cancel_convert(NULL,0);card.file_number=5;observe_shot();assert(qcount==3);
 for(int i=0;i<QUEUE_SIZE;i++){struct film_job j={.film=0};enqueue(&j);}assert(qcount==QUEUE_SIZE && dropped==3);cancel_convert(NULL,0);
 within=4;auto_enabled=0;output_size=FOS_SIZE_L;film_settings(1);within=0;auto_enabled=1;output_size=0;film_settings(0);assert(within==4 && !auto_enabled && output_size==FOS_SIZE_L);
 auto_enabled=0;card.file_number=7;observe_shot();assert(!qcount);
 lutjpeg_shutdown();assert(!registered && !running);
 for(int i=0;i<32;i++)assert(!dma_allocs[i].p);
 puts("PASS seven-film automatic adapter: Canon 8.3 fault reproduction, JPEG kept on optional print failure, DCIM companion, RAW kept, persistent on-camera review/palette, counts, queue, DMA, cancel and I/O faults");
}
