"""Execute the production shoot initializer with small setup stubs."""
from pathlib import Path
import tempfile,subprocess
root=Path(__file__).resolve().parents[1]
s=(root/'firmware/src/shoot.c').read_text();body=s.split('static void shoot_init()\n{',1)[1].split('\n}\n\nINIT_FUNC("shoot", shoot_init);',1)[0]
pre='''#include <assert.h>
#define CONFIG_600D 1
#define SEM_CREATE_UNLOCKED 1
#define COUNT(a) (sizeof(a)/sizeof(a[0]))
static int set_maindial_sem,menus,starts;
static int shoot_menus[]={1},expo_menus[]={2};
static int create_named_semaphore(const char *n,int v){(void)n;return v;}
static void menu_add(const char *n,int *p,int count){(void)n;(void)p;assert(count==1);menus++;}
static void photo_lut_auto_start(void){assert(menus==2);starts++;}
'''
with tempfile.TemporaryDirectory() as t:
 p=Path(t);(p/'test.c').write_text(pre+'static void boot(void){'+body+'\n}\nint main(void){boot();assert(starts==1 && set_maindial_sem==1);}\n')
 subprocess.run(['gcc','-Wall','-Wextra','-Werror',p/'test.c','-o',p/'test'],check=True);subprocess.run([p/'test'],check=True)
print('PASS production boot reaches film startup policy after menu registration; offline policy checked by loader tests')
