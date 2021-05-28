
#ifndef TILE_H
#define TILE_H

#include <exec/types.h>

#define FLAG_REDRAW 01
#define BOTH_REDRAW 03

#define setWall(t)  setTile(t, TK_WALL,  WSK_WALL,  FSK_FLOOR, 0)
#define setFloor(t) setTile(t, TK_FLOOR, FSK_FLOOR, FSK_FLOOR, 0);

typedef enum Kind
{
    TK_FLOOR,
    TK_WALL,
    TK_OBJECT,
    TK_ITEM
} EKind;

typedef enum FloorKind
{
    FSK_FLOOR,
    FSK_FLAGSTONE,
    FSK_MUD,
    FSK_FILLED_MUD
} EFloor;

typedef enum WallKind
{
    WSK_WALL,
    WSK_DOOR
} EWall;

typedef enum ObjectKind
{
    OSK_BOX,
    OSK_HERO
} EObject;

typedef enum ItemKind
{
    ISK_FRUITS,
    ISK_KEY
} EItem;

/* Uproszczone typy dla edytora i grafiki */

typedef enum SimpleKinds
{
    SIMPLE_FLOOR,
    SIMPLE_WALL,
    SIMPLE_BOX,
    SIMPLE_FLAGSTONE,
    SIMPLE_MUD
} ESimple;

typedef struct STile
{
    UWORD Kind    : 4,
          SubKind : 4,
          Floor   : 4,
          Flags   : 4;
} CTile;

VOID setTile (CTile *p, UWORD kind, UWORD sub, UWORD floor, UWORD flags);

VOID setSimpleTile (CTile *p, UWORD simple);

VOID removeObject (CTile *p);

#endif /* TILE_H */
