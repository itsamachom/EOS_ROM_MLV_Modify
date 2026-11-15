/* Exercise the actual core loader with a fake linker/filesystem. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define CONFIG_600D 1
#define _module_h_
static int missing_mo,missing_sym,startup_ready=1,load_fail,old_abi,bootstrap_fail,global_loaded;
static int loads,unloads,boots,stops,sleeps;
static int module_startup_complete(void){return startup_ready;}
static int FIO_GetFileSize(const char *path,uint32_t *size){*size=2048;return strstr(path,".mo")?missing_mo:missing_sym;}
static FILE *FIO_CreateFileOrAppend(const char *s){(void)s;return (FILE*)1;}
static void FIO_WriteFile(FILE *f,const void *s,size_t n){assert(f==(FILE*)1);(void)s;(void)n;}
static void FIO_CloseFile(FILE *f){assert(f==(FILE*)1);}
static void msleep(int n){sleeps+=n;}
static void *module_load(char *s){assert(strstr(s,"lutjpeg.mo"));loads++;return load_fail?NULL:(void*)0x2000;}
static unsigned module_get_symbol(void *handle,char *name){(void)name;return (handle || global_loaded)?0x1000:0;}
static int module_exec(void *handle,char *name,int args,...){
 assert(handle || global_loaded);assert(args==0);
 if(!strcmp(name,"lutjpeg_abi"))return old_abi?1:0x4c554932;
 if(!strcmp(name,"lutjpeg_bootstrap")){boots++;return bootstrap_fail?-1:0;}
 assert(!strcmp(name,"lutjpeg_shutdown"));stops++;return 0;
}
static int module_unload(void *h){assert(h==(void*)0x2000);unloads++;return 0;}
static void task_create(const char *s,int p,int z,void (*fn)(void*),void *a){(void)s;(void)p;(void)z;fn(a);}
static void menu_redraw(void){}
#define NotifyBox(...) ((void)0)
#define MENU_SELECT_FUNC(n) void n(void *priv,int delta)
#define MENU_UPDATE_FUNC(n) void n(void)
#define MENU_SET_VALUE(...) ((void)0)
#define MENU_SET_HELP(...) ((void)0)
#include "lutjpeg-loader-600d.h"
static void reset(void){missing_mo=missing_sym=load_fail=old_abi=bootstrap_fail=global_loaded=0;loads=unloads=boots=stops=sleeps=0;startup_ready=1;photo_lut_loading=0;photo_lut_handle=NULL;}
int main(void){
 reset();missing_mo=1;photo_lut_load_select(NULL,0);assert(!loads && strstr(photo_lut_status,"Missing ML/modules/lutjpeg.mo"));
 reset();missing_sym=1;photo_lut_load_select(NULL,0);assert(!loads && strstr(photo_lut_status,"600D_102.sym"));
 reset();startup_ready=0;photo_lut_load_select(NULL,0);assert(!loads && sleeps==5000 && !photo_lut_loading);
 reset();load_fail=1;photo_lut_load_select(NULL,0);assert(loads==1 && !unloads && strstr(photo_lut_status,"load/link failed"));
 reset();old_abi=1;photo_lut_load_select(NULL,0);assert(loads==1 && unloads==1 && !boots && strstr(photo_lut_status,"Old lutjpeg.mo"));
 reset();bootstrap_fail=1;photo_lut_load_select(NULL,0);assert(loads==1 && unloads==1 && boots==1 && !photo_lut_handle);
 reset();photo_lut_load_select(NULL,0);assert(loads==1 && boots==1 && photo_lut_handle && !photo_lut_loading);
 photo_lut_load_select(NULL,0);assert(loads==1 && boots==2);
 photo_lut_shutdown();assert(stops==1);
 reset();global_loaded=1;photo_lut_load_select(NULL,0);assert(!loads && boots==1 && !photo_lut_handle);
 photo_lut_shutdown();assert(!stops);
 puts("PASS direct loader missing files/startup/link/ABI/init failures, reuse, autoload reuse and shutdown");
}
