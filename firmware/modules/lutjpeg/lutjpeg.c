/* Offline RAW file converter. GPL-2.0-or-later; external recipe assets retain their licenses. */
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
#include "upstream/fos.h"
#include "upstream/fos_dev.h"
#include "../../src/offline-bridge.h"
#include "film-colour.h"
#ifndef LUTJPEG_HOST_TEST
#undef BGMT_Q
#include "../../platform/600D.102/gui.h"
#endif
static unsigned char *bounce;
static int local_errno;
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


#include "offline-converter.h"
