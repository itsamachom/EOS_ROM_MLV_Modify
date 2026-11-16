/* Thai-Menu LUT prototype. GPL-2.0-or-later; processing is credited GPL-2.0-only upstream. */
#ifdef LUTJPEG_HOST_TEST
#include "camera_fixture.h"
#else
#include "dryos.h"
#include "module.h"
#include "config.h"
#include "menu.h"
#include "lens.h"
#include "shoot.h"
#include "debug.h"
#include "fileprefix.h"
#endif
#include "upstream/fos.h"
#include "upstream/fos_dev.h"

static int selected;
static const int films[]={0,2,7};
static const char *const names[]={"Portra 400","Ektar 100","Tri-X 400"};
static const char *const dirs[]={"P400","E100","TX400"};
static volatile int running,percent,cancel;
static char status[100]="RAW/RAW+JPEG required; output 720x480";
static char input[128],output[128],print[128];
static int chosen;
static unsigned char *bounce;
static int local_errno;
int *lut_errno(void){return &local_errno;}
const char *fos_port_model(void){return "Canon EOS 600D";}
int fos_port_ms(void){return get_ms_clock();}
int fos_port_dev_cap(void){return 64;}
void *fos_port_alloc(int n){return n>0?tmp_malloc(n):NULL;}
void fos_port_free(void *p){if(p)free(p);}
void fos_port_log(const char *s){fos_copy(status,sizeof(status),s,sizeof(status)-1);}
void *fos_port_open(const char *path,int write){return write?FIO_CreateFile(path):FIO_OpenFile(path,O_RDONLY|O_SYNC);}
int fos_port_read(void *f,void *buf,int size){
    int total=0;
    while(total<size){int n=MIN(size-total,8192);int got=FIO_ReadFile(f,bounce,n);if(got<=0)break;memcpy((char*)buf+total,bounce,got);total+=got;if(got!=n)break;}
    return total;
}
int fos_port_write(void *f,const void *buf,int size){
    int total=0;
    while(total<size){int n=MIN(size-total,8192);memcpy(bounce,(char*)buf+total,n);int got=FIO_WriteFile(f,bounce,n);if(got<=0)break;total+=got;if(got!=n)break;}
    return total;
}
int fos_port_seek(void *f,int offset){return FIO_SeekSkipFile(f,offset,SEEK_SET)<0?-1:0;}
void fos_port_close(void *f){if(f)FIO_CloseFile(f);}

static void convert_task(void *unused){
    (void)unused;int ok=0;
    struct fos_dev *d=NULL;
    char table_dir[64],jpeg_part[128],print_part[128];
    snprintf(table_dir,sizeof(table_dir),"%s:/ML/LUTDATA",get_shooting_card()->drive_letter);
    snprintf(jpeg_part,sizeof(jpeg_part),"%s",output);char *ext=strrchr(jpeg_part,'.');if(ext)strcpy(ext,".TMP");
    snprintf(print_part,sizeof(print_part),"%s",print);ext=strrchr(print_part,'.');if(ext)strcpy(ext,".PART");
    bounce=fio_malloc(8192);
    if(!bounce){snprintf(status,sizeof(status),"No DMA memory");goto done;}
    /* A completed camera job and an unchanged size across 500 ms are required. */
    uint32_t before=0,after=0;
    if(FIO_GetFileSize(input,&before)){snprintf(status,sizeof(status),"RAW disappeared");goto done;}
    msleep(500);
    if(lens_info.job_state || FIO_GetFileSize(input,&after) || before!=after){snprintf(status,sizeof(status),"RAW still being saved; retry");goto done;}
    fos_dev_cache(table_dir);
    d=fos_dev_start(input,print_part,films[chosen],1,"","");
    if(!d){snprintf(status,sizeof(status),"No processing memory");goto done;}
    fos_dev_jpeg(d,jpeg_part);
    int p=0,start=get_ms_clock();
    while(p>=0 && p<1000){
        if(cancel || lens_info.job_state || get_halfshutter_pressed() || RECORDING || is_movie_mode()){
            snprintf(status,sizeof(status),"Cancelled; RAW unchanged");goto done;
        }
        p=fos_dev_step(d,10);percent=p>=0?p/10:0;
        if(p<0){snprintf(status,sizeof(status),"%s",fos_dev_error(d));goto done;}
        if(percent%10==0)menu_redraw();
        msleep(10);
    }
    uint32_t size=0;
    if(FIO_GetFileSize(jpeg_part,&size) || size<100){snprintf(status,sizeof(status),"JPEG incomplete");goto done;}
    if(FIO_RenameFile(jpeg_part,output)){FIO_RemoveFile(output);snprintf(status,sizeof(status),"JPEG rename failed");goto done;}
    ok=1;
    /* Screen print is auxiliary. A valid JPEG remains even if its rename fails. */
    FIO_RenameFile(print_part,print);
    snprintf(status,sizeof(status),"%s ready (%ds); ML/LUTOUT/%s",names[chosen],(get_ms_clock()-start)/1000,dirs[chosen]);
 done:
    if(d)fos_dev_free(d);
    fos_dev_release();
    if(bounce){fio_free(bounce);bounce=NULL;}
    if(!ok)FIO_RemoveFile(jpeg_part);
    FIO_RemoveFile(print_part);
    percent=ok?100:0;
    NotifyBox(5000,"%s",status);menu_redraw();running=0;
}
static MENU_SELECT_FUNC(start_convert){
    (void)priv;(void)delta;
    if(running)return;
    if(lens_info.job_state || RECORDING || is_movie_mode() || get_halfshutter_pressed()){
        NotifyBox(3000,"Finish shooting before converting");return;
    }
    const struct card_info *card=get_shooting_card();uint32_t size=0;
    const char *prefixes[]={get_file_prefix(),"IMG_","_MG_"};
    int found=0;
    for(int i=0;i<3;i++){
        snprintf(input,sizeof(input),"%s/%s%04d.CR2",get_dcim_dir(),prefixes[i],card->file_number);
        if(!FIO_GetFileSize(input,&size) && size>1024){found=1;break;}
    }
    if(!found){NotifyBox(5000,"Latest image has no RAW.\nSet Canon RAW+JPEG, then take a photo.");return;}
    chosen=COERCE(selected,0,(int)COUNT(films)-1);
    char parent[64],dir[80];
    snprintf(parent,sizeof(parent),"%s:/ML/LUTOUT",card->drive_letter);
    FIO_CreateDirectory(parent);
    snprintf(dir,sizeof(dir),"%s/%s",parent,dirs[chosen]);FIO_CreateDirectory(dir);
    int slot;
    for(slot=1;slot<=9999;slot++){
        snprintf(output,sizeof(output),"%s/LUT_%04d.JPG",dir,slot);
        snprintf(print,sizeof(print),"%s/LUT_%04d.BMP",dir,slot);
        char part[128];snprintf(part,sizeof(part),"%s/LUT_%04d.TMP",dir,slot);
        if(!is_file(output) && !is_file(print) && !is_file(part))break;
    }
    if(slot>9999){NotifyBox(3000,"Output folder full");return;}
    cancel=0;percent=0;running=1;snprintf(status,sizeof(status),"Reading RAW: %s",names[chosen]);
    task_create("lut_jpeg",0x1c,0x8000,convert_task,0);
}
static MENU_SELECT_FUNC(cancel_convert){(void)priv;(void)delta;if(running)cancel=1;}
static MENU_UPDATE_FUNC(show_status){MENU_SET_VALUE("%s",running?"Working":"Ready");MENU_SET_RINFO("%d%%",percent);MENU_SET_HELP("%s",status);}
static struct menu_entry entries[]={
 {.name="LUT JPEG",.select=menu_open_submenu,.icon_type=IT_SUBMENU,.update=show_status,.help="Experimental RAW to LUT to JPEG. 600D.102 only.",.submenu_width=710,
 .children=(struct menu_entry[]){
  {.name="Film LUT",.priv=&selected,.min=0,.max=COUNT(films)-1,.choices=CHOICES("Portra 400","Ektar 100","Tri-X 400"),.help="lut4: matrix, tone curve and selective colour. Three test looks.",.help2="No ISO/Picture Style change. New JPEG from RAW; not Live View."},
  {.name="Convert latest RAW",.select=start_convert,.help="After shooting: SET to create a new 720x480 JPEG.",.help2="Canon RAW/RAW+JPEG required. Output: ML/LUTOUT. Originals kept."},
  {.name="Conversion status",.update=show_status,.help="Progress and last output folder."},
  {.name="Cancel conversion",.select=cancel_convert,.help="Stop processing; source RAW is preserved."},MENU_EOL}},
};
static int registered;
unsigned int lutjpeg_abi(void){return 0x4c554933;}
unsigned int lutjpeg_bootstrap(void){if(!registered){menu_add("Shoot",entries,COUNT(entries));registered=1;}return 0;}
/* Module shutdown must wait for cancellation/close; loader ignores deinit returns. */
unsigned int lutjpeg_shutdown(void){cancel=1;while(running)msleep(20);if(registered){menu_remove("Shoot",entries,COUNT(entries));registered=0;}return 0;}
MODULE_INFO_START()
 MODULE_INIT(lutjpeg_bootstrap)
 MODULE_DEINIT(lutjpeg_shutdown)
MODULE_INFO_END()
