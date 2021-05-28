
#include <stdio.h>
#include <assert.h>

#include "Object.h"

#define SGN(a) ((a)>0?(1):(-1))
#define ABS(a) ((a)>0?(a):-(a))

/* Rotate if needed */
BOOL objectRotate(struct object *o)
{
    WORD dir = o->dir, reqdir = o->reqdir;
    WORD diff = SGN(reqdir - dir), dist = ABS(reqdir - dir);

    assert(o->offset == 0);

    if (dist == 0)
        /* No rotate needed */
        return(TRUE);

    /* Calc shorter distance */
    if ((DIRS - dist) < dist)
        /* Change direction */
        diff = -diff;

    dir += diff;

    if (dir == -1)
        dir = DIRS - 1;
    else if (dir == DIRS)
        dir = 0;

    o->dir = dir;
    return(FALSE);
}

WORD DX(WORD dir)
{
    if (dir == UP || dir == DOWN)
        return(0);
    if (dir == LEFT)
        return(-1);
    else
        return(1);
}

WORD DY(WORD dir)
{
    if (dir == LEFT || dir == RIGHT)
        return(0);
    if (dir == UP)
        return(-1);
    else
        return(1);
}

/* Move object */
void objectMove(struct object *o, WORD dir)
{
    if (o->state == STOP) {
        o->reqdir = dir;
        o->state = MOVE;

        o->x += DX(dir);
        o->y += DY(dir);
    }
    else
        o->nextdir = dir;
}

/* One step of movement */
void objectStep(struct object *o)
{
    WORD offset = o->offset;
    WORD state = o->state;

    if (state == STOP)
        return;

    if (offset == 0)
        if (!objectRotate(o))
            return;

    if (++offset == 16) {
        offset = 0;
        state = STOP;
    }

    o->state = state;
    o->offset = offset;
}

int main(void)
{
    struct object o = { 0 };

    o.state = STOP;
    objectMove(&o, RIGHT);

    printf("State\tDir\tReqDir\tOffset\n");
    while (o.state != STOP) {
        printf("%d\t%d\t%d\t%d\n", o.state, o.dir, o.reqdir, o.offset);
        objectStep(&o);
    }
    printf("%d\t%d\t%d\t%d\n", o.state, o.dir, o.reqdir, o.offset);
    return(0);
}
