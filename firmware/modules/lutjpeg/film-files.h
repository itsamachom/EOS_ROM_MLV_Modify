/* JPEG-first commit, optional bounded display preview, and reversible DCIM export. */
#define FILM_PIXELS (720u*480u)
static int copy_checked(const char *src,const char *dst){
    uint32_t size=0,after=0;int ok=0;FILE *in=NULL,*out=NULL;
    if(is_file(dst) || FIO_GetFileSize(src,&size))return 0;
    unsigned char *dma=fio_malloc(8192);if(!dma)return 0;
    in=FIO_OpenFile(src,O_RDONLY|O_SYNC);if(!in)goto done;
    out=FIO_CreateFile(dst);if(!out)goto done;
    while(size){unsigned n=MIN(size,8192);if(FIO_ReadFile(in,dma,n)!=(int)n || FIO_WriteFile(out,dma,n)!=(int)n)goto done;size-=n;}
    ok=1;
 done:
    if(in){FIO_CloseFile(in);}if(out){FIO_CloseFile(out);}fio_free(dma);
    if(ok){uint32_t original=0;ok=!FIO_GetFileSize(src,&original) && !FIO_GetFileSize(dst,&after) && original==after;}
    if(!ok && out){FIO_RemoveFile(dst);}
    return ok;
}
static int jpeg_valid(const char *path){
    uint32_t n=0;if(FIO_GetFileSize(path,&n) || n<100)return 0;
    unsigned char *dma=fio_malloc(16);if(!dma)return 0;
    FILE *f=FIO_OpenFile(path,O_RDONLY|O_SYNC);int ok=0;
    if(f && FIO_ReadFile(f,dma,2)==2 && dma[0]==255 && dma[1]==216 && FIO_SeekSkipFile(f,n-2,SEEK_SET)>=0 && FIO_ReadFile(f,dma,2)==2 && dma[0]==255 && dma[1]==217)ok=1;
    if(f){FIO_CloseFile(f);}fio_free(dma);return ok;
}
/* Keep a verified film JPEG in ML even when the Canon companion cannot be updated. */
static int publish_companion(const char *source,const char *film,char *published,unsigned cap){
    char camera[128],prepared[128],backup[128];snprintf(camera,sizeof(camera),"%s",source);
    char *ext=strrchr(camera,'.');if(!ext)return 0;strcpy(ext,".JPG");
    snprintf(prepared,sizeof(prepared),"%s",camera);strcpy(strrchr(prepared,'.'),".FJ6");
    snprintf(backup,sizeof(backup),"%s",film);strcpy(strrchr(backup,'.'),".ORG");
    if(is_file(prepared) || !copy_checked(film,prepared))return 0;
    if(!jpeg_valid(prepared)){FIO_RemoveFile(prepared);return 0;}
    int original=is_file(camera);
    if(original && !copy_checked(camera,backup)){FIO_RemoveFile(prepared);return 0;}
    if(original && FIO_RemoveFile(camera)){FIO_RemoveFile(prepared);return 0;}
    if(FIO_RenameFile(prepared,camera)){
        FIO_RemoveFile(camera);
        if(original)copy_checked(backup,camera);
        FIO_RemoveFile(prepared);return 0;
    }
    snprintf(published,cap,"%s",camera);return 1;
}
static int store_review(const unsigned char *indices,const unsigned char *palette,const char *path){
    unsigned char *pixels=indices && palette?malloc(FILM_PIXELS):NULL;
    if(!pixels){
        last_preview[0]=0;
#ifndef LUTJPEG_HOST_TEST
        BMP_LOCK(photo_film_palette_release();if(review_pixels){free(review_pixels);review_pixels=NULL;});
#else
        if(review_pixels){free(review_pixels);review_pixels=NULL;}
#endif
        return 0;
    }
    for(unsigned i=0;i<FILM_PIXELS;i++)pixels[i]=indices[i]>=1 && indices[i]<=144?indices[i]+79:2;
    unsigned crc=film_crc(~0u,palette,144*3);
    uint32_t h[4]={0x36565046u,720,480,~film_crc(crc,pixels,FILM_PIXELS)};
    FILE *f=fos_port_open(path,1);int saved=0;
    if(f){saved=fos_port_write(f,h,16)==16 && fos_port_write(f,palette,144*3)==144*3 && fos_port_write(f,pixels,FILM_PIXELS)==(int)FILM_PIXELS;fos_port_close(f);}
    if(!saved)FIO_RemoveFile(path);
#ifndef LUTJPEG_HOST_TEST
    BMP_LOCK(photo_film_palette_release();unsigned char *old=review_pixels;review_pixels=pixels;memcpy(review_palette,palette,144*3);if(old)free(old));
#else
    if(review_pixels){free(review_pixels);}review_pixels=pixels;memcpy(review_palette,palette,144*3);
#endif
    if(saved)snprintf(last_preview,sizeof(last_preview),"%s",path);else last_preview[0]=0;
    return 1; /* An in-memory view is still available if its optional file failed. */
}
static int load_review(void){
    if(review_pixels)return 1;
    uint32_t size=0;if(!last_preview[0] || FIO_GetFileSize(last_preview,&size) || size!=16+144*3+FILM_PIXELS)return 0;
    unsigned char *dma=fio_malloc(8192),*pixels=malloc(FILM_PIXELS);FILE *f=NULL;int ok=0;
    if(!dma || !pixels)goto done;
    f=FIO_OpenFile(last_preview,O_RDONLY|O_SYNC);if(!f || FIO_ReadFile(f,dma,16)!=16)goto done;
    uint32_t h[4];memcpy(h,dma,16);if(h[0]!=0x36565046u || h[1]!=720 || h[2]!=480)goto done;
    unsigned char palette[144*3];if(FIO_ReadFile(f,dma,144*3)!=144*3)goto done;memcpy(palette,dma,144*3);
    unsigned crc=film_crc(~0u,palette,144*3);
    for(unsigned off=0;off<FILM_PIXELS;){unsigned n=MIN(FILM_PIXELS-off,8192);if(FIO_ReadFile(f,dma,n)!=(int)n)goto done;for(unsigned j=0;j<n;j++)if(dma[j]!=2 && (dma[j]<80 || dma[j]>=224))goto done;memcpy(pixels+off,dma,n);crc=film_crc(crc,dma,n);off+=n;}
    if(~crc!=h[3])goto done;
    review_pixels=pixels;memcpy(review_palette,palette,144*3);pixels=NULL;ok=1;
 done:
    if(f){FIO_CloseFile(f);}if(dma){fio_free(dma);}if(pixels){free(pixels);}return ok;
}
struct film_last_record {uint32_t magic,index;char jpeg[128],preview[128],source[128];uint32_t crc;};
static void last_record(int save){
    struct film_last_record *r=fio_malloc(sizeof(*r));if(!r)return;
    if(save){memset(r,0,sizeof(*r));r->magic=0x364c4946u;r->index=last_saved_film;snprintf(r->jpeg,128,"%s",last_saved);snprintf(r->preview,128,"%s",last_preview);snprintf(r->source,128,"%s",last_source);r->crc=~film_crc(~0u,(unsigned char*)r,sizeof(*r)-4);}
    FILE *f=save?FIO_CreateFile("ML/SETTINGS/FILMLAST.CFG"):FIO_OpenFile("ML/SETTINGS/FILMLAST.CFG",O_RDONLY|O_SYNC);
    if(f){if(save)FIO_WriteFile(f,r,sizeof(*r));else if(FIO_ReadFile(f,r,sizeof(*r))==sizeof(*r) && r->magic==0x364c4946u && r->index<7 && r->crc==~film_crc(~0u,(unsigned char*)r,sizeof(*r)-4) && r->jpeg[127]==0 && r->preview[127]==0 && r->source[127]==0){memcpy(last_saved,r->jpeg,128);memcpy(last_preview,r->preview,128);memcpy(last_source,r->source,128);last_saved_film=r->index;}FIO_CloseFile(f);}
    fio_free(r);
}
