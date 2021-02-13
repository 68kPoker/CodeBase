
/* Position, directions, size */

#ifndef POS_H
#define POS_H

#include <exec/types.h>

/* Position */
struct Pos
{
    UWORD x, y;
};

/* Directions */
enum
{
    LEFT,
    RIGHT,
    UP,
    DOWN
};

/* Size */
struct Size
{
    UWORD w, h;
};

/* Directions offsets */
WORD *getOffsets(UWORD w);

#endif /* POS_H */
