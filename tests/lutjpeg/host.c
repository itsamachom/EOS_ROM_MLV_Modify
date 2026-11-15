/* Production processing core with host FIO/memory adapters; GPL-2.0-or-later. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "fos.h"
#include "fos_dev.h"
static size_t used,peak,limit=8*1024*1024;
int fos_port_ms(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000+t.tv_nsec/1000000;}
int fos_port_dev_cap(void){return 64;}
const char *fos_port_model(void){return "Canon EOS 600D";}
void *fos_port_alloc(int n){if(n<=0 || used+(size_t)n>limit)return NULL;size_t *p=malloc(n+sizeof(size_t));if(!p)return NULL;*p=n;used+=n;if(used>peak)peak=used;return p+1;}
void fos_port_free(void *v){if(v){size_t *p=(size_t*)v-1;used-=*p;free(p);}}
void fos_port_log(const char *s){fprintf(stderr,"%s\n",s);}
void *fos_port_open(const char *s,int w){return fopen(s,w?"wb":"rb");}
int fos_port_read(void *f,void *p,int n){return fread(p,1,n,f);}
int fos_port_write(void *f,const void *p,int n){return fwrite(p,1,n,f);}
int fos_port_seek(void *f,int n){return fseek(f,n,SEEK_SET);}
void fos_port_close(void *f){fclose(f);}
#ifdef FOS_DEV_TEST
void fos_dev_test_sample(int x,int y,int v){(void)x;(void)y;(void)v;}
#endif
int main(int argc,char **argv){
#ifdef FOS_DEV_TEST
 if(argc==3 && !strcmp(argv[1],"-T")){
  const int films[]={0,2,7};fos_dev_cache(argv[2]);
  for(int i=0;i<3;i++)if(!fos_dev_table(films[i]))return 2;
  fos_dev_release();assert(used==0);return 0;
 }
#endif
 assert(argc==6);
 if(getenv("LUT_TEST_MEM"))limit=atoi(getenv("LUT_TEST_MEM"));
 int film=atoi(argv[1]);fos_dev_cache(argv[2]);
 struct fos_dev *d=fos_dev_start(argv[3],argv[4],film,1,"","");
 if(!d){assert(used==0);return 2;}fos_dev_jpeg(d,argv[5]);
 int p=0,steps=0;while(p>=0 && p<1000 && steps++<100000)p=fos_dev_step(d,10);
 if(p<0)fprintf(stderr,"Error: %s\n",fos_dev_error(d));
 int ok=p==1000;fos_dev_free(d);fos_dev_release();assert(used==0);
 fprintf(stderr,"Host peak processing allocation: %zu bytes; %d steps\n",peak,steps);
 return ok?0:2;
}
