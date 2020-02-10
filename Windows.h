
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id$
*/

#ifndef WINDOWS_H
#define WINDOWS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

struct windowInfo
{
    struct screenInfo* si;
    UWORD left, top, width, height;
};

void blitBitMap(struct BitMap *bm, WORD srcx, WORD srcy, struct windowInfo *wi, WORD destx, WORD desty, WORD width, WORD height);
void drawBackground(struct windowInfo *wi, struct BitMap *bm, WORD srcleft, WORD srctop, WORD width, WORD height);

#endif /* WINDOWS_H */
