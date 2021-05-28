
#include "vport.h"

struct screenInfo
    {
    struct Screen *s;
    struct TextFont *font;
    struct ScreenBuffer *sb[2];
    struct DBufStatus dbuf;
    };

BOOL initSBuf(struct ScreenBuffer *sb[], struct DBufStatus *dbs, struct Screen *s);
VOID freeSBuf(struct ScreenBuffer *sb[], struct DBufStatus *dbs, struct Screen *s);

BOOL openScreen(struct screenInfo *si, struct ILBMInfo *ii);
VOID closeScreen(struct screenInfo *si);

VOID changeScreen(struct screenInfo *si);
