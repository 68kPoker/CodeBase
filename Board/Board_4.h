
#ifndef BOARD_H
#define BOARD_H

#include "Tile.h"

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define isPerimeter(x, y) ((x) == 0 || (x) == BOARD_WIDTH - 1 || (y) == 0 || (y) == BOARD_HEIGHT - 1)
#define withinPerimeter(x, y) ((x) > 0 && (x) < BOARD_WIDTH - 1 && (y) > 0 && (y) < BOARD_HEIGHT - 1)

typedef struct SBoard
{
    CTile Tiles [BOARD_HEIGHT][BOARD_WIDTH];
    UWORD HeroX, HeroY; /* Position of unique hero */
    UWORD Boxes;
} CBoard;

VOID clearBoard (CBoard *p);
VOID setHero (CBoard *p, UWORD x, UWORD y);

#endif /* BOARD_H */
