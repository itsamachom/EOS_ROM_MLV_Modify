/* Shared actual camera-key handling for the seven-film UI. */
static int film_ui_key(int key){
    if(key==BGMT_PLAY){
        if(ui_mode==2){close_film_ui(1);return 1;}
        if(last_saved[0] && (review_pixels || last_preview[0])){pending_view=1;return 0;}
    }
    if(!ui_mode)return 1;
    if(key==BGMT_PRESS_HALFSHUTTER){close_film_ui(1);return 1;}
    if(key==BGMT_MENU){close_film_ui(0);return 0;}
    if(key==BGMT_INFO){ui_mode=3;photo_film_mode(3);return 0;}
    if(ui_mode==1){
        if(key==BGMT_PRESS_LEFT || key==BGMT_WHEEL_LEFT)browsing=(browsing+6)%7;
        if(key==BGMT_PRESS_RIGHT || key==BGMT_WHEEL_RIGHT)browsing=(browsing+1)%7;
        if(key==BGMT_PRESS_SET){within=browsing;live_ready=0;pending_preview=1;film_settings(1);close_film_ui(1);}
        menu_redraw();
    }return 0;
}
