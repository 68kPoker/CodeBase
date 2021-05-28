
#include "Screen.h"
#include "Game.h"

UWORD getGfx(struct gameTile *t)
{
    switch (t->kind)
    {
        case FLOOR_KIND:
            switch (t->type)
            {
                case NORMAL_FLOOR:
                    return(FLOOR_TILE);
                    break;

                case FLAGSTONE_FLOOR:
                    return(PLACE_TILE);
                    break;
            }
            break;
        case WALL_KIND:
            switch (t->type)
            {
                case NORMAL_WALL:
                    return(WALL_TILE);
                    break;

                case DOOR_WALL:
                    return(DOOR_TILE);
                    break;
            }
            break;
        case ITEM_KIND:
            switch (t->type)
            {
                case KEY_ITEM:
                    return(KEY_TILE);
                    break;

                case FRUIT_ITEM:
                    return(FRUIT_TILE);
                    break;
            }
            break;
        case OBJECT_KIND:
            switch (t->type)
            {
                case HERO_OBJECT:
                    return(HERO_TILE);
                    break;

                case BOX_OBJECT:
                    if (t->floor == FLAGSTONE_FLOOR)
                        return(PLACED_TILE);
                    else
                        return(SKULL_TILE);
            }
            break;
    }
}

void setTile(struct gameTile *t, UWORD gfx)
{
    t->floor = NORMAL_FLOOR;

    switch (gfx)
    {
        case FLOOR_TILE:
            t->kind = FLOOR_KIND;
            t->type = NORMAL_FLOOR;
            break;

        case WALL_TILE:
            t->kind = WALL_KIND;
            t->type = NORMAL_WALL;
            break;

        case SKULL_TILE:
            t->kind = OBJECT_KIND;
            t->type = BOX_OBJECT;
            break;

        case PLACE_TILE:
            t->kind = FLOOR_KIND;
            t->type = FLAGSTONE_FLOOR;
            break;

        case HERO_TILE:
            t->kind = OBJECT_KIND;
            t->type = HERO_OBJECT;
            break;

        case DOOR_TILE:
            t->kind = WALL_KIND;
            t->type = DOOR_WALL;
            break;

        case KEY_TILE:
            t->kind = ITEM_KIND;
            t->type = KEY_ITEM;
            break;

        case FRUIT_TILE:
            t->kind = ITEM_KIND;
            t->type = FRUIT_ITEM;
            break;
    }
}

BOOL convertBoard(struct gameBoard *gb, struct window *w)
{
    WORD x, y;
    BOOL heroPlaced = FALSE;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            struct tile *src = &w->array[(y * w->width) + x];

            struct gameTile *t = &gb->board[y][x];

            if (y == 0)
            {
                setTile(t, WALL_TILE);
            }
            else
            {
                setTile(t, src->gfx);
                if (t->kind == OBJECT_KIND && t->type == HERO_OBJECT)
                {
                    heroPlaced = TRUE;
                    gb->herox = x;
                    gb->heroy = y;
                }
            }
        }
    }
    return(heroPlaced);
}

BOOL moveHero(struct gameBoard *gb, WORD dx, WORD dy, struct window *w)
{
    WORD x, y;
    struct gameTile *gt, *next;
    struct tile *t;
    BOOL move = FALSE;

    if ((!dx && !dy) || (dx && dy))
        return(FALSE);

    x = gb->herox;
    y = gb->heroy;

    gt = &gb->board[y][x];
    t = &w->array[(y * w->width) + x];

    x += dx;
    y += dy;

    next = &gb->board[y][x];

    if (next->kind == FLOOR_KIND)
    {
        move = TRUE;
    }
    else if (next->kind == OBJECT_KIND)
    {
        if (next->type == BOX_OBJECT)
        {
            struct gameTile *past = &gb->board[y + dy][x + dx];

            if (past->kind == FLOOR_KIND)
            {
                past->kind = OBJECT_KIND;
                past->floor = past->type;
                past->type = next->type;

                struct tile *t = &w->array[((y + dy) * w->width) + (x + dx)];

                t->gfx = getGfx(past);
                t->update = 2;

                move = TRUE;
            }
        }
    }



    if (move)
    {
        gt->kind = FLOOR_KIND;
        gt->type = gt->floor;
        t->gfx = getGfx(gt);
        t->update = 2;

        t = &w->array[(y * w->width) + x];

        if (next->kind == FLOOR_KIND)
            next->floor = next->type;

        next->kind = OBJECT_KIND;

        next->type = HERO_OBJECT;

        t->gfx = getGfx(next);
        t->update = 2;

        gb->herox = x;
        gb->heroy = y;

        return(TRUE);
    }
    return(FALSE);
}
