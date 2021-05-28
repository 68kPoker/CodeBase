
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

#define BUFFER_LEN 40

struct screenData
{
    struct TextAttr ta;
    struct TextFont *font;
    ULONG *colors;
    struct BitMap *bm[2];
    struct Screen *s;
    struct MsgPort *mp[2];
    struct ScreenBuffer *sb[2];
    BOOL safe[2];
    WORD frame;
};

struct textInfo
{
    UBYTE text[BUFFER_LEN + 1];
    WORD x, y;
};

struct animationData
{
    BOOL updated; /* Update? */
    BOOL prevupdated; /* Updated in previous frame? */
    APTR info;
    void (*draw)(struct animationData *ad, struct RastPort *rp);
};

BOOL getScreenData(struct screenData *sd, UWORD width, UWORD height, UBYTE depth, ULONG modeID);
void dropScreenData(struct screenData *sd);

void drawText(struct animationData *ad, struct RastPort *rp);

#endif /* SCREEN_H */
