
#include "Cell.h"

struct Size *getSize(void)
{
    static struct Size size = { BOARD_WIDTH, BOARD_HEIGHT };

    return &size;
}

/* Calc boxes, locate hero etc. */
BOOL scanBoard(struct Board *b)
{
    struct Pos pos;
    struct Size *size = getSize();
    struct Cell *pc = (struct Cell *)b->cells;
    BOOL heroLocated = FALSE;

    b->boxCount = 0;

    for (pos.y = 0; pos.y < size->h; pos.y++)
    {
        for (pos.x = 0; pos.x < size->w; pos.x++, pc++)
        {
            if (pc->type == OBJECT)
            {
                if (pc->sub == HERO)
                {
                    if (heroLocated)
                    {
                        /* Mulitple heroes */
                        return FALSE;
                    }
                    heroLocated = TRUE;
                    b->heroPos = pos;
                }
                else if (pc->sub == BOX)
                {
                    b->boxCount++;
                }
            }
        }
    }
    return TRUE;
}

void moveObject(struct Board *b, struct Cell *pc, struct Cell *nextPc)
{
    nextPc->type = OBJECT;
    nextPc->sub = pc->sub;

    pc->type = FLOOR;
    pc->sub = pc->floor;
}

BOOL pushObject(struct Board *b, struct Cell *pc, WORD offset)
{
    struct Cell *prevPc = pc - offset, *nextPc = pc + offset;
    BOOL move = FALSE;

    if (pc->sub == BOX)
    {
        if (nextPc->type != FLOOR)
        {
        }
        else if (prevPc->sub == HERO)
        {
            if (nextPc->floor == FLAGSTONE)
            {
                b->placedCount++;
            }
            if (pc->floor == FLAGSTONE)
            {
                b->placedCount--;
            }
            move = TRUE;
        }
    }

    if (move)
    {
        moveObject(pc, nextPc);
    }
    return move;
}

BOOL moveHero(struct Board *b, UWORD dir)
{
    struct Size *size = getSize();
    WORD offset = getOffsets(size->w)[dir];

    BOOL move = FALSE;

    struct Cell *pc = &b->cells[b->heroPos.y][b->heroPos.x];
    struct Cell *nextPc = pc + offset;

    if (nextPc->type == FLOOR)
    {
        move = TRUE;
    }
    else if (nextPc->type == WALL)
    {
        /* Check keys etc. */
    }
    else if (nextPc->type == ITEM)
    {
        /* Collect */
        move = TRUE;
    }
    else if (nextPc->type == OBJECT)
    {
        move = pushObject(b, nextPc, offset);
    }

    if (move)
    {
        moveObject(pc, nextPc);
    }
    return move;
}
