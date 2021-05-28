
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/interrupts.h>
#include <graphics/gfx.h>

#define DEPTH 5

struct copper
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screen
{
    struct Screen *s;
    struct BitMap *bm[2]; /* Custom bitmap(s) */
    struct TextFont *tf;
    struct screenParam *sp; /* Optional */

    struct Interrupt copIs;
    struct copper cop;
    struct DBufInfo *dbi;
    struct MsgPort *safePort;
    BOOL safe;
    WORD frame;
};

/* Optional parameters */
struct screenParam
{
    struct Rectangle dclip;
    ULONG modeID;
    UWORD depth;
    struct TextAttr *ta;
    ULONG *colors;
};

LONG openScreen(struct screen *s);
void closeScreen(struct screen *s);

void safeToDraw(struct screen *s);
void changeScreen(struct screen *s);

#endif /* SCREEN_H */
