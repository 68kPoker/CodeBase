
    /***************************************************************\
    *                                                               *
    *   Board.h                                                     *
    *                                                               *
    *       Board manipulation.                                     *
    *                                                               *
    \***************************************************************/

#ifndef BOARD_H
#define BOARD_H

#include "Cell.h"

#define ABS(a) ((a)>=0?(a):-(a))

#define BOARD_WIDE 20
#define BOARD_TALL 16

typedef struct Board {
    CELL board[ BOARD_TALL ][ BOARD_WIDE ];
} BOARD;

BOOL initBoard( BOARD *b, WORD wide, WORD tall );

#endif /* BOARD_H */
