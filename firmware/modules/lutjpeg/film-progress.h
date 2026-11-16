/* Production animated film background/status renderer; GPL-2.0-or-later. */
static void film_progress_draw(int now,int active,int draw_stage,int percent,const char *name,const char *message,int queued,int done,int width,int height,int full_size,const char *source,const char *target,int failures)
{
    static const char *const descriptions[]={"Preparing colours","Reading your RAW photo","Balancing light and colour","Saving your JPEG","Photo ready","Conversion stopped","Making a larger JPEG"};
    const char *description=active?descriptions[COERCE(draw_stage,0,6)]:message;
    bmp_fill(COLOR_BLACK,0,0,720,480);
    /* Long film strip with moving sprocket holes, behind the status panel. */
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,54,688,source);
    bmp_fill(COLOR_GRAY(10),0,92,720,300);
    int offset=(now/80)%48;
    for(int x=-48+offset;x<720;x+=48){int left=x<0?0:x,right=x+22>720?720:x+22;if(right>left){bmp_fill(COLOR_GRAY(50),left,84,right-left,14);bmp_fill(COLOR_GRAY(50),left,369,right-left,14);}}
    for(int x=-260+offset*5;x<720;x+=260)if(x>=0 && x+4<=720)bmp_fill(COLOR_GRAY(25),x,108,4,248);
    bmp_fill(COLOR_BLACK,32,136,656,208);
    photo_film_text(FONT(FONT_LARGE,COLOR_WHITE,COLOR_BLACK),24,20,672,active?"Developing your film":percent==100?"Photo ready":"Conversion stopped");
    photo_film_text(FONT(FONT_LARGE,COLOR_YELLOW,COLOR_BLACK),48,150,624,name);
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),48,201,624,description);
    char count[16],line[96];film_uint(count,(unsigned)COERCE(percent,0,100));snprintf(line,sizeof(line),"%s",count);strcat(line,"%  ");
    strcat(line,"Waiting ");film_uint(count,queued<0?0:(unsigned)queued);strcat(line,count);
    strcat(line,"  Saved ");film_uint(count,done<0?0:(unsigned)done);strcat(line,count);
    photo_film_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),48,244,624,line);
    bmp_fill(COLOR_GRAY(20),48,294,624,20);if(percent>0)bmp_fill(COLOR_CYAN,48,294,624*COERCE(percent,0,100)/100,20);
    char failed_count[16];film_uint(failed_count,failures<0?0:(unsigned)failures);
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),24,350,130,"Failed");
    photo_film_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),170,350,100,failed_count);
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),24,386,672,"Last saved file");
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),24,414,672,target[0]?target:"Original RAW is kept unchanged");
    if(!active && width)bmp_printf(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),24,444,"JPEG %dx%d  %s",width,height,full_size?"":"Memory: smaller output");
}
