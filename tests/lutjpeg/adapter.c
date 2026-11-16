#define LUTJPEG_HOST_TEST
#include "../../firmware/modules/lutjpeg/lutjpeg.c"
static int cancel_on_read,fail_writes;
static int FIO_ReadFile(FILE *f,void *p,int n){if(cancel_on_read)cancel=1;assert((p==bounce || n==24) && n<=8192);return fread(p,1,n,f);}
static int FIO_WriteFile(FILE *f,const void *p,int n){assert((p==bounce || n==24) && n<=8192);return fail_writes?-1:(int)fwrite(p,1,n,f);}
static void drain(void){service_tick();mock_now+=600;service_tick();}
int main(void){
 assert(lutjpeg_bootstrap()==0 && registered && auto_enabled && qcount==0);
 pic_quality=0x3010000;quality_requested=0;service_tick();
 assert(quality_requests==1 && pic_quality==PICQ_RAW_JPG_LARGE_FINE);service_tick();assert(quality_requests==1);
 auto_enabled=0;pic_quality=0x3010000;quality_requested=0;service_tick();assert(quality_requests==1);auto_enabled=1;pic_quality=PICQ_RAW_JPG_LARGE_FINE;
 output_size=FOS_SIZE_S; /* Most fault checks use the fast output; large tested separately. */
 collection=9;
 for(int i=0;i<3;i++){
  within=i;start_convert(NULL,0);drain();assert(!running && percent==100);
  assert(is_file(output) && strstr(status,"saved"));
 }
 within=0;start_convert(NULL,0);drain();assert(strstr(output,"LUT_0002.JPG") && is_file(output));
 cancel_on_read=1;start_convert(NULL,0);drain();cancel_on_read=0;
 assert(!running && !percent && !is_file("A:/ML/LUTOUT/L000/LUT_0003.JPG"));
 fail_writes=1;start_convert(NULL,0);drain();fail_writes=0;
 assert(!running && !percent && !is_file("A:/ML/LUTOUT/L000/LUT_0003.JPG"));
 assert(!is_file("A:/ML/LUTOUT/L000/LUT_0003.TMP"));
 fail_rename=1;start_convert(NULL,0);drain();fail_rename=0;
 assert(!running && !percent && !is_file("A:/ML/LUTOUT/L000/LUT_0003.JPG"));
 assert(is_file("A:/DCIM/100CANON/IMG_0001.CR2"));
 /* Startup never reprocesses the existing file; a new capture queues itself. */
 assert(qcount==0);observe_shot();assert(qcount==0);
 card.file_number=2;observe_shot();assert(qcount==1);
 int captured=queue[qhead].film;within=2;assert(queue[qhead].film==captured);
 cancel_convert(NULL,0);assert(!qcount);
 /* A fresh card's first shot must not be mistaken for boot metadata. */
 observed_number=0;observed_busy=0;card.file_number=1;observe_shot();assert(qcount==1);cancel_convert(NULL,0);
 card.file_number=2;observe_shot();cancel_convert(NULL,0);
 /* Burst numbering, queue bound, and explicit overflow count. */
 card.file_number=5;observe_shot();assert(qcount==3);
 for(int i=0;i<QUEUE_SIZE;i++){struct film_job j={.film=0};enqueue(&j);}
 assert(qcount==QUEUE_SIZE && dropped==3);
 cancel_convert(NULL,0);
 /* Settings round trip and corrupt settings fallback preserve defaults. */
 collection=2;within=4;auto_enabled=0;output_size=FOS_SIZE_L;film_settings(1);
 collection=within=0;auto_enabled=1;output_size=0;film_settings(0);
 assert(collection==2 && within==4 && !auto_enabled && output_size==FOS_SIZE_L);
 collection=0;within=9;auto_enabled=1;
 /* Stable new RAW automatically converts, with no call to start_convert. */
 card.file_number=6;FILE *src=fopen("A:/DCIM/100CANON/IMG_0001.CR2","rb"),*dst=fopen("A:/DCIM/100CANON/IMG_0006.CR2","wb");assert(src && dst);
 char buf[8192];int n;while((n=fread(buf,1,sizeof(buf),src)))assert(fwrite(buf,1,n,dst)==(size_t)n);fclose(src);fclose(dst);
 output_size=FOS_SIZE_S;service_tick();assert(qcount==1);mock_now+=600;service_tick();assert(!qcount && percent==100 && !running);
 /* Queue disabled means a capture is observed but not queued. */
 auto_enabled=0;card.file_number=7;observe_shot();assert(!qcount);
 lutjpeg_shutdown();assert(!registered && !running);
 puts("PASS actual automatic adapter: baseline, new-shot detection, stable RAW, film snapshot, burst queue, overflow, cancellation, settings, output collisions, DMA and I/O failure cleanup");
 return 0;
}
