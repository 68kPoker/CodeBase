
#include "Board.h"

VOID clearBoard (CBoard *p)
{
    UWORD x, y;

    p->Boxes = 0;
    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            CTile *t = &p->Tiles [y][x];

            isPerimeter(x, y) ? setWall(t) : setFloor(t);
        }
    }
    p->HeroX = 1; /* Initial hero position */
    p->HeroY = 1;
}

VOID setHero (CBoard *p, UWORD x, UWORD y)
{
    p->HeroX = x;
    p->HeroY = y;
}
