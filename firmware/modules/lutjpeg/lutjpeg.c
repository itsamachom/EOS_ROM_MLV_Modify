/* Automatic RAW film lab. GPL-2.0-or-later; external recipe assets retain their licenses. */
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
#include "bmp.h"
#include "gui-common.h"
#include "propvalues.h"
#include "property.h"
#endif
#ifdef BGMT_Q
#undef BGMT_Q
#endif
#include "../../platform/600D.102/gui.h"
#include "upstream/fos.h"
#include "upstream/fos_dev.h"
#include "seven-films.h"
#include "film-colour.h"

static int within,auto_enabled=1,output_size=FOS_SIZE_L;
static volatile int running,percent,cancel,stopping,monitor_alive;
static int chosen,last_width,last_height,last_made,completed,failed,dropped,stage;
static char status[100]="Choose a film, enable RAW+JPEG, then shoot";
static char input[128],output[128],print[128];
static unsigned char *bounce;
static int local_errno;
static void *queue_sem;
#define QUEUE_SIZE 16
struct film_job {char path[128],drive[4];int film,attempts,size;};
static struct film_job queue[QUEUE_SIZE],current_job;
static int qhead;
static volatile int qcount;
static char observed_dir[96];
static int observed_number=-1,observed_busy,stable_since,quality_requested;
static uint32_t stable_size;
static int current_index(void){return COERCE(within,0,6);}
static void lock_queue(void){take_semaphore(queue_sem,0);}
static void unlock_queue(void){give_semaphore(queue_sem);}
int *lut_errno(void){return &local_errno;}
const char *fos_port_model(void){return "Canon EOS 600D";}
int fos_port_ms(void){return get_ms_clock();}
int fos_port_dev_cap(void){return 64;}
void *fos_port_alloc(int n){return n>0?tmp_malloc(n):NULL;}
void fos_port_free(void *p){if(p)free(p);}
void fos_port_log(const char *s){(void)s; /* Internal logs do not replace the user-facing stage. */}
void *fos_port_open(const char *path,int write){return write?FIO_CreateFile(path):FIO_OpenFile(path,O_RDONLY|O_SYNC);}
int fos_port_read(void *f,void *buf,int size){int total=0;while(total<size){int n=MIN(size-total,8192),got=FIO_ReadFile(f,bounce,n);if(got<=0)break;memcpy((char*)buf+total,bounce,got);total+=got;if(got!=n)break;}return total;}
int fos_port_write(void *f,const void *buf,int size){int total=0;while(total<size){int n=MIN(size-total,8192);memcpy(bounce,(char*)buf+total,n);int got=FIO_WriteFile(f,bounce,n);if(got<=0)break;total+=got;if(got!=n)break;}return total;}
int fos_port_seek(void *f,int offset){return FIO_SeekSkipFile(f,offset,SEEK_SET)<0?-1:0;}
void fos_port_close(void *f){if(f)FIO_CloseFile(f);}

static unsigned config_crc(const uint32_t *p){unsigned h=2166136261u;for(int i=0;i<5;i++)h=(h^p[i])*16777619u;return h;}
static void film_settings(int save){
    uint32_t *dma=fio_malloc(24);if(!dma)return;
    const char *path="ML/SETTINGS/FILMLAB.CFG";
    if(save){dma[0]=0x36464a4c;dma[1]=within;dma[2]=auto_enabled;dma[3]=output_size;dma[4]=0;dma[5]=config_crc(dma);}
    FILE *f=save?FIO_CreateFile(path):FIO_OpenFile(path,O_RDONLY|O_SYNC);
    if(f){
        if(save)FIO_WriteFile(f,dma,24);
        else if(FIO_ReadFile(f,dma,24)==24 && dma[0]==0x36464a4c && dma[1]<7 && dma[2]<=1 && dma[3]<=FOS_SIZE_L && dma[5]==config_crc(dma)){
            within=dma[1];auto_enabled=dma[2];output_size=dma[3];
        }
        FIO_CloseFile(f);
    }
    fio_free(dma);
}

#ifndef LUTJPEG_HOST_TEST
extern void photo_film_text(int font,int x,int y,int width,const char *text);
#endif
#include "film-ui.h"
#include "film-files.h"
#ifndef LUTJPEG_HOST_TEST
/* The core wrapper uses the same Thai atlas and clipped text renderer as Photo UI. */
extern void photo_film_text(int font,int x,int y,int width,const char *text);
static int screen_visible,screen_until,last_screen_ms;
#include "film-progress.h"
static void draw_progress(void){
    int now=get_ms_clock();
    if(gui_menu_shown() || lens_info.job_state || RECORDING || get_halfshutter_pressed() || lv){
        if(screen_visible){screen_visible=0;if(!gui_menu_shown())canon_gui_enable_front_buffer(1);redraw();}return;
    }
    if(now-last_screen_ms<150)return;
    if(!running && now>screen_until){if(screen_visible){screen_visible=0;if(!gui_menu_shown())canon_gui_enable_front_buffer(1);redraw();}return;}
    last_screen_ms=now;if(!screen_visible)canon_gui_disable_front_buffer();screen_visible=1;
    BMP_LOCK(film_progress_draw(now,running,stage,percent,film_names[chosen],status,qcount,completed,last_width,last_height,last_made==current_job.size,leaf(current_job.path[0]?current_job.path:last_source),last_saved,failed));
}
#else
static int screen_until;
static void draw_progress(void){}
#endif

static int enqueue(const struct film_job *job){
    int ok=0;lock_queue();
    if(qcount<QUEUE_SIZE){queue[(qhead+qcount)%QUEUE_SIZE]=*job;qcount++;ok=1;}else dropped++;
    unlock_queue();return ok;
}
static int prepare_output(void){
    char parent[64],dir[80];
    snprintf(parent,sizeof(parent),"%s:/ML/LUTOUT",current_job.drive);FIO_CreateDirectory(parent);
    snprintf(dir,sizeof(dir),"%s/L%03d",parent,films[chosen]);FIO_CreateDirectory(dir);
    for(int slot=1;slot<=9999;slot++){
        snprintf(output,sizeof(output),"%s/LUT_%04d.JPG",dir,slot);snprintf(print,sizeof(print),"%s/LUT_%04d.FPV",dir,slot);
        char a[128],b[128];snprintf(a,sizeof(a),"%s/LUT_%04d.TMP",dir,slot);snprintf(b,sizeof(b),"%s/LUT_%04d.PBM",dir,slot);
        if(!is_file(output) && !is_file(print) && !is_file(a) && !is_file(b))return 1;
    }
    snprintf(status,sizeof(status),"Output folder full");return 0;
}
static void convert_task(void *unused){
    (void)unused;int ok=0,interrupted=0;struct fos_dev *d=NULL;
    char table_dir[64],jpeg_part[128]="",print_part[128]="";
    last_width=last_height=0;
    FILE *guard=FIO_CreateFile("ML/LUTDATA/FILMERR.LCK");
    if(!guard){snprintf(status,sizeof(status),"Cannot write to card; RAW kept");goto done;}
    FIO_CloseFile(guard);
    if(!prepare_output())goto done;
    snprintf(input,sizeof(input),"%s",current_job.path);
    snprintf(table_dir,sizeof(table_dir),"%s:/ML/LUTDATA",current_job.drive);
    snprintf(jpeg_part,sizeof(jpeg_part),"%s",output);char *ext=strrchr(jpeg_part,'.');if(ext)strcpy(ext,".TMP");
    snprintf(print_part,sizeof(print_part),"%s",print);ext=strrchr(print_part,'.');if(ext)strcpy(ext,".PBM");
    bounce=fio_malloc(8192);if(!bounce){snprintf(status,sizeof(status),"Not enough memory; try again later");goto done;}
    uint32_t before=0,after=0;
    if(FIO_GetFileSize(input,&before)){snprintf(status,sizeof(status),"RAW file is missing");goto done;}
    msleep(500);
    if(lens_info.job_state || FIO_GetFileSize(input,&after) || before!=after){interrupted=1;snprintf(status,sizeof(status),"Waiting for camera to finish saving");goto done;}
    fos_dev_cache(table_dir);d=fos_dev_start(input,print_part,films[chosen],1,"","");
    if(!d){snprintf(status,sizeof(status),"Not enough processing memory");goto done;}
    fos_dev_big(d,current_job.size,NULL);fos_dev_jpeg(d,jpeg_part);
    int p=0;
    while(p>=0 && p<1000){
        if(stopping || cancel || lens_info.job_state || get_halfshutter_pressed() || RECORDING || is_movie_mode()){
            interrupted=!stopping && !cancel;snprintf(status,sizeof(status),cancel?"Conversion cancelled; RAW kept":"Paused for shooting; RAW kept");goto done;
        }
        p=fos_dev_step(d,10);stage=fos_dev_doing(d);percent=p>=0?p/10:0;
        if(p<0){snprintf(status,sizeof(status),"%s",fos_dev_error(d));goto done;}
        draw_progress();menu_redraw();msleep(10);
    }
    last_made=fos_dev_made(d,&last_width,&last_height);
    uint32_t size=0;
    if(FIO_GetFileSize(jpeg_part,&size) || size<100 || !jpeg_valid(jpeg_part)){snprintf(status,sizeof(status),"Could not finish JPEG; please retry");goto done;}
    if(FIO_RenameFile(jpeg_part,output)){FIO_RemoveFile(output);snprintf(status,sizeof(status),"Could not save JPEG; please retry");goto done;}
    ok=1;
    snprintf(last_source,sizeof(last_source),"%s",current_job.path);
    snprintf(last_saved,sizeof(last_saved),"%s",output);last_saved_film=chosen;
    int preview_ok=store_review(fos_dev_screen_indices(d),fos_dev_screen_palette(d),print);
    int dcim_ok=publish_companion(input,output,last_saved,sizeof(last_saved));
    last_record(1);
    snprintf(status,sizeof(status),dcim_ok?"Film JPEG saved in DCIM; RAW kept":"Film JPEG saved in ML; DCIM update failed");
    if(preview_ok)pending_view=1;
 done:
    if(d)fos_dev_free(d);
    fos_dev_release();
    if(bounce){fio_free(bounce);bounce=NULL;}
    if(!ok && jpeg_part[0])FIO_RemoveFile(jpeg_part);
    if(print_part[0])FIO_RemoveFile(print_part);
    if(interrupted){current_job.attempts++;if(!enqueue(&current_job))failed++;}
    else if(!ok)failed++;
    FIO_RemoveFile("ML/LUTDATA/FILMERR.LCK");
    if(ok)completed++;
    percent=ok?100:0;screen_until=get_ms_clock()+4000;running=0;
    NotifyBox(3000,"%s",status);menu_redraw();
}

/* Observe shot identity, never the old image present at boot. Capture the film
 * here so changing the selection cannot recolour a job already in the queue. */
static void observe_shot(void){
    const struct card_info *card=get_shooting_card();
    if(!card || !card->drive_letter)return;
    const char *dir=get_dcim_dir();int number=card->file_number;
    if(number<0 || number>9999 || !dir || !card->drive_letter)return;
    if(observed_number<0){observed_number=number;snprintf(observed_dir,sizeof(observed_dir),"%s",dir);return;}
    if(lens_info.job_state)observed_busy=1;
    if(number==observed_number && !strcmp(dir,observed_dir))return;
    int first=!strcmp(dir,observed_dir) && number>observed_number?observed_number+1:number;
    if(observed_number==0 && number>1 && !observed_busy){first=number+1;} /* Late boot metadata. */
    if(auto_enabled && number-first+1>QUEUE_SIZE){dropped+=number-first+1-QUEUE_SIZE;first=number-QUEUE_SIZE+1;}
    if(auto_enabled && !is_movie_mode() && !RECORDING)for(int n=first;n<=number;n++){
        struct film_job j={.film=current_index(),.size=output_size};
        snprintf(j.drive,sizeof(j.drive),"%s",card->drive_letter);
        snprintf(j.path,sizeof(j.path),"%s/%s%04d.CR2",dir,get_file_prefix(),n);enqueue(&j);
    }
    observed_number=number;observed_busy=0;snprintf(observed_dir,sizeof(observed_dir),"%s",dir);
}
static void service_tick(void){
    if(stopping)return;
    if(auto_enabled && !quality_requested && pic_quality && !lens_info.job_state && !RECORDING && !is_movie_mode()){
        quality_requested=1;
        if(!(pic_quality & 0x60000) || (pic_quality & 0xF)){int quality=PICQ_RAW_JPG_LARGE_FINE;prop_request_change(PROP_PIC_QUALITY,&quality,4);}
    }
    load_trial_art();load_live_preview();
    if(pending_view && !running && !lens_info.job_state && !get_halfshutter_pressed() && !RECORDING){
        pending_view=0;
        if(load_review()){
#ifndef LUTJPEG_HOST_TEST
            ui_mode=2;photo_film_mode(2);
#endif
        }else {
            last_preview[0]=0;snprintf(status,sizeof(status),last_saved[0]?"JPEG saved; display preview unavailable":"No converted photo yet");
#ifndef LUTJPEG_HOST_TEST
            ui_mode=2;photo_film_mode(2);
#endif
        }
    }
    observe_shot();draw_progress();
    if(running || lens_info.job_state || get_halfshutter_pressed() || RECORDING || is_movie_mode()){stable_since=0;return;}
    struct film_job j;lock_queue();int pending=qcount;if(pending)j=queue[qhead];unlock_queue();if(!pending)return;
    const struct card_info *card=get_shooting_card();
    if(!card || !card->drive_letter || strcmp(j.drive,card->drive_letter)){snprintf(status,sizeof(status),"Card changed; queued RAW not converted");goto discard;}
    uint32_t size=0;int now=get_ms_clock();
    if(FIO_GetFileSize(j.path,&size) || size<1024){
        if(!stable_since)stable_since=now;
        if(now-stable_since<15000)return;
        snprintf(status,sizeof(status),"No RAW found - enable RAW+JPEG and retry");goto discard;
    }
    if(!stable_since || size!=stable_size){stable_since=now;stable_size=size;return;}
    if(now-stable_since<500)return;
    lock_queue();if(!qcount || strcmp(queue[qhead].path,j.path)){unlock_queue();stable_since=0;return;}current_job=queue[qhead];qhead=(qhead+1)%QUEUE_SIZE;qcount--;unlock_queue();
    chosen=current_job.film;cancel=percent=0;running=1;stage=FOS_DEV_DEVELOPING;stable_since=0;
    snprintf(status,sizeof(status),"Developing your photo");
    task_create("film_convert",0x1e,0x8000,convert_task,NULL);return;
 discard:
    lock_queue();if(qcount && !strcmp(queue[qhead].path,j.path)){qhead=(qhead+1)%QUEUE_SIZE;qcount--;failed++;}unlock_queue();stable_since=0;NotifyBox(5000,"%s",status);
}
#ifndef LUTJPEG_HOST_TEST
static void monitor_task(void *unused){(void)unused;monitor_alive=1;while(!stopping){service_tick();msleep(200);}monitor_alive=0;}
#endif
static MENU_SELECT_FUNC(start_convert){
    (void)priv;(void)delta;const struct card_info *card=get_shooting_card();
    if(!card || !card->drive_letter || !get_dcim_dir()){snprintf(status,sizeof(status),"No camera card available");return;}
    struct film_job job={.film=current_index(),.size=output_size};snprintf(job.drive,sizeof(job.drive),"%s",card->drive_letter);
    const char *prefixes[]={get_file_prefix(),"IMG_","_MG_"};uint32_t size=0;int found=0;
    for(int i=0;i<3;i++){snprintf(job.path,sizeof(job.path),"%s/%s%04d.CR2",get_dcim_dir(),prefixes[i],card->file_number);if(!FIO_GetFileSize(job.path,&size) && size>1024){found=1;break;}}
    if(!found){snprintf(status,sizeof(status),"Set RAW+JPEG, then take a photo");NotifyBox(5000,"%s",status);return;}
    if(enqueue(&job)){stable_since=0;snprintf(status,sizeof(status),"Photo added to conversion queue");}else NotifyBox(3000,"Queue full; original RAW is kept");
}
static MENU_SELECT_FUNC(cancel_convert){(void)priv;(void)delta;lock_queue();qcount=0;unlock_queue();cancel=1;snprintf(status,sizeof(status),"Conversion cancelled; RAW kept");}
static MENU_SELECT_FUNC(open_film_picker){(void)priv;(void)delta;browsing=current_index();ui_mode=1;photo_film_mode(1);}
static MENU_SELECT_FUNC(open_film_result){(void)priv;(void)delta;pending_view=1;}
static MENU_SELECT_FUNC(change_auto){(void)priv;(void)delta;auto_enabled=!auto_enabled;if(auto_enabled)quality_requested=0;film_settings(1);}
static MENU_SELECT_FUNC(change_size){(void)priv;output_size=(output_size+delta+FOS_SIZES)%FOS_SIZES;film_settings(1);}
static MENU_SELECT_FUNC(enable_raw){
    (void)priv;(void)delta;
    if(!lens_info.job_state && !RECORDING && !is_movie_mode()){int quality=PICQ_RAW_JPG_LARGE_FINE;prop_request_change(PROP_PIC_QUALITY,&quality,4);}
}
static MENU_UPDATE_FUNC(show_film){MENU_SET_VALUE("%s",film_names[current_index()]);MENU_SET_HELP("%s",film_notes[current_index()]);}
static MENU_UPDATE_FUNC(show_retry){const struct card_info *c=get_shooting_card();char source[24];snprintf(source,sizeof(source),"%s%04d.CR2",get_file_prefix(),c?c->file_number:0);MENU_SET_VALUE("%s",source);MENU_SET_HELP("Convert this one RAW only; originals are kept.");}
static MENU_UPDATE_FUNC(show_status){char n[16];film_uint(n,completed);MENU_SET_VALUE("%s",running?"Developing":"Ready");MENU_SET_RINFO("%s saved",n);MENU_SET_HELP("%s",status);}
static struct menu_entry entries[]={
 {.name="LUT JPEG",.select=menu_open_submenu,.icon_type=IT_SUBMENU,.update=show_status,.help="Choose film, set RAW+JPEG once, shoot. JPEG develops automatically.",.submenu_width=710,
 .children=(struct menu_entry[]){
  {.name="Choose film",.select=open_film_picker,.update=show_film,.icon_type=IT_ACTION,.help="Choose one of seven complete films. No separate collection setting."},
  {.name="View film photo",.select=open_film_result,.icon_type=IT_ACTION,.help="Open the last successfully developed film image on this camera."},
  {.name="After each photo",.priv=&auto_enabled,.min=0,.max=1,.choices=CHOICES("Manual","Automatic"),.select=change_auto,.help="Automatic is enabled by default. RAW files are required and kept."},
  {.name="JPEG output size",.priv=&output_size,.min=0,.max=FOS_SIZE_L,.choices=CHOICES("Small 720x480","Medium ~1728x1152","Large ~2592x1728"),.select=change_size,.help="Large is the default. Actual size and memory fallback are shown after conversion."},
  {.name="Enable RAW+JPEG",.select=enable_raw,.help="SET once to save RAW plus Canon JPEG for automatic film development."},
  {.name="Develop last photo",.select=start_convert,.update=show_retry,.help="Manual retry of the latest RAW. Normal shooting converts automatically."},
  {.name="Photo progress",.update=show_status,.help="An animated film screen appears automatically during development."},
  {.name="Cancel conversion",.select=cancel_convert,.help="Cancel current and waiting conversions. Originals are kept."},MENU_EOL}},
};
static int registered;
unsigned int lutjpeg_abi(void){return 0x4c554933;}
unsigned int lutjpeg_bootstrap(void){
    if(!registered){queue_sem=create_named_semaphore("film_queue",SEM_CREATE_UNLOCKED);if(!queue_sem)return 1;live_sem=create_named_semaphore("film_live",SEM_CREATE_UNLOCKED);if(!live_sem)return 1;film_settings(0);last_record(0);stopping=0;register_film_ui();observe_shot();menu_add("Shoot",entries,COUNT(entries));registered=1;
#ifndef LUTJPEG_HOST_TEST
        monitor_alive=1;task_create("film_monitor",0x1e,0x4000,monitor_task,NULL);
#endif
    }return 0;
}
unsigned int lutjpeg_shutdown(void){
    stopping=cancel=1;live_ready=0;
#ifndef LUTJPEG_HOST_TEST
    photo_film_register(NULL);
#endif
    while(running || monitor_alive){msleep(20);}
    take_semaphore(live_sem,0);if(live_table){free(live_table);live_table=NULL;}give_semaphore(live_sem);
#ifndef LUTJPEG_HOST_TEST
    BMP_LOCK(photo_film_palette_release();if(review_pixels){free(review_pixels);review_pixels=NULL;}if(trial_art){free(trial_art);trial_art=NULL;});
#else
    if(review_pixels){free(review_pixels);review_pixels=NULL;}if(trial_art){free(trial_art);trial_art=NULL;}
#endif
    if(registered){menu_remove("Shoot",entries,COUNT(entries));registered=0;}return 0;
}
MODULE_INFO_START()
 MODULE_INIT(lutjpeg_bootstrap)
 MODULE_DEINIT(lutjpeg_shutdown)
MODULE_INFO_END()
