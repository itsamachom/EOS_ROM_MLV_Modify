/* Shared production seven-film picker renderer. */
static void count_text(int x,int y,unsigned n){char b[16];film_uint(b,n);photo_film_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),x,y,130,b);}
static void film_can(int x,int y,int index,int selected){
    static const int colors[]={COLOR_GREEN1,COLOR_CYAN,COLOR_ORANGE,COLOR_GRAY(55),COLOR_YELLOW,COLOR_BLUE,COLOR_GREEN2};
    int c=colors[index],w=194,h=260;
    bmp_fill(selected?COLOR_GRAY(20):COLOR_GRAY(5),x,y,w,h);
    if(trial_art){
        photo_film_art_pixels(trial_art+FILM7_META_BYTES+index*FILM7_CELL_BYTES,trial_palette,x+7,y+10,180,214);
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),x+6,y+232,w-12,film_names[index]);return;
    }
    bmp_fill(COLOR_GRAY(60),x+47,y+16,100,12);bmp_fill(COLOR_GRAY(20),x+38,y+28,118,190);
    bmp_fill(COLOR_GRAY(50),x+38,y+28,8,190);bmp_fill(c,x+46,y+63,110,114);
    bmp_fill(COLOR_BLACK,x+46,y+176,110,28);bmp_fill(COLOR_GRAY(60),x+38,y+218,118,8);
    photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),x+6,y+232,w-12,film_names[index]);
    char no[16];film_uint(no,index+1);photo_film_text(FONT(FONT_LARGE,COLOR_BLACK,c),x+86,y+104,60,no);
}
static void film_picker_draw(int browsing,int selected){
    bmp_fill(COLOR_BLACK,0,0,720,480);
        photo_film_text(FONT(FONT_LARGE,COLOR_WHITE,COLOR_BLACK),16,6,688,"Choose one film");
        count_text(624,8,browsing+1);photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),660,8,50,"/7");
        for(int pos=0;pos<3;pos++){int index=(browsing+pos+6)%7;film_can(28+pos*234,pos==1?76:98,index,pos==1);}
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,372,200,"Active film");
        photo_film_text(FONT(FONT_MED,COLOR_YELLOW,COLOR_BLACK),240,372,464,film_names[selected]);
        photo_film_text(FONT(FONT_MED,COLOR_CYAN,COLOR_BLACK),16,408,688,"Left/Right: film   SET: use   PLAY: result");
        photo_film_text(FONT(FONT_MED,COLOR_WHITE,COLOR_BLACK),16,446,688,"MENU: back   INFO: photo status");
}
