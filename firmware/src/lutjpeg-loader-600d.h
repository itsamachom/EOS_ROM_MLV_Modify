/* 600D manual LUT loader. GPL-2.0-or-later; included by shoot.c.
 * Always available even when startup module scanning was skipped. */
#ifdef CONFIG_600D
#include "module.h"
extern int module_startup_complete(void);
static volatile int photo_lut_loading;
static void *photo_lut_handle;
static char photo_lut_status[96]="SET: Load LUT engine directly";
#define PHOTO_LUT_ABI 0x4c554932
static void photo_lut_log(const char *message) {
    FILE *f=FIO_CreateFileOrAppend("ML/LUTLOAD.LOG");
    if(f){FIO_WriteFile(f,message,strlen(message));FIO_WriteFile(f,"\n",1);FIO_CloseFile(f);}
}
static void photo_lut_load_task(void *unused) {
    (void)unused;uint32_t size=0;
    photo_lut_log("Thai-Menu LUT2: manual load requested");
    int waited=0;
    while(!module_startup_complete() && waited<5000){msleep(50);waited+=50;}
    if(!module_startup_complete()){snprintf(photo_lut_status,sizeof(photo_lut_status),"Module startup busy; retry");goto done;}
    if(FIO_GetFileSize("ML/modules/lutjpeg.mo",&size) || size<1024){
        snprintf(photo_lut_status,sizeof(photo_lut_status),"Missing ML/modules/lutjpeg.mo; copy full ML folder");goto done;
    }
    if(FIO_GetFileSize("ML/modules/600D_102.sym",&size) || size<1024){
        snprintf(photo_lut_status,sizeof(photo_lut_status),"Missing ML/modules/600D_102.sym; copy full ML folder");goto done;
    }
    void *handle=photo_lut_handle;
    int owned=0;
    /* Reuse an already linked instance; never initialize the same menu twice. */
    if(!handle && !module_get_symbol(NULL,"lutjpeg_bootstrap")){
        char filename[]="ML/modules/lutjpeg.mo";
        handle=module_load(filename);owned=1;
        if(!handle){snprintf(photo_lut_status,sizeof(photo_lut_status),"LUT load/link failed; see ML/LUTLOAD.LOG and console");goto done;}
    }
    if(!module_get_symbol(handle,"lutjpeg_abi") || module_exec(handle,"lutjpeg_abi",0)!=PHOTO_LUT_ABI){
        snprintf(photo_lut_status,sizeof(photo_lut_status),"Old lutjpeg.mo; copy LUT2 autoexec.bin and ML together");
        if(owned)module_unload(handle);
        goto done;
    }
    if(module_exec(handle,"lutjpeg_bootstrap",0)!=0){
        snprintf(photo_lut_status,sizeof(photo_lut_status),"LUT initialization failed");
        if(owned)module_unload(handle);
        goto done;
    }
    if(owned)photo_lut_handle=handle;
    snprintf(photo_lut_status,sizeof(photo_lut_status),"LUT ready; select S > LUT JPEG again");
 done:
    photo_lut_log(photo_lut_status);
    NotifyBox(6000,"%s",photo_lut_status);
    photo_lut_loading=0;menu_redraw();
}
static MENU_SELECT_FUNC(photo_lut_load_select) {
    (void)priv;(void)delta;
    if(photo_lut_loading)return;
    photo_lut_loading=1;
    snprintf(photo_lut_status,sizeof(photo_lut_status),"Loading LUT engine...");
    task_create("lut_loader",0x1c,0x4000,photo_lut_load_task,NULL);
}
static MENU_UPDATE_FUNC(photo_lut_load_update) {
    MENU_SET_VALUE("%s",photo_lut_loading?"Loading":"SET Load");
    MENU_SET_HELP("%s",photo_lut_status);
}
/* Independent instances are not in the autoload module list. Close them at shutdown. */
void photo_lut_shutdown(void) {
    while(photo_lut_loading)msleep(20);
    if(photo_lut_handle)module_exec(photo_lut_handle,"lutjpeg_shutdown",0);
}
#endif
