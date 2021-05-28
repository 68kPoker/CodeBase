
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

struct copperInfo
{
    struct ViewPort *vp;
    WORD            signal;
    struct Task     *task;
};

BOOL addUserCopList(struct Screen *s);
BOOL addCopperInterrupt(struct ViewPort *vp, struct Interrupt *is, struct copperInfo *ci);
void remCopperInterrupt(struct Interrupt *is);

#endif /* SCREEN_H */
