/* Production animated film background/status renderer; GPL-2.0-or-later. */
static void film_progress_draw(int now,int active,int draw_stage,int percent,const char *name,const char *message,int queued,int done,int width,int height,int full_size)
{
    static const char *const descriptions[]={"Preparing colours","Reading your RAW photo","Balancing light and colour","Saving your JPEG","Photo ready","Conversion stopped","Making a larger JPEG"};
    const char *description=active?descriptions[COERCE(draw_stage,0,6)]:message;
    bmp_fill(COLOR_BLACK,0,0,720,480);
    /* Long film strip with moving sprocket holes, behind the status panel. */
    bmp_fill(COLOR_GRAY(10),0,76,720,316);
    int offset=(now/80)%48;
    for(int x=-48+offset;x<720;x+=48){int left=x<0?0:x,right=x+22>720?720:x+22;if(right>left){bmp_fill(COLOR_GRAY(50),left,84,right-left,14);bmp_fill(COLOR_GRAY(50),left,369,right-left,14);}}
    for(int x=-260+offset*5;x<720;x+=260)if(x>=0 && x+4<=720)bmp_fill(COLOR_GRAY(25),x,108,4,248);
    bmp_fill(COLOR_BLACK,32,136,656,208);
    photo_film_text(FONT(FONT_LARGE,COLOR_WHITE,COLOR_BLACK),24,20,672,active?"Developing your film":percent==100?"Photo ready":"Conversion stopped");
    photo_film_text(FONT(FONT_LARGE,COLOR_YELLOW,COLOR_BLACK),48,150,624,name);
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),48,201,624,description);
    bmp_printf(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),48,244,"%d%%   Queue %d   Done %d",percent,queued,done);
    bmp_fill(COLOR_GRAY(20),48,294,624,20);if(percent>0)bmp_fill(COLOR_CYAN,48,294,624*COERCE(percent,0,100)/100,20);
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),24,408,672,"Original RAW is kept unchanged");
    if(!active && width)bmp_printf(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),24,444,"JPEG %dx%d  %s",width,height,full_size?"":"Memory: smaller output");
}
