
#include <exec/types.h>

#define SAFE 0
#define DISP 1

struct DBufStatus
    {
    struct MsgPort *mp[2];
    BOOL safe[2];
    WORD frame;
    };

struct DBufExtInfo
    {
    struct ViewPort *vp;
    struct BitMap *bm[2];
    };

BOOL initDBuf(struct DBufStatus *p);
BOOL safeDBuf(struct DBufStatus *p, WORD type);
BOOL bindDBuf(struct DBufStatus *p, struct DBufInfo *dbi);
VOID freeDBuf(struct DBufStatus *p);
