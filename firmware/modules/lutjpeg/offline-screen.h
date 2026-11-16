/* Shared production offline picker / selection / vertical progress renderer. */
static void draw_text(int x,int y,int w,const char *s){
#ifndef LUTJPEG_HOST_TEST
 photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),x,y,w,s);
#else
 (void)x;(void)y;(void)w;(void)s;
#endif
}
static void number(int x,int y,unsigned n){char b[16];film_uint(b,n);draw_text(x,y,120,b);}
#ifndef LUTJPEG_HOST_TEST
extern void photo_film_text(int,int,int,int,const char*);
static void canister(int x,int y,int index,int w,int h){
 if(art)photo_offline_pixels(art+FILM7_META_BYTES+offline_art[index]*FILM7_CELL_BYTES,x,y,w,h);
 else {bmp_fill(COLOR_GRAY(25),x+w/5,y,w*3/5,h);bmp_fill(COLOR_CYAN,x+w/5,y+h/3,w*3/5,h/3);}
}
static int offline_draw(void){
 if(!mode || stopping)return 0;
 BMP_LOCK(
  bmp_fill(COLOR_BLACK,0,0,720,480);
  draw_text(16,4,688,"RAW to Film Tone");
  if(mode==1){
   for(int p=0;p<3;p++){int i=(film+p+4)%5;canister(32+p*232,76,i,176,242);draw_text(28+p*232,332,218,offline_names[i]);}
   draw_text(20,390,680,"Left/Right: film   SET: choose RAW files");number(620,4,film+1);
  }else if(mode==2){
   draw_text(16,44,500,offline_names[film]);draw_text(510,44,120,"Selected");number(644,44,selected_count());
   int first=cursor/7*7;
   for(int n=first;n<raw_count && n<first+7;n++){
    int y=88+(n-first)*37;char line[160];snprintf(line,sizeof(line),"%s %s",marks[n]?"[X]":"[ ]",raw_files[n]);
    if(n==cursor)bmp_fill(COLOR_GRAY(15),12,y,696,35);draw_text(20,y,680,line);
   }
   if(!raw_count)draw_text(20,120,680,busy?"Reading RAW files":"No CR2 files found in DCIM");
   draw_text(16,390,688,"SET: mark photo   Q: convert selected photos");
  }else{
   canister(32,82,job_film,200,270);draw_text(244,46,120,"This photo");
   bmp_fill(COLOR_GRAY(20),260,90,48,260);if(percent>0){bmp_fill(COLOR_CYAN,260,350-percent*260/100,48,percent*260/100);}
   number(252,362,percent);draw_text(310,362,36,"%");
   draw_text(360,88,340,offline_names[job_film]);draw_text(360,140,160,"Total photos");number(580,140,total);
   draw_text(360,188,160,"Saved");number(580,188,done);draw_text(360,236,160,"Failed");number(580,236,failed);
   draw_text(360,284,160,"Warnings");number(580,284,warnings);draw_text(16,400,688,last_file);
  }
  draw_text(16,438,688,status);
 );return 1;
}
#else
static int offline_draw(void){return mode!=0;}
#endif
