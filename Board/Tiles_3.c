
#include "Tile.h"

VOID setTile (CTile *p, UWORD kind, UWORD sub, UWORD floor, UWORD flags)
{
    p->Kind    = kind;
    p->SubKind = sub;
    p->Floor   = floor;
    p->Flags   = flags | BOTH_REDRAW;
}

/* Usuï obiekt lub przedmiot */

VOID removeObject (CTile *p)
{
    p->Kind     = TK_FLOOR;
    p->SubKind  = p->Floor;
    p->Flags   |= BOTH_REDRAW;
}

VOID setSimpleTile (CTile *p, UWORD simple)
{
    switch (simple)
    {
        case SIMPLE_WALL:
            setWall(p);
            break;

        case SIMPLE_FLOOR:
            setFloor(p);
            break;

        case SIMPLE_BOX:
            setTile(p, TK_OBJECT, OSK_BOX, FSK_FLOOR, 0);
            break;

        case SIMPLE_FLAGSTONE:
            setTile(p, TK_FLOOR, FSK_FLAGSTONE, FSK_FLAGSTONE, 0);
            break;

        case SIMPLE_MUD:
            setTile(p, TK_FLOOR, FSK_MUD, FSK_MUD, 0);
            break;
    }
}
