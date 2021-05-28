
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/interrupts.h>
#include <graphics/gfx.h>

#define COP_PRI  0 /* Copper interrupt server priority */
#define COP_LINE 0 /* Copper interrupt vpos */

/* Gathered from graphics file */
struct graphics
{
    ULONG *pal;
    ULONG modeID;
    struct Rectangle dclip;
    struct BitMap *bm;
};

/* Info for the copper interrupt server */
struct copper
{
    struct ViewPort *vp; /* Enabled on this ViewPort */
    WORD signal; /* Signal to send */
    struct Task *task;
};

/* Extended screen information */
struct screen
{
    struct BitMap *bm[2];
    struct Screen *s;
    struct DBufInfo *dbi;
    BOOL safe; /* Safe to draw? */
    UWORD frame; /* Current hidden frame */
    struct graphics *gfx;
    struct Interrupt is;
    struct copper cop;
};

BOOL getGraphics(struct graphics *gfx, STRPTR name);
void freeGraphics(struct graphics *gfx);

struct Screen *openScreen(struct screen *s, UWORD w, UWORD h, UBYTE d, struct graphics *gfx, STRPTR gfxName);
void closeScreen(struct screen *s);

#endif /* SCREEN_H */
