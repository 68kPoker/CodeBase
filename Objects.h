
#ifndef OBJECTS_H
#define OBJECTS_H

#include <exec/nodes.h>

enum
{
    OT_BOX,
    OT_HERO,
    OT_ITEM,
    OT_COUNT
};

/* Item subTypes */
enum
{
    ITEM_GOLD,
    ITEM_FRUITS,
    ITEM_COUNT
};

struct Object
{
    struct Node node;
    WORD x, y; /* Coordinates */
    WORD type;
    UBYTE subType, subType2;
    UWORD identifier;
    struct ObjectState
    {
        WORD state;
        WORD prevX, prevY; /* Previous position */
        BOOL updated; /* Is position updated? */
    } state;
};

#endif /* OBJECTS_H */
