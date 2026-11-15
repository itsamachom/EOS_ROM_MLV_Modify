/* 600D bound LUT overlay loader. GPL-2.0-or-later; no camera-side TCC. */
#ifdef CONFIG_600D
#ifndef PHOTO_LUT_LOADER_TEST
#include "arm-mcr.h"
#endif
static volatile int photo_lut_loading;
static unsigned char *photo_lut_handle;
static uint32_t photo_lut_stop;
static char photo_lut_status[96]="SET: Load LUT JPEG engine";
#define PHOTO_LUT_ABI 0x4c554933
#ifndef PHOTO_LUT_CALL
#define PHOTO_LUT_CALL(base,off) (((int (*)(void))((base)+(off)))())
#endif
static unsigned photo_lut_crc(unsigned crc,const unsigned char *p,unsigned n){
    while(n--){crc^=*p++;for(int k=0;k<8;k++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}return crc;
}
static int photo_lut_file_crc(const char *path,unsigned char *dma,uint32_t expected_size,uint32_t expected_crc){
    uint32_t size=0;if(FIO_GetFileSize(path,&size) || (expected_size && size!=expected_size))return 0;
    FILE *f=FIO_OpenFile(path,O_RDONLY|O_SYNC);if(!f)return 0;
    unsigned crc=~0u;int ok=1;
    while(size){unsigned n=size>8192?8192:size;if(FIO_ReadFile(f,dma,n)!=(int)n){ok=0;break;}crc=photo_lut_crc(crc,dma,n);size-=n;}
    FIO_CloseFile(f);return ok && ~crc==expected_crc;
}
static void photo_lut_log(const char *message){
    FILE *f=FIO_CreateFileOrAppend("ML/LUTLOAD.LOG");
    if(f){FIO_WriteFile(f,message,strlen(message));FIO_WriteFile(f,"\n",1);FIO_CloseFile(f);}
}
static void photo_lut_load_task(void *unused){
    (void)unused;uint32_t h[16],size=0;FILE *f=NULL;
    unsigned char *dma=NULL,*image=NULL;int ready=0;
    photo_lut_log("Thai-Menu LUT3: direct overlay load; no module startup wait");
    if(photo_lut_handle){ready=1;goto done;}
    if(FIO_GetFileSize("ML/LUTDATA/LUTJPEG.BIN",&size) || size<64){snprintf(photo_lut_status,sizeof(photo_lut_status),"Missing ML/LUTDATA/LUTJPEG.BIN; copy full ML folder");goto done;}
    dma=fio_malloc(8192);if(!dma){snprintf(photo_lut_status,sizeof(photo_lut_status),"No file-read memory");goto done;}
    f=FIO_OpenFile("ML/LUTDATA/LUTJPEG.BIN",O_RDONLY|O_SYNC);
    if(!f || FIO_ReadFile(f,dma,64)!=64)goto bad;
    memcpy(h,dma,64);
    if(h[0]!=0x3354554c || h[1]!=1 || h[2]!=0x600d0102 || h[12]!=PHOTO_LUT_ABI || ~photo_lut_crc(~0u,(const unsigned char*)h,60)!=h[15] ||
       h[3]<64 || h[3]>h[4] || h[4]>512*1024 || h[5]>4096 || size!=64+h[3]+4*h[5])goto bad;
    for(int i=6;i<=8;i++)if((h[i]&3) || h[i]>h[3]-4)goto bad;
    if(!photo_lut_file_crc("autoexec.bin",dma,h[14],h[13]) ||
       !photo_lut_file_crc("ML/modules/600D_102.sym",dma,0,h[11])){
        snprintf(photo_lut_status,sizeof(photo_lut_status),"Program files differ; copy autoexec.bin and ML together");goto done;
    }
    image=malloc(h[4]);if(!image){snprintf(photo_lut_status,sizeof(photo_lut_status),"No LUT engine memory");goto done;}
    memset(image,0,h[4]);unsigned crc=~0u,off=0;
    while(off<h[3]){unsigned n=h[3]-off;if(n>8192)n=8192;if(FIO_ReadFile(f,dma,n)!=(int)n)goto bad;memcpy(image+off,dma,n);crc=photo_lut_crc(crc,dma,n);off+=n;}
    if(~crc!=h[9])goto bad;
    crc=~0u;off=0;uint32_t previous=0;
    while(off<h[5]){
        unsigned count=h[5]-off;if(count>2048)count=2048;
        if(FIO_ReadFile(f,dma,count*4)!=(int)(count*4))goto bad;
        crc=photo_lut_crc(crc,dma,count*4);
        for(unsigned i=0;i<count;i++){
            uint32_t pos,value;memcpy(&pos,dma+i*4,4);
            if((pos&3) || pos>h[3]-4 || ((off+i)>0 && pos<=previous))goto bad;
            previous=pos;
            memcpy(&value,image+pos,4);if(value>h[4])goto bad;
            value+=(uint32_t)(uintptr_t)image;memcpy(image+pos,&value,4);
        }
        off+=count;
    }
    if(~crc!=h[10])goto bad;
    sync_caches();
    if(PHOTO_LUT_CALL(image,h[8])!=PHOTO_LUT_ABI)goto bad;
    if(PHOTO_LUT_CALL(image,h[6])!=0){PHOTO_LUT_CALL(image,h[7]);goto bad;}
    photo_lut_handle=image;image=NULL;photo_lut_stop=h[7];ready=1;
    goto done;
 bad:
    snprintf(photo_lut_status,sizeof(photo_lut_status),"LUT engine file invalid/incomplete; copy full ML folder");
 done:
    if(f)FIO_CloseFile(f);
    if(dma)fio_free(dma);
    if(image)free(image);
    if(ready)snprintf(photo_lut_status,sizeof(photo_lut_status),"LUT ready; open FLIM TONE > LUT JPEG again");
    photo_lut_log(photo_lut_status);
    NotifyBox(3500,"%s",ready?"LUT ready":"Cannot load LUT. See ML/LUTLOAD.LOG");
    photo_lut_loading=0;menu_redraw();
}
static MENU_SELECT_FUNC(photo_lut_load_select){
    (void)priv;(void)delta;if(photo_lut_loading)return;
    photo_lut_loading=1;snprintf(photo_lut_status,sizeof(photo_lut_status),"Loading LUT engine...");
    task_create("lut_loader",0x1c,0x4000,photo_lut_load_task,NULL);
}
static MENU_UPDATE_FUNC(photo_lut_load_update){
    MENU_SET_VALUE("%s",photo_lut_loading?"Loading":"SET Load");MENU_SET_HELP("%s",photo_lut_status);
}
void photo_lut_shutdown(void){
    while(photo_lut_loading)msleep(20);
    if(photo_lut_handle)PHOTO_LUT_CALL(photo_lut_handle,photo_lut_stop);
}
#endif
