
    /***************************************************************\
    *                                                               *
    *   Cell.h                                                      *
    *                                                               *
    *       Cell manipulation.                                      *
    *                                                               *
    \***************************************************************/

#ifndef CELL_H
#define CELL_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

enum {
    FLOOR_NONE,
    FLOOR_NORMAL,
    FLOOR_FLAGSTONE
};

enum {
    OBJECT_NONE,
    OBJECT_WALL,
    OBJECT_BOX
};

enum drawModes {
    OBJECT,          /* Change object only         */
    OBJECT_AND_FLOOR /* Change floor and an object */
};

struct interface {
    WORD drawMode;
};

typedef WORD floor_t, object_t;
typedef struct Cell {
    floor_t  floor;
    object_t object;
} CELL;

BOOL clearCell( CELL *cell, BOOL full );
BOOL objectPresent( CELL *cell );
void deleteCell( CELL *cell );

object_t clearObject( CELL *cell );
object_t getObject( CELL *cell );

floor_t clearFloor( CELL *cell );
floor_t getFloor( CELL *cell );

object_t setObject( CELL *cell, object_t obj );
floor_t  setFloor( CELL *cell, floor_t floor );

#endif /* CELL_H */
