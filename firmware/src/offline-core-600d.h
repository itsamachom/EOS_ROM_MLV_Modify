#include "offline-bridge.h"
static struct offline_hooks offline_hooks;
void photo_offline_register(const struct offline_hooks *h){take_semaphore(menu_sem,0);if(h)offline_hooks=*h;else memset(&offline_hooks,0,sizeof(offline_hooks));give_semaphore(menu_sem);}
void photo_offline_pixels(const unsigned char *p,int x,int y,int w,int h){
 if(!p || x<0 || y<0 || w<1 || h<1 || x+w>720 || y+h>480)return;
 unsigned char *v=bmp_vram();
 for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)
  bmp_putpixel_fast(v,x+xx,y+yy,p[(yy*194/h)*176+xx*176/w]);
}
