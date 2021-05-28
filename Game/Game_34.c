
#include <stdio.h>

#include "Game.h"

#define isPerimeter(x, y) (x) == 0 || (x) == WIDTH - 1 || (y) == 0 || (y) == HEIGHT - 1

WORD offsets[] = { -WIDTH, 1, WIDTH, -1 };

/* Zainicjuj planszë */
void boardClear(BOARD *b)
{
    WORD x, y;
    TILE *t = &b->tiles[0][0];
    b->heroX = b->heroY = 1;

    for (y = 0; y < HEIGHT; y++)
    {
        for (x = 0; x < WIDTH; x++, t++)
        {
            t->floor  = FLOOR;
            t->object = NOOBJ;
            if (isPerimeter(x, y))
            {
                t->floor = WALL;
            }
        }
    }
    b->tiles[1][1].object = HERO;
    b->dir = DOWN;
    b->boxes = b->placed = b->keys = 0;
}

/* Zmieï poîoûenie bohatera */
void heroPlace(BOARD *b, WORD newX, WORD newY)
{
    b->tiles[b->heroY][b->heroX].object = NOOBJ;
    b->heroX = newX;
    b->heroY = newY;
    b->tiles[newY][newX].object = HERO;
}

void putHero(BOARD *b, TILE *t)
{
    heroPlace(b, (t - b->tiles) % WIDTH, (t - b->tiles) / WIDTH);
}

void putBox(BOARD *b, TILE *t)
{
    if (t->object == NOOBJ)
    {
        t->object = BOX;
        b->boxes++;

        if (t->floor == FLAGSTONE)
        {
            b->placed++;
        }
    }
}

void remBox(BOARD *b, TILE *t)
{
    if (t->object == BOX)
    {
        t->object = NOOBJ;
        b->boxes--;

        if (t->floor == FLAGSTONE)
        {
            b->placed--;
        }
    }
}

void putWall(TILE *t)
{
    if (t->object == NOOBJ)
    {
        t->floor = WALL;
    }
}

void remWall(TILE *t)
{
    if (t->floor == WALL)
    {
        t->floor = FLOOR;
    }
}

void fillMud(TILE *t)
{
    if (t->floor == MUD)
    {
        t->floor = FILLED_MUD;
    }
}

BOOL openDoor(BOARD *b, TILE *t)
{
    if (t->object == DOOR)
    {
        if (b->keys > 0)
        {
            b->keys--;
            t->object = NOOBJ;
            return(TRUE);
        }
    }
    return(FALSE);
}

void putObject(BOARD *b, TILE *t, WORD object)
{
    if (object == BOX)
    {
        putBox(b, t);
    }
    else
    {
        t->object = object;
    }
}

void remObject(BOARD *b, TILE *t)
{
    if (t->object == BOX)
    {
        remBox(b, t);
    }
    else
    {
        t->object = NOOBJ;
    }
}

/* Change floor only */
void putFloor(BOARD *b, TILE *t, WORD floor)
{
    WORD object = t->object; /* Remember */

    remObject(b, t);
    t->floor = floor;
    putObject(b, t, object);
}


BOOL pushBox(BOARD *b, TILE *t, WORD dir)
{
    if (t->object == BOX)
    {
        WORD offset = offsets[dir / 2];
        TILE *s = t + offset;

        if (s->object != NOOBJ || s->floor == WALL)
        {
            return(FALSE);
        }

        remBox(b, t);
        if (s->floor == MUD)
        {
            fillMud(s);
        }
        else
        {
            putBox(b, s);
        }
        return(TRUE);
    }
    return(FALSE);
}

BOOL moveHero(BOARD *b, WORD dir)
{
    TILE *t = &b->tiles[b->heroY][b->heroX];

    if (t->object == HERO)
    {
        WORD offset = offsets[dir / 2];
        TILE *s = t + offset;

        if (s->floor == WALL || s->floor == MUD)
        {
            return(FALSE);
        }

        if (s->object == BOX)
        {
            if (!pushBox(b, s, dir))
            {
                return(FALSE);
            }
        }
        else if (s->object == DOOR)
        {
            if (!openDoor(b, s))
            {
                return(FALSE);
            }
            return(TRUE);
        }
        putHero(b, s); /* Change hero position */
        return(TRUE);
    }
    return(FALSE);
}

/* Przygotuj edytor */
void editorSetup(EDITOR *ed)
{
    ed->cursX = ed->cursY = 1;
    ed->changeFloor = TRUE;
    ed->curTile.floor = FLOOR;
    ed->curTile.object = BOX;
}

int main(void)
{
    BOARD b;

    boardClear(&b);

    putFloor(&b, &b.tiles[1][4], FLAGSTONE);
    putBox(&b, &b.tiles[1][2]);
    putFloor(&b, &b.tiles[1][5], MUD);

    printf("%d/%d (%d/%d)\n", b.placed, b.boxes, b.heroX, b.heroY);
    printf("MUD = %d\n", b.tiles[1][5].floor);

    moveHero(&b, RIGHT);
    printf("%d/%d (%d/%d)\n", b.placed, b.boxes, b.heroX, b.heroY);

    moveHero(&b, RIGHT);
    printf("%d/%d (%d/%d)\n", b.placed, b.boxes, b.heroX, b.heroY);

    moveHero(&b, RIGHT);
    printf("%d/%d (%d/%d)\n", b.placed, b.boxes, b.heroX, b.heroY);

    printf("MUD = %d\n", b.tiles[1][5].floor);
    return 0;

}
