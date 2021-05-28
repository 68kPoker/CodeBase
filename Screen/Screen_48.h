
#include <exec/types.h>

#define COPLINE 16

struct copperData
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct BitMap *allocBitMap(void);
struct Screen *openScreen(struct BitMap *bm, struct TextFont **tf);
BOOL addCopperInt(struct Interrupt *is, struct copperData *cd, struct ViewPort *vp);
void remCopperInt(struct Interrupt *is);
BOOL addCopperList(struct ViewPort *vp);
