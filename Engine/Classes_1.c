
#include <stdio.h>

#include "Classes.h"

WORD keys = 0, placedBoxes = 0;
WORD enteringObject = OBJECT_HERO;
struct Field *nextField;
struct Board board;

WORD dirx = 0, diry = 0;

void displayScroll()
{
    printf("This is example scroll.\n");
}

enterField( WORD object, struct Field *enteredField )
{
    WORD result;

    enteringObject = object;
    nextField = enteredField + (diry * BOARD_WIDTH) + dirx;

    switch( enteredField->root )
    {
        case FLOOR:
            switch( enteredField->sub )
            {
                case FLOOR_FLAGSTONE: return(handleFlagstone( METHOD_ENTER ));
            }
            break;
        case WALL:
            switch( enteredField->sub )
            {
                case WALL_SOLID: return(handleSolid( METHOD_ENTER ));
                case WALL_DOOR: return(handleDoor( METHOD_ENTER ));
            }
            break;
        case OBJECT:
            switch( enteredField->sub )
            {
                case OBJECT_BOX: return(handleBox( METHOD_ENTER ));
            }
            break;
    }
    return( RESULT_NONE );
}

leaveField( WORD object, struct Field *leftField )
{
    enteringObject = object;

    switch( leftField->floor )
    {
        case FLOOR_FLAGSTONE: return(handleFlagstone( METHOD_LEAVE ));
    }
    return( RESULT_NONE );
}

checkPush( WORD object )
{
    WORD prevEntering = enteringObject, result;

    result = enterField( object, nextField );

    enteringObject = prevEntering;
    return( result );
}

handleFloor( WORD m )
{
    return( RESULT_NONE );
}

handleWall( WORD m )
{
    return( RESULT_NONE );
}

handleObject( WORD m )
{
    return( RESULT_NONE );
}

handleNormal( WORD m )
{
    return( RESULT_OK );
}

handleFlagstone( WORD m )
{
    extern WORD enteringObject, placedBoxes;

    if( enteringObject == OBJECT_BOX )
    {
        if( m == METHOD_ENTER )
        {
            placedBoxes++;
        }
        else if( m == METHOD_LEAVE )
        {
            placedBoxes--;
        }
    }
    return( RESULT_OK );
}

handleSolid( WORD m )
{
    return( RESULT_BLOCK );
}

handleDoor( WORD m )
{
    extern WORD enteringObject, keys;

    if( enteringObject == OBJECT_HERO && keys > 0 )
    {
        keys--;
        return( RESULT_OK );
    }
    return( RESULT_BLOCK );
}

handleBox( WORD m )
{
    extern WORD enteringObject, checkPush( WORD object );

    if( enteringObject == OBJECT_HERO )
    {
        return( checkPush( OBJECT_BOX ) );
    }
    return( RESULT_BLOCK );
}

handleHero( WORD m )
{
    return( RESULT_BLOCK );
}

handleScroll( WORD m )
{
    extern void displayScroll();

    displayScroll();
    return( RESULT_OK );
}

handleKey( WORD m )
{
    extern WORD keys;

    keys++;
    return( RESULT_OK );
}
