
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id$
*/

#ifndef SCREEN_H
#define SCREEN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define DEPTH  5
#define COLORS (1 << DEPTH)

struct screenInfo
{
    struct Screen* s;
    struct BitMap* bm[2];
    struct DBufInfo* dbi;
    struct MsgPort* mp[2];
    BOOL safe[2];
    struct Window* w;
    ULONG* colors;
    UWORD frame;
};

struct Screen* openScreen();
ULONG*         allocColors();

void closeScreen(struct Screen* s);

#endif /* SCREEN_H */
