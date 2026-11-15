"""Compile the real startup branch: crash recovery must list without loading."""
from pathlib import Path
import tempfile,subprocess
root=Path(__file__).resolve().parents[1];source=(root/'firmware/src/module.c').read_text()
body=source.split('static void module_load_task(void* unused) \n{',1)[1].split('    /* main loop, also wait until clean shutdown */',1)[0]
pre=r'''
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
static int module_autoload_disabled,module_ignore_crashes,module_startup_done;
static int lock_exists,mode=-1,lists,updates,created,notices;
static const char *module_lockfile="ML/modules/LOADING.LCK";
static int FIO_GetFileSize(const char *f,uint32_t *s){(void)f;*s=0;return lock_exists?0:-1;}
static FILE *FIO_CreateFile(const char *f){(void)f;created++;return (FILE*)1;}
static int FIO_WriteFile(FILE *f,const void *p,int n){(void)p;assert(f==(FILE*)1);return n;}
static void FIO_CloseFile(FILE *f){assert(f==(FILE*)1);}
static void msleep(int n){(void)n;}
#define NotifyBox(...) notices++
static void _module_load_all(int m){lists++;mode=m;}
static void module_menu_update(void){updates++;}
'''
main=r'''
int main(void){
 lock_exists=1;startup();assert(lists==1 && updates==1 && mode==1 && !created && notices==1 && module_startup_done);
 lists=updates=notices=module_startup_done=0;module_autoload_disabled=1;startup();assert(lists==1 && mode==1 && !created && module_startup_done);
 lists=updates=module_startup_done=0;module_autoload_disabled=0;lock_exists=0;startup();assert(lists==1 && mode==0 && created==1 && module_startup_done);
 lists=updates=module_startup_done=0;lock_exists=1;module_ignore_crashes=1;startup();assert(lists==1 && mode==0 && created==2 && module_startup_done);
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(pre+'\nstatic void startup(void){\n'+body+'\n}\n'+main)
 subprocess.run(['gcc','-Wall','-Wextra','-Werror',p/'test.c','-o',p/'test'],check=True)
 subprocess.run([p/'test'],check=True)
 subprocess.run(['gcc','-O1','-g','-fsanitize=address,undefined','-I'+str(root/'firmware/src'),str(root/'tests/lutjpeg/loader.c'),'-o',str(p/'loader')],check=True)
 import os
 subprocess.run([p/'loader'],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
print('PASS crash guard preserves read-only module listing and startup-ready state')
