
#ifndef MAIN_H
#define MAIN_H

#include <dos/dos.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

extern struct copperData
{
    UWORD vpos;
    struct ViewPort *vp;
    UWORD signal;
    struct Task *task;
} cop_data;

extern BPTR out; /* Output */

extern ULONG *colors; /* RGB */

extern struct BitMap *bitmaps[2], *gfx; /* Graphics */
extern struct Screen *screen;
extern struct Window *window;

extern struct Window *openWindow(WORD left, WORD top, WORD width, WORD height);

#endif /* MAIN_H */
