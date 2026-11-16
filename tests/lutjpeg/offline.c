#include "camera_fixture.h"
#include <dirent.h>
#include "../../firmware/platform/600D.102/gui.h"
#include "../../firmware/src/offline-bridge.h"
#define OFFLINE_TEST_FS 1
#define ATTR_DIRECTORY 16
#define IS_ERROR(p) (!(p))
struct fio_file {char name[128];int mode;};
struct file_info {char name[128];int mode;};
struct fio_dirent {DIR *dir;char path[128];};
static int open_dirs;
static struct fio_file *alloc_fio_file(void){return malloc(sizeof(struct fio_file));}
static int FIO_FindNextEx(struct fio_dirent *d,struct fio_file *f){
 struct dirent *e=readdir(d->dir);if(!e)return -1;
 snprintf(f->name,128,"%s",e->d_name);char p[300];snprintf(p,sizeof(p),"%s/%s",d->path,e->d_name);struct stat s;
 f->mode=!stat(p,&s) && S_ISDIR(s.st_mode)?ATTR_DIRECTORY:0;return 0;
}
static struct fio_dirent *FIO_FindFirstEx(const char *p,struct fio_file *f){
 struct fio_dirent *d=malloc(sizeof(*d));d->dir=opendir(p);if(!d->dir){free(d);return NULL;}
 snprintf(d->path,128,"%s",p);open_dirs++;
 if(FIO_FindNextEx(d,f)){closedir(d->dir);free(d);open_dirs--;return NULL;}return d;
}
static void FIO_FindClose(struct fio_dirent *d){closedir(d->dir);free(d);open_dirs--;}
static struct file_info convert_fio_file_info(struct fio_file *f){struct file_info r;snprintf(r.name,128,"%s",f->name);r.mode=f->mode;return r;}
static int gate,hook_registered,fail_writes,cancel_on_read;
int photo_offline_allowed(void){return gate;}
void photo_offline_register(const struct offline_hooks *h){hook_registered=!!h;}
#define LUTJPEG_HOST_TEST
#include "../../firmware/modules/lutjpeg/lutjpeg.c"
static int FIO_ReadFile(FILE *f,void *p,int n){assert(test_is_dma(p,n) && n<=8192);if(cancel_on_read)cancel=1;return fread(p,1,n,f);}
static int FIO_WriteFile(FILE *f,const void *p,int n){assert(test_is_dma(p,n) && n<=8192);return fail_writes?-1:(int)fwrite(p,1,n,f);}
static unsigned hash(const char *p){FILE *f=fopen(p,"rb");assert(f);unsigned n,h=~0u;unsigned char b[8192];while((n=fread(b,1,sizeof(b),f)))h=film_crc(h,b,n);fclose(f);return ~h;}
int main(void){
 unsigned original=hash("A:/DCIM/100CANON/IMG_0001.CR2");
 unsigned old_jpeg=hash("A:/DCIM/100CANON/IMG_0001.JPG");
 assert(lutjpeg_bootstrap()!=0 && !registered && !mode && !busy);
 gate=1;assert(!lutjpeg_bootstrap() && registered && mode==1 && !busy && !raw_count && art);
 assert(!quality_requests && hook_registered);
 offline_key(BGMT_PRESS_SET);assert(raw_count==5 && !busy && mode==2 && !open_dirs);
 for(int i=0;i<5;i++)assert(strstr(raw_files[i],".CR2"));
 allow_long_names=1;
 for(int i=0;i<5;i++){
  film=i;memset(marks,0,sizeof(marks));marks[0]=1;start_batch();
  assert(!busy && total==1 && done==1 && !failed && !warnings && strstr(last_file,offline_tags[i]));
  bounce=fio_malloc(8192);assert(jpeg_ok(last_file));fio_free(bounce);bounce=NULL;
 }
 mode=2;memset(marks,0,sizeof(marks));
 for(int i=0;i<5;i++){cursor=i;offline_key(BGMT_PRESS_SET);}
 assert(selected_count()==4 && !marks[4]);offline_key(BGMT_Q);assert(total==4 && done==4 && !failed);
 allow_long_names=0;start_batch();assert(done==4 && !failed && warnings==4 && strstr(last_file,"/DCIM/900FILM_"));
 fail_writes=1;start_batch();fail_writes=0;assert(!done && failed==4);
 fail_rename=1;start_batch();fail_rename=0;assert(!done && failed==4);
 cancel_on_read=1;start_batch();cancel_on_read=0;assert(cancel && !done && !failed);
 assert(hash("A:/DCIM/100CANON/IMG_0001.JPG")==old_jpeg);
 assert(hash("A:/DCIM/100CANON/IMG_0001.CR2")==original && !quality_requests);
 mode=3;assert(offline_key(BGMT_PLAY)==0 && mode==0);assert(offline_key(BGMT_PLAY)==-1);
 lutjpeg_shutdown();assert(!hook_registered && !art && !busy && !registered);
 for(int i=0;i<32;i++)assert(!dma_allocs[i].p);
 puts("PASS offline converter: manual initialization only; actual DCIM enumeration/close/sort, five films, selection limit 4, batch counts, JPEG/DCF/named copies, 8.3 fallback, original RAW kept, write failures, Play passthrough and zero capture-property changes");
}
