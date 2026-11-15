#define LUTJPEG_HOST_TEST
#include "../../firmware/modules/lutjpeg/lutjpeg.c"
static int cancel_on_read,fail_writes;
static int FIO_ReadFile(FILE *f,void *p,int n){if(cancel_on_read)cancel=1;assert(p==bounce && n<=8192);return fread(p,1,n,f);}
static int FIO_WriteFile(FILE *f,const void *p,int n){assert(p==bounce && n<=8192);return fail_writes?-1:(int)fwrite(p,1,n,f);}
int main(void){
 for(int i=0;i<3;i++){
  selected=i;start_convert(NULL,0);assert(!running && percent==100);
  char expected[128];snprintf(expected,sizeof(expected),"A:/ML/LUTOUT/%s/LUT_0001.JPG",dirs[i]);
  assert(is_file(expected) && strstr(status,"ready"));
 }
 selected=0;start_convert(NULL,0);assert(strstr(output,"LUT_0002.JPG") && is_file(output));
 cancel_on_read=1;selected=0;start_convert(NULL,0);cancel_on_read=0;
 assert(!running && !percent && !is_file("A:/ML/LUTOUT/P400/LUT_0003.JPG"));
 fail_writes=1;start_convert(NULL,0);fail_writes=0;
 assert(!running && !percent && !is_file("A:/ML/LUTOUT/P400/LUT_0003.JPG"));
 assert(!is_file("A:/ML/LUTOUT/P400/LUT_0003.TMP"));
 fail_rename=1;start_convert(NULL,0);fail_rename=0;
 assert(!running && !percent && !is_file("A:/ML/LUTOUT/P400/LUT_0003.JPG"));
 assert(is_file("A:/DCIM/100CANON/IMG_0001.CR2"));
 /* Missing RAW must produce no conversion, no output and no running job. */
 card.file_number=2;start_convert(NULL,0);assert(!running && !strstr(input,"0001"));
 puts("PASS actual adapter: DMA bounce, selected LUT, distinct outputs, collision avoidance, missing RAW");
 return 0;
}
