/* Explicit RAW file conversion only; GPL-2.0-or-later. */
#include "trial-art.h"
#ifndef LUTJPEG_HOST_TEST
extern void photo_film_text(int,int,int,int,const char*);
#endif
#define RAW_LIMIT 128
#define BATCH_LIMIT 4
static const int offline_films[]={8,9,11,19,20};
static const int offline_art[]={0,1,2,4,5};
static const char *offline_names[]={"Velvia","Astia","Classic Neg","Gold 200","Ektachrome"};
static const char *offline_tags[]={"Velvia","Astia","ClassicNeg","Gold200","Ektachrome"};
static char raw_files[RAW_LIMIT][128],batch[BATCH_LIMIT][128];
static int marks[RAW_LIMIT],raw_count,cursor,film,job_film,total,done,failed,warnings;
static volatile int mode,busy,cancel,stopping,percent;
static unsigned char *art;
static char status[96]="Choose film",last_file[128];
static int registered;
static const char *base_name(const char *s){const char *p=strrchr(s,'/');return p?p+1:s;}
static void load_art(void){
 if(art)return;
 unsigned char *b=malloc(FILM7_BYTES),*dma=fio_malloc(8192);FILE *f=NULL;int ok=0;
 if(!b || !dma)goto out;
 f=FIO_OpenFile("ML/LUTDATA/FILM7.BIN",O_RDONLY|O_SYNC);if(!f)goto out;
 for(unsigned off=0;off<FILM7_BYTES;){unsigned n=MIN(8192,FILM7_BYTES-off);if(FIO_ReadFile(f,dma,n)!=(int)n)goto out;memcpy(b+off,dma,n);off+=n;}
 if(~film_crc(~0u,b,FILM7_META_BYTES)!=FILM7_META_CRC)goto out;
 for(int i=0;i<7;i++){uint32_t crc;memcpy(&crc,b+656+i*4,4);if(~film_crc(~0u,b+FILM7_META_BYTES+i*FILM7_CELL_BYTES,FILM7_CELL_BYTES)!=crc)goto out;}
 /* Use existing ML grayscale entries. Never change LCD palette registers. */
 for(unsigned off=FILM7_META_BYTES;off<FILM7_BYTES;off++){
  unsigned index=b[off];uint32_t rgb=0;
  if(index>=80 && index<240)memcpy(&rgb,b+16+(index-80)*4,4);
  unsigned luma=(((rgb>>16)&255)*299+((rgb>>8)&255)*587+(rgb&255)*114)/1000;
#ifndef LUTJPEG_HOST_TEST
  b[off]=index?COLOR_GRAY(luma*100/255):COLOR_BLACK;
#else
  b[off]=index?(unsigned char)(38+luma*41/255):2;
#endif
 }
 art=b;b=NULL;ok=1;
 out:if(f)FIO_CloseFile(f);if(dma)fio_free(dma);if(b)free(b);(void)ok;
}
static int selected_count(void){int n=0;for(int i=0;i<raw_count;i++)n+=marks[i]!=0;return n;}
static void scan_dir(const char *path,int depth){
#if !defined(LUTJPEG_HOST_TEST) || defined(OFFLINE_TEST_FS)
 struct fio_file *file=alloc_fio_file();if(!file)return;
 struct fio_dirent *dir=FIO_FindFirstEx(path,file);
 if(IS_ERROR(dir)){free(file);return;}
 do{
  struct file_info info=convert_fio_file_info(file);
  if(!info.name[0] || info.name[0]=='.')continue;
  char p[128];if(snprintf(p,sizeof(p),"%s/%s",path,info.name)>=(int)sizeof(p))continue;
  if(info.mode & ATTR_DIRECTORY){if(depth<1)scan_dir(p,depth+1);}
  else {char *ext=strrchr(info.name,'.');if(ext && (!strcmp(ext,".CR2") || !strcmp(ext,".cr2")) && raw_count<RAW_LIMIT)snprintf(raw_files[raw_count++],128,"%s",p);}
 }while(!cancel && !stopping && raw_count<RAW_LIMIT && FIO_FindNextEx(dir,file)==0);
 FIO_FindClose(dir);free(file);
#else
 (void)path;(void)depth;
#endif
}
static void scan_task(void *unused){
 (void)unused;load_art();raw_count=cursor=0;memset(marks,0,sizeof(marks));
 char path[32];const struct card_info *c=get_shooting_card();
 if(c){snprintf(path,sizeof(path),"%s:/DCIM",c->drive_letter);scan_dir(path,0);}
 for(int i=1;i<raw_count;i++)for(int j=i;j>0 && strcmp(raw_files[j-1],raw_files[j])>0;j--){char tmp[128];memcpy(tmp,raw_files[j],128);memcpy(raw_files[j],raw_files[j-1],128);memcpy(raw_files[j-1],tmp,128);}
 snprintf(status,sizeof(status),raw_count==RAW_LIMIT?"First 128 RAW files listed":"SET marks photo; Q starts conversion");
 if(!stopping && !cancel){mode=2;}busy=0;menu_redraw();
}
static int jpeg_ok(const char *p){
 uint32_t n=0;if(FIO_GetFileSize(p,&n) || n<100)return 0;
 FILE *f=FIO_OpenFile(p,O_RDONLY|O_SYNC);int ok=0;
 if(f && FIO_ReadFile(f,bounce,2)==2 && bounce[0]==255 && bounce[1]==216 && FIO_SeekSkipFile(f,n-2,SEEK_SET)>=0 && FIO_ReadFile(f,bounce,2)==2 && bounce[0]==255 && bounce[1]==217)ok=1;
 if(f){FIO_CloseFile(f);}return ok;
}
static int copy_file(const char *src,const char *dst){
 uint32_t n=0,after=0;int ok=0;FILE *in=NULL,*out=NULL;
 if(is_file(dst) || FIO_GetFileSize(src,&n))return 0;
 in=FIO_OpenFile(src,O_RDONLY|O_SYNC);if(!in)goto end;
 out=FIO_CreateFile(dst);if(!out)goto end;
 while(n){unsigned k=MIN(n,8192);if(FIO_ReadFile(in,bounce,k)!=(int)k || FIO_WriteFile(out,bounce,k)!=(int)k)goto end;n-=k;}
 ok=1;
 end:if(in)FIO_CloseFile(in);if(out)FIO_CloseFile(out);
 if(ok){uint32_t original=0;ok=!FIO_GetFileSize(src,&original) && !FIO_GetFileSize(dst,&after) && original==after;}
 if(!ok && out){FIO_RemoveFile(dst);}return ok;
}
static int output_paths(const char *raw,int index,char *temp,char *dcf,char *named){
 char folder[64],stem[64];snprintf(folder,sizeof(folder),"%c:/DCIM/900FILM_",raw[0]);FIO_CreateDirectory(folder);
 for(int i=1;i<=9999;i++){
  snprintf(dcf,128,"%s/FILM%04d.JPG",folder,i);snprintf(temp,128,"%s/FILM%04d.TMP",folder,i);
  if(!is_file(dcf) && !is_file(temp))break;
  if(i==9999)return 0;
 }
 snprintf(folder,sizeof(folder),"%c:/ML/FILMOUT",raw[0]);FIO_CreateDirectory(folder);
 snprintf(stem,sizeof(stem),"%s",base_name(raw));char *e=strrchr(stem,'.');if(!e)return 0;*e=0;
 for(int i=0;i<=999;i++){
  if(i)snprintf(named,128,"%s/%s_%s_%03d.JPG",folder,stem,offline_tags[index],i);
  else snprintf(named,128,"%s/%s_%s.JPG",folder,stem,offline_tags[index]);
  if(!is_file(named))return 1;
 }return 0;
}
static int convert_one(const char *raw){
 char temp[128],dcf[128],named[128],tables[48];struct fos_dev *dev=NULL;int ok=0;
 if(!output_paths(raw,job_film,temp,dcf,named)){snprintf(status,sizeof(status),"Output folder full");return 0;}
 snprintf(tables,sizeof(tables),"%c:/ML/LUTDATA",raw[0]);
 fos_dev_cache(tables);dev=fos_dev_start(raw,"",offline_films[job_film],1,"","");
 if(!dev){snprintf(status,sizeof(status),"Not enough memory");goto end;}
 fos_dev_big(dev,FOS_SIZE_L,NULL);fos_dev_jpeg(dev,temp);
 int p=0;
 while(p>=0 && p<1000){
  if(cancel || stopping || lens_info.job_state || RECORDING || get_halfshutter_pressed())goto end;
  p=fos_dev_step(dev,10);percent=p<0?0:p/10;menu_redraw();msleep(10);
 }
 if(p<0){snprintf(status,sizeof(status),"%s",fos_dev_error(dev));goto end;}
 if(!jpeg_ok(temp)){snprintf(status,sizeof(status),"Cannot save JPEG");goto end;}
 if(FIO_RenameFile(temp,dcf)){FIO_RemoveFile(dcf);snprintf(status,sizeof(status),"Cannot save JPEG");goto end;}
 ok=1;snprintf(last_file,sizeof(last_file),"%s",dcf);
 if(copy_file(dcf,named)){snprintf(last_file,sizeof(last_file),"%s",named);}else warnings++;
 /* DCF alias is also the manifest key; no original camera file is replaced. */
 char map[128];snprintf(map,sizeof(map),"%s",dcf);strcpy(strrchr(map,'.'),".TXT");
 FILE *f=FIO_CreateFile(map);if(f){int n=snprintf((char*)bounce,8192,"RAW: %s\nFilm: %s\nNamed: %s\nDCF: %s\n",raw,offline_names[job_film],last_file,dcf);if(FIO_WriteFile(f,bounce,n)!=n)warnings++;FIO_CloseFile(f);}else warnings++;
 end:if(dev)fos_dev_free(dev);fos_dev_release();if(!ok)FIO_RemoveFile(temp);return ok;
}
static void batch_task(void *unused){
 (void)unused;bounce=fio_malloc(8192);
 if(!bounce){snprintf(status,sizeof(status),"Not enough file memory");failed=total;goto end;}
 for(int i=0;i<total && !cancel && !stopping;i++){
  percent=0;snprintf(status,sizeof(status),"%s",base_name(batch[i]));
  if(convert_one(batch[i]))done++;else if(!cancel)failed++;
 }
 fio_free(bounce);bounce=NULL;
 snprintf(status,sizeof(status),cancel?"Cancelled; original RAW kept":failed?"Finished with failed files":warnings?"JPEG saved; check filename/export warnings":"Finished; original RAW kept");
 end:busy=0;menu_redraw();
}
static void start_batch(void){
 if(busy){return;}total=0;
 for(int i=0;i<raw_count && total<BATCH_LIMIT;i++)if(marks[i])snprintf(batch[total++],128,"%s",raw_files[i]);
 if(!total){snprintf(status,sizeof(status),"Select 1 to 4 RAW files first");return;}
 job_film=film;done=failed=warnings=percent=cancel=0;mode=3;busy=1;
 task_create("raw_film",0x1e,0x8000,batch_task,NULL);
}
static int offline_key(int key){
 if(!mode || stopping)return -1;
 if(key==BGMT_PLAY || key==BGMT_PRESS_HALFSHUTTER){cancel=1;mode=0;return 0;}
 if(key==BGMT_MENU){if(busy)cancel=1;else if(mode==2)mode=1;else mode=0;menu_redraw();return 1;}
 if(busy)return 1;
 if(mode==1){
  if(key==BGMT_PRESS_LEFT || key==BGMT_WHEEL_LEFT)film=(film+4)%5;
  if(key==BGMT_PRESS_RIGHT || key==BGMT_WHEEL_RIGHT)film=(film+1)%5;
  if(key==BGMT_PRESS_SET){busy=1;cancel=0;snprintf(status,sizeof(status),"Reading RAW files");task_create("film_scan",0x1e,0x4000,scan_task,NULL);}
 }else if(mode==2){
  if(raw_count){
   if(key==BGMT_PRESS_UP || key==BGMT_WHEEL_UP)cursor=(cursor+raw_count-1)%raw_count;
   if(key==BGMT_PRESS_DOWN || key==BGMT_WHEEL_DOWN)cursor=(cursor+1)%raw_count;
   if(key==BGMT_PRESS_SET){if(marks[cursor])marks[cursor]=0;else if(selected_count()<BATCH_LIMIT)marks[cursor]=1;else snprintf(status,sizeof(status),"Maximum 4 photos");}
  }
  if(key==BGMT_Q)start_batch();
 }else if(mode==3 && key==BGMT_PRESS_SET)mode=1;
 menu_redraw();return 1;
}
#include "offline-screen.h"
static MENU_SELECT_FUNC(open_converter){(void)priv;(void)delta;if(!busy){mode=1;cancel=0;load_art();}menu_redraw();}
static struct menu_entry entries[]={
 {.name="LUT JPEG",.select=open_converter,.icon_type=IT_ACTION,.help="Offline RAW converter: five films, select up to four photos. Originals kept."},
};
unsigned int lutjpeg_abi(void){return 0x4c554933;}
unsigned int lutjpeg_bootstrap(void){
 if(!photo_offline_allowed())return 1;
 if(!registered){stopping=0;load_art();menu_add("Shoot",entries,COUNT(entries));struct offline_hooks h={offline_draw,offline_key};photo_offline_register(&h);registered=1;mode=1;}
 return 0;
}
unsigned int lutjpeg_shutdown(void){
 stopping=cancel=1;while(busy)msleep(20);photo_offline_register(NULL);mode=0;
 if(registered){menu_remove("Shoot",entries,COUNT(entries));registered=0;}
 if(art){free(art);art=NULL;}return 0;
}
MODULE_INFO_START()
 MODULE_INIT(lutjpeg_bootstrap)
 MODULE_DEINIT(lutjpeg_shutdown)
MODULE_INFO_END()
