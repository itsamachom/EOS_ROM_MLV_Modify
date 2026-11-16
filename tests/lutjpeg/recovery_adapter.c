/* Execute the actual module entry: stale .en flags cannot activate film code. */
#define LUTJPEG_HOST_TEST
#include "../../firmware/modules/lutjpeg/lutjpeg.c"
static int FIO_ReadFile(FILE *f,void *p,int n){(void)f;(void)p;(void)n;assert(!"recovery must not read");return -1;}
static int FIO_WriteFile(FILE *f,const void *p,int n){(void)f;(void)p;(void)n;assert(!"recovery must not write");return -1;}
int main(void){
 int before=mock_now;
 assert(lutjpeg_abi()==0x4c554933);
 assert(lutjpeg_bootstrap()!=0 && !registered && !running && !monitor_alive && !qcount);
 assert(mock_now==before && !quality_requests);
 for(int i=0;i<32;i++)assert(!dma_allocs[i].p);
 puts("PASS recovery module rejects initialization without menus, monitoring, conversion, property changes or file I/O");
 return 0;
}
