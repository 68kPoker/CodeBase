
    /***************************************************************\
    *                                                               *
    *   Board.c                                                     *
    *                                                               *
    *       Board manipulation.                                     *
    *                                                               *
    \***************************************************************/

#include "Board.h"

    /***************************************************************\
    *                                                               *
    *   initBoard()                                                 *
    *                                                               *
    *       Clears board making a floor rectangle in center         *
    *       of given width and height with wall border.             *
    *                                                               *
    \***************************************************************/

BOOL initBoard( BOARD *b, WORD wide, WORD tall )
{
    WORD x, y;

    if( wide > BOARD_WIDE || tall > BOARD_TALL )
        return( FALSE );

    WORD centerX = BOARD_WIDE - 1;
    WORD centerY = BOARD_TALL - 1;

    for (y = 0; y < BOARD_TALL; y++)
    {
        for (x = 0; x < BOARD_WIDE; x++)
        {
            CELL *c = &b->board[ y ][ x ];

            WORD diffX = ABS((x * 2) - centerX) + 1;
            WORD diffY = ABS((y * 2) - centerY) + 1;
            if (diffX > wide || diffY > tall)
            {
                /* Outside center */
                deleteCell(c);
            }
            else
            {
                /* Inside center */
                clearFloor(c);
                if (diffX == wide || diffY == tall)
                {
                    /* Border */
                    setObject(c, OBJECT_WALL);
                }
            }
        }
    }
    return( TRUE );
}

/** EOF **/
