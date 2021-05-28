
#ifndef COPPER_H
#define COPPER_H

#include <exec/interrupts.h>

#define COPLINE 0

BOOL addCopperInt(struct Interrupt *is, struct copperData *cd, struct ViewPort *vp);
VOID remCopperInt(struct Interrupt *is);
BOOL addCopperList(struct ViewPort *vp);

struct screenData
{
    struct Interrupt is;
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp;
    UBYTE safe;
    UBYTE frame;
    struct copperData
    {
        struct ViewPort *vp;
        WORD signal;
        struct Task *task;
    } cd;
    struct TextFont *font;
    struct Region *update[2];
};

extern void myCopper();

#endif /* COPPER_H */
