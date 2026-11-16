#ifndef LUTJPEG_CAMERA_FIXTURE_H
#define LUTJPEG_CAMERA_FIXTURE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#define O_RDONLY 0
#define O_SYNC 0
#define MIN(a,b) ((a)<(b)?(a):(b))
#define COERCE(a,b,c) ((a)<(b)?(b):(a)>(c)?(c):(a))
#define COUNT(a) (sizeof(a)/sizeof((a)[0]))
#define RECORDING 0
struct card_info {char *drive_letter;int file_number;};
static struct card_info card={"A",1};
static struct {int job_state;} lens_info;
static int mock_now=1000;
static int get_ms_clock(void){return mock_now;}
static void msleep(int n){mock_now+=n;}
static struct card_info *get_shooting_card(void){return &card;}
static int get_halfshutter_pressed(void){return 0;}
static int is_movie_mode(void){return 0;}
static const char *get_dcim_dir(void){return "A:/DCIM/100CANON";}
static const char *get_file_prefix(void){return "IMG_";}
static int FIO_GetFileSize(const char *p,uint32_t *n){struct stat s;if(stat(p,&s))return -1;*n=s.st_size;return 0;}
static int is_file(const char *p){uint32_t n;return !FIO_GetFileSize(p,&n);}
static void FIO_CreateDirectory(const char *p){mkdir(p,0700);}
static int fail_preview_file,fail_optional_print;
static int valid83(const char *p){const char *s=strrchr(p,'/');s=s?s+1:p;const char *dot=strrchr(s,'.');return dot?(dot-s<=8 && strlen(dot+1)<=3):strlen(s)<=8;}
static int allow_long_names;
static FILE *FIO_CreateFile(const char *p){if((!allow_long_names && !valid83(p)) || (fail_preview_file && strstr(p,".FPV")) || (fail_optional_print && strstr(p,".PBM")))return NULL;return fopen(p,"wb");}
static FILE *FIO_OpenFile(const char *p,int f){(void)f;return fopen(p,"rb");}
static int FIO_ReadFile(FILE *f,void *p,int n);
static int FIO_WriteFile(FILE *f,const void *p,int n);
static long FIO_SeekSkipFile(FILE *f,int n,int w){return fseek(f,n,w)?-1:ftell(f);}
static void FIO_CloseFile(FILE *f){fclose(f);}
static int fail_rename,fail_companion_rename;
static int FIO_RenameFile(const char *a,const char *b){assert(!is_file(b));if(fail_rename || (fail_companion_rename && strstr(a,".FJ6"))){FILE *f=fopen(b,"wb");if(f)fclose(f);return -1;}return rename(a,b);}
static int FIO_RemoveFile(const char *p){return remove(p);}
#define tmp_malloc malloc
static struct {void *p;size_t n;} dma_allocs[32];
static void *test_fio_malloc(size_t n){void *p=malloc(n);if(p)for(int i=0;i<32;i++)if(!dma_allocs[i].p){dma_allocs[i].p=p;dma_allocs[i].n=n;return p;}return p;}
static int test_is_dma(const void *p,unsigned n){for(int i=0;i<32;i++)if(p==dma_allocs[i].p && n<=dma_allocs[i].n)return 1;return 0;}
static void test_fio_free(void *p){for(int i=0;i<32;i++)if(dma_allocs[i].p==p){dma_allocs[i].p=NULL;free(p);return;}assert(!p);}
#define fio_malloc test_fio_malloc
#define fio_free test_fio_free
#define NotifyBox(n,...) ((void)(n))
static void menu_redraw(void){}
static void task_create(const char *name,int pri,int stack,void (*fn)(void*),void *arg){(void)name;(void)pri;(void)stack;fn(arg);}
#define MENU_SELECT_FUNC(n) void n(void *priv,int delta)
struct menu_display_info {int unused;};
struct menu_entry {const char *name,*help,*help2;int *priv;int min,max,submenu_width,icon_type;const char **choices;struct menu_entry *children;void (*update)(struct menu_entry*,struct menu_display_info*);void (*select)(void*,int);};
#define MENU_UPDATE_FUNC(n) void n(struct menu_entry *entry,struct menu_display_info *info)
#define MENU_SET_VALUE(...) ((void)0)
#define MENU_SET_RINFO(...) ((void)0)
#define MENU_SET_HELP(...) ((void)0)
#define MENU_SET_HELP2(...) ((void)0)
#define CHOICES(...) ((const char *[]){__VA_ARGS__})
#define IT_SUBMENU 10
#define IT_ACTION 11
static void menu_open_submenu(void *p,int d){(void)p;(void)d;}
#define MENU_EOL {0}
#define menu_add(...) ((void)0)
#define menu_remove(...) ((void)0)
#define MODULE_INFO_START()
#define MODULE_INIT(n)
#define MODULE_DEINIT(n)
#define MODULE_INFO_END()

#define SEM_CREATE_UNLOCKED 1
static void *create_named_semaphore(const char *s,int v){(void)s;(void)v;return (void*)1;}
static void take_semaphore(void *s,int v){(void)s;(void)v;}
static void give_semaphore(void *s){(void)s;}

#define PICQ_RAW_JPG_LARGE_FINE 0x3070000
#define PROP_PIC_QUALITY 1
static int pic_quality=PICQ_RAW_JPG_LARGE_FINE,quality_requests;
static void prop_request_change(unsigned p,const void *v,size_t n){assert(p==PROP_PIC_QUALITY && n==4);pic_quality=*(const int*)v;quality_requests++;}

static int test_film_mode;
void photo_film_mode(int mode){test_film_mode=mode;}
void photo_film_palette_release(void){}
static void gui_stop_menu(void){}

#endif
