
    /***************************************************************\
    *                                                               *
    *   Cell.c                                                      *
    *                                                               *
    *       Cell manipulation.                                      *
    *                                                               *
    \***************************************************************/

#include "Cell.h"

    /***************************************************************\
    *                                                               *
    *   clearCell()                                                 *
    *                                                               *
    *       Clear given cell - two-step, or one-step (full).        *
    *       Return FALSE if only object was cleared.                *
    *       TRUE if the floor was set to normal.                    *
    *                                                               *
    \***************************************************************/

BOOL clearCell( CELL *cell, BOOL full )
{
    if( full || objectPresent( cell ) ) {
        clearObject( cell );
        if( !full )
            return( FALSE );
    }
    clearFloor( cell );
    return( TRUE );
}

    /***************************************************************\
    *                                                               *
    *   objectPresent()                                             *
    *                                                               *
    *       Checks if there is an object on the cell.               *
    *                                                               *
    \***************************************************************/

BOOL objectPresent( CELL *cell )
{
    return( cell->object != OBJECT_NONE );
}

    /***************************************************************\
    *                                                               *
    *   clearObject()                                               *
    *                                                               *
    *       Clears (removes) an object from the cell.               *
    *       Returns type of object removed.                         *
    *                                                               *
    \***************************************************************/

object_t clearObject( CELL *cell )
{
    object_t temp = cell->object;

    cell->object = OBJECT_NONE;
    return( temp );
}

    /***************************************************************\
    *                                                               *
    *   clearFloor()                                                *
    *                                                               *
    *       Resets floor to normal.                                 *
    *       Returns previous floor type.                            *
    *                                                               *
    \***************************************************************/

floor_t clearFloor( CELL *cell )
{
    floor_t temp = cell->floor;

    cell->floor = FLOOR_NORMAL;
    return( temp );
}

    /***************************************************************\
    *                                                               *
    *   deleteCell()                                                *
    *                                                               *
    *       Completely removes the cell.                            *
    *                                                               *
    \***************************************************************/

void deleteCell( CELL *cell )
{
    cell->floor = FLOOR_NONE;
    cell->object = OBJECT_NONE;
}

    /***************************************************************\
    *                                                               *
    *   setFloor()                                                  *
    *                                                               *
    *       Sets floor to given type.                               *
    *                                                               *
    \***************************************************************/

floor_t setFloor( CELL *cell, floor_t floor )
{
    floor_t temp = cell->floor;

    cell->floor = floor;
    return( temp );
}

    /***************************************************************\
    *                                                               *
    *   getFloor()                                                  *
    *                                                               *
    *       Gets floor type.                                        *
    *                                                               *
    \***************************************************************/

floor_t getFloor( CELL *cell )
{
    return( cell->floor );
}

    /***************************************************************\
    *                                                               *
    *   setObject()                                                 *
    *                                                               *
    *       Sets object to given type.                              *
    *                                                               *
    \***************************************************************/

object_t setObject( CELL *cell, object_t obj )
{
    object_t temp = cell->object;

    cell->object = obj;
    return( temp );
}

    /***************************************************************\
    *                                                               *
    *   getObject()                                                 *
    *                                                               *
    *       Gets object type.                                       *
    *                                                               *
    \***************************************************************/

object_t getObject( CELL *cell )
{
    return( cell->object );
}

/** EOF **/
