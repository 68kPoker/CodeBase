
/* Data types */

#ifndef DATA_H
#define DATA_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif

struct graphics
{
    Object *o;
    struct Window *window;
    struct BitMap *bitmap;
};

BOOL loadGraphics(struct graphics *gfx, STRPTR name, struct window *w);
VOID unloadGraphics(struct graphics *gfx);

#endif /* DATA_H */
