
/* Engine */

#include "Board.h"
#include "DBufScreen.h"

TILE getTile( struct Board *board, COORDS coords )
{
    return( board->tileBoard[ coords.y ][ coords.x ] );
}

TILE *getTilePtr( struct Board *board, COORDS coords )
{
    return( &board->tileBoard[ coords.y ][ coords.x ] );
}

COORDS getSibling( COORDS coords, DIFF diff )
{
    COORDS destCoords = { coords.x + diff.dx, coords.y + diff.dy };

    return( destCoords );
}

DIFF getDir( WORD subType )
{
    DIFF dir;

    switch( subType ) {
        case UP:
            dir.dx = 0;
            dir.dy = -1;
            break;
        case RIGHT:
            dir.dx = 1;
            dir.dy = 0;
            break;
        case DOWN:
            dir.dx = 0;
            dir.dy = 1;
            break;
        case LEFT:
            dir.dx = -1;
            dir.dy = 0;
            break;
    }
    return( dir );
}

void setTile( struct Board *board, COORDS coords, WORD type, WORD subType )
{
    TILE *ptr = getTilePtr( board, coords );

    ptr->type = type;
    ptr->subType = subType;

    if (coords.y > 0 && coords.y < 15)
        addBlit(board->blits, DRAWICON, tileToIcon(*ptr), coords.x, coords.y, 16, 16);
}

/* Box movement */
LONG boxMove( struct Board *board, COORDS coords, DIFF diff, BOOL pushed )
{
    BOOL moved = FALSE;
    COORDS destCoords = getSibling( coords, diff );
    DIFF dir;

    TILE thisBox = getTile( board, coords );
    TILE destTile = getTile( board, destCoords );

    switch( destTile.type ) {
        case FLOOR:
            /* Free to enter */
            setTile( board, destCoords, thisBox.type, thisBox.subType );
            setTile( board, coords, FLOOR, 0 );
            moved = TRUE;
            break;

        case WALL:
        case BOX:
        case ITEM:
            /* Blocked */
            break;

        case PLACE:
            /* Place. Check if they match */
            if( destTile.subType == thisBox.subType ) {
                /* Matched, convert into gold */
                setTile( board, destCoords, ITEM, GOLD );
                setTile( board, coords, FLOOR, 0 );
                moved = TRUE;
            }
            else
                /* Doesn't match */
                ;
            break;

        case ARROW:
            /* Try to move in direction */
            dir = getDir( destTile.subType );
            printf("DIR = %d %d\n", dir.dx, dir.dy);

            /* Move box onto arrow */

            setTile( board, destCoords, thisBox.type, thisBox.subType );
            setTile( board, coords, FLOOR, 0 );

            moved = TRUE;

            /* Check next move */

            if( dir.dx == -diff.dx && dir.dy == -diff.dy )
                /* Trying to move back, stop */
                ;
            else {
                /* Move further */
                boxMove( board, destCoords, dir, FALSE );
            }
            break;
    }
    return( moved );
}

/* Main engine routine - hero movement */
LONG heroMove( struct Board *board, DIFF diff )
{
    BOOL moved = FALSE;
    COORDS heroCoords = board->heroCoords;
    COORDS destCoords = getSibling( heroCoords, diff );
    TILE destTile = getTile( board, destCoords );

    switch( destTile.type ) {
        case FLOOR:
            /* Free to enter */
            setTile( board, destCoords, HERO, 0 );
            setTile( board, heroCoords, FLOOR, 0 );
            moved = TRUE;
            break;

        case WALL:
        case PLACE:
        case ARROW:
            /* Blocked */
            break;

        case BOX:
            /* Try to push box */
            if( boxMove( board, destCoords, diff, TRUE ) ) {
                /* Free to enter */
                setTile( board, destCoords, HERO, 0 );
                setTile( board, heroCoords, FLOOR, 0 );
                moved = TRUE;
            }
            break;

        case ITEM:
            /* Collect item */
            if( destTile.subType == GOLD )
                board->goldCollected++;
            else if( destTile.subType == FRUIT )
                board->fruit++;

            setTile( board, destCoords, HERO, 0 );
            setTile( board, heroCoords, FLOOR, 0 );
            moved = TRUE;
            break;
    }
    if (moved)
        board->heroCoords = destCoords;
    return( moved );
}
