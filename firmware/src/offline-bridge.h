/* Menu-only interface. No camera event, display-filter or capture callbacks. */
#ifndef OFFLINE_BRIDGE_H
#define OFFLINE_BRIDGE_H
struct offline_hooks {int (*draw)(void);int (*key)(int);};
void photo_offline_register(const struct offline_hooks *h);
int photo_offline_allowed(void);
void photo_offline_pixels(const unsigned char *p,int x,int y,int w,int h);
#endif
