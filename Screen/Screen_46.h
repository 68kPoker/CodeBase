
/* $Header: Work:Magazyn/RCS/Screen.h,v 1.1 12/.0/.1 .2:.4:.0 Robert Exp $ */

#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

#define COMMANDS 3
#define PRI      0

struct copperData
{
    struct ViewPort *vp;
    WORD            signal;
    struct Task     *task;
};

BOOL initBitMaps(struct BitMap *bitmaps[], UWORD rasWidth, UWORD rasHeight, UBYTE rasDepth);
BOOL initFont(struct TextFont **font, struct TextAttr *ta);
ULONG *initPal(UBYTE depth);
struct Screen *openScreen(STRPTR title, struct BitMap *bitmaps[], struct TextAttr *ta, ULONG *pal);
struct DBufInfo *addDBuf(struct ViewPort *vp, BOOL *safe, UWORD *frame);
BOOL addUCopList(struct ViewPort *vp);
BOOL addCopperIs(void (*copperIs)(void), struct Interrupt *is, struct copperData *cd, struct ViewPort *vp);

VOID remCopperIs(struct Interrupt *is);
VOID remDBuf(struct DBufInfo *dbi, BOOL safe);
VOID closeScreen(struct Screen *s);
VOID freePal(ULONG *pal);
VOID closeFont(struct TextFont *font);
VOID freeBitMaps(struct BitMap *bitmaps[]);

#endif /* SCREEN_H */
