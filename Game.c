
#include "Game.h"
#include "Graphics.h"

#include <dos/dos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

/* Create board template */

BOOL initBoard(struct board *board, WORD tmpwidth, WORD tmpheight)
{
}

BOOL loadBoard(struct board *board)
{
}

BOOL saveBoard(struct board *board)
{
}

/* Free memory */

void freeBoard(struct board *board)
{
}

void drawBoard(struct board *board)
{
}

void drawFloor(struct board *board, WORD x, WORD y)
{
}

/* Move object in given direction */

void moveObject(struct board *board, struct object *obj, WORD offsetx, WORD offsety)
{
}

int main()
{
    struct Screen *s;
    struct BitMap *bm[2];
    struct gfxInfo gi = { 0 };

    if (loadGraphics(&gi, "Data/Warehouse.iff"))
    {
        if (bm[0] = createBitMap(&gi))
        {
            if (s = openScreen(bm[0], &gi))
            {
                Delay(300);
                closeScreen(s);
            }
            FreeBitMap(bm[0]);
        }
        unloadGraphics(&gi);
    }
    return(RETURN_OK);
}
