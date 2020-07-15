
/* Screen and windows */

#ifndef SCRN_WIN_H
#define SCRN_WIN_H

#include "DrawAnim.h"

/* Sources */
enum
{
    SSAFE,
    SDISP,
    SUSER
};

struct screenUser
{
    struct Screen   *s;
    struct Window   *bdw;
    struct BitMap   *bm[2];
    struct DBufInfo *dbi;
    struct MsgPort  *mp[2];
    WORD            frame;
    BOOL            safe[2];
    struct rastPortUser rpu;
};

void animScreen(struct screenUser *su);
BOOL openScreen(struct screenUser *su);
struct Window *openWindow(struct screenUser *su);
void closeScreen(struct screenUser *su);

#endif /* SCRN_WIN_H */
