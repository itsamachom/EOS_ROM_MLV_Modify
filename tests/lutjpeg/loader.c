/* Actual direct-image loader with fake execution, real files and DMA assertions. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define CONFIG_600D 1
#define PHOTO_LUT_LOADER_TEST 1
#define O_RDONLY 0
#define O_SYNC 0
static int boots,stops,abi_bad,boot_bad,cache_sync,fail_alloc,fail_dma,short_read,handles,live_images,fail_guard;
static unsigned char *dma_ptr;
static int FIO_GetFileSize(const char *p,uint32_t *n){struct stat st;if(stat(p,&st))return -1;*n=st.st_size;return 0;}
static FILE *FIO_OpenFile(const char *p,int flags){(void)flags;FILE *f=fopen(p,"rb");if(f)handles++;return f;}
static FILE *FIO_CreateFileOrAppend(const char *p){FILE *f=fopen(p,"ab");if(f)handles++;return f;}
static FILE *FIO_CreateFile(const char *p){if(fail_guard)return NULL;FILE *f=fopen(p,"wb");if(f)handles++;return f;}
static int FIO_RemoveFile(const char *p){return remove(p);}
static int FIO_ReadFile(FILE *f,void *p,unsigned n){assert(p==dma_ptr && n<=8192);return fread(p,1,short_read && n>1?n-1:n,f);}
static int FIO_WriteFile(FILE *f,const void *p,unsigned n){return fwrite(p,1,n,f);}
static void FIO_CloseFile(FILE *f){handles--;fclose(f);}
static void *fio_malloc(unsigned n){assert(n==8192);return fail_dma?NULL:(dma_ptr=malloc(n));}
static void fio_free(void *p){assert(p==dma_ptr);free(p);dma_ptr=NULL;}
static void *image_alloc(unsigned n){if(fail_alloc)return NULL;void *p=malloc(n);if(p)live_images++;return p;}
static void image_free(void *p){assert(live_images>0);live_images--;free(p);}
static void sync_caches(void){cache_sync++;}
static void msleep(int n){(void)n;}
static void menu_redraw(void){}
static void task_create(const char *s,int p,int z,void (*fn)(void*),void *a){(void)s;(void)p;(void)z;fn(a);}
#define NotifyBox(...) ((void)0)
#define MENU_SELECT_FUNC(n) void n(void *priv,int delta)
#define MENU_UPDATE_FUNC(n) void n(void)
#define MENU_SET_VALUE(...) ((void)0)
#define MENU_SET_HELP(...) ((void)0)
static int fake_call(unsigned char *base,unsigned off){
 uint32_t guard_size;assert(!FIO_GetFileSize("ML/LUTDATA/FILMBOOT.LCK",&guard_size));
 uint32_t ptr;memcpy(&ptr,base,4);assert(ptr==(uint32_t)(uintptr_t)base+16);
 if(off==12)return abi_bad?0:0x4c554933;
 if(off==4){boots++;return boot_bad?-1:0;}
 assert(off==8);stops++;return 0;
}
#define PHOTO_LUT_CALL(base,off) fake_call(base,off)
#define malloc image_alloc
#define free image_free
#include "lutjpeg-loader-600d.h"
#undef malloc
#undef free
static uint32_t crc(const void *p,unsigned n){return ~photo_lut_crc(~0u,p,n);}
static unsigned char blob[64+64+8];static uint32_t *h;
static void write_file(const char *p,const void *b,unsigned n){FILE *f=fopen(p,"wb");assert(f);assert(fwrite(b,1,n,f)==n);fclose(f);}
static void prepare(void){
 if(photo_lut_handle){image_free(photo_lut_handle);photo_lut_handle=NULL;}
 assert(!live_images && !handles && !dma_ptr);
 boots=stops=abi_bad=boot_bad=cache_sync=fail_alloc=fail_dma=short_read=fail_guard=0;photo_lut_loading=0;
 remove("ML/LUTDATA/FILMBOOT.LCK");
 memset(blob,0,sizeof(blob));h=(uint32_t*)blob;
 unsigned char core[64]={1,2,3},sym[64]={4,5,6};
 write_file("autoexec.bin",core,64);write_file("ML/modules/600D_102.sym",sym,64);
 h[0]=0x3354554c;h[1]=1;h[2]=0x600d0102;h[3]=64;h[4]=128;h[5]=1;h[6]=4;h[7]=8;h[8]=12;
 h[11]=crc(sym,64);h[12]=PHOTO_LUT_ABI;h[13]=crc(core,64);h[14]=64;
 uint32_t ptr=16;memcpy(blob+64,&ptr,4);
}
static void save(void){h[9]=crc(blob+64,h[3]);h[10]=crc(blob+128,h[5]*4);h[15]=crc(blob,60);write_file("ML/LUTDATA/LUTJPEG.BIN",blob,128+h[5]*4);}
static void test_load(void){photo_lut_loading=1;photo_lut_load_task(NULL);}
static void fail_clean(void){test_load();assert(!photo_lut_handle && !live_images && !dma_ptr && !handles && !photo_lut_loading);}
int main(void){
 prepare();save();
 /* Test actual public entry points with and without stale crash markers. */
 for(int stale=0;stale<2;stale++){
  if(stale){write_file("ML/LUTDATA/FILMERR.LCK","",0);write_file("ML/LUTDATA/FILMBOOT.LCK","",0);}
  photo_lut_auto_start();
  assert(!boots && !cache_sync && !photo_lut_handle && !handles && !dma_ptr && !photo_lut_loading);
  assert(strstr(photo_lut_status,"RAW to Film"));
 }
 remove("ML/LUTDATA/FILMERR.LCK");
 prepare();save();photo_lut_load_select(NULL,0);assert(boots==1 && photo_lut_handle);photo_lut_shutdown();
 prepare();save();fail_guard=1;fail_clean();assert(!boots && !cache_sync && strstr(photo_lut_status,"guard"));
 prepare();remove("ML/LUTDATA/LUTJPEG.BIN");fail_clean();assert(strstr(photo_lut_status,"Missing"));
 prepare();save();remove("ML/modules/600D_102.sym");fail_clean();assert(strstr(photo_lut_status,"differ"));
 prepare();save();write_file("autoexec.bin","wrong",5);fail_clean();assert(!boots);
 prepare();save();blob[24]^=1;write_file("ML/LUTDATA/LUTJPEG.BIN",blob,132);fail_clean();assert(!cache_sync);
 prepare();save();blob[65]^=1;write_file("ML/LUTDATA/LUTJPEG.BIN",blob,132);fail_clean();assert(!boots);
 prepare();save();blob[128]=4;write_file("ML/LUTDATA/LUTJPEG.BIN",blob,132);fail_clean();
 prepare();h[5]=2;save();fail_clean(); /* duplicate relocation */
 prepare();blob[128]=64;save();fail_clean(); /* relocation beyond image */
 prepare();h[6]=64;save();fail_clean(); /* bad entrypoint */
 prepare();blob[65]=16;save();fail_clean(); /* relocatable value outside memory */
 prepare();save();fail_alloc=1;fail_clean();assert(strstr(photo_lut_status,"memory"));
 prepare();save();fail_dma=1;fail_clean();
 prepare();save();short_read=1;fail_clean();
 prepare();save();abi_bad=1;fail_clean();assert(!boots);
 prepare();save();boot_bad=1;fail_clean();assert(boots==1 && stops==1);
 boots=0;photo_lut_auto_start();assert(!boots);
 prepare();save();test_load();assert(photo_lut_handle && boots==1 && cache_sync==1 && !handles && !dma_ptr);
 test_load();assert(boots==1 && cache_sync==1);
 photo_lut_shutdown();assert(stops==1);
 prepare();assert(!live_images);
 puts("PASS direct loader CRC/bounds/relocation/core matching, DMA, low memory, ABI/init cleanup; offline boot disabled, explicit manual entry works, low-level guard and orderly shutdown");
}
