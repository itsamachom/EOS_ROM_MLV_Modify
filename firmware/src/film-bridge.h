/* Core/film-overlay interface; GPL-2.0-or-later. No module loader callbacks. */
#ifndef FILM_BRIDGE_H
#define FILM_BRIDGE_H
#include <stdint.h>
struct film_hooks {
    int (*key)(int);
    int (*draw)(int);
    int (*filter_ready)(void);
    void (*filter)(const uint32_t *,uint32_t *,unsigned);
};
void photo_film_register(const struct film_hooks *hooks);
void photo_film_mode(int mode);
int photo_film_key(int key);
int photo_film_draw(void);
int photo_film_filter_active(void);
int photo_film_filter_update(void);
void photo_film_review_pixels(const unsigned char *pixels,const unsigned char *palette);
void photo_film_palette_release(void);
void photo_film_art_pixels(const unsigned char *pixels,const unsigned char *palette,int x,int y,int w,int h);
#endif
