
#include "Pos.h"

/* Directions offsets */
WORD *getOffsets(UWORD w)
{
    static WORD offsets[4];

    offsets[LEFT]  = -1;
    offsets[RIGHT] =  1;
    offsets[UP]    = -w;
    offsets[DOWN]  =  w;

    return offsets;
}
