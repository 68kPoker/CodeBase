
#include "Warehouse.h"

#include <exec/memory.h>
#include <clib/exec_protos.h>

/* Init Warehouse - to given size */
struct warehouse *initWarehouse(WORD width, WORD height)
{
    WORD x, y;
    struct cell c = { 0 };

    struct warehouse *wh = AllocMem(sizeof(*wh), MEMF_PUBLIC|MEMF_CLEAR);

    if (!wh)
        return(NULL);

    for (y = 0; y < BOARD_H; y++)
    {
        for (x = 0; x < BOARD_W; x++)
        {
            /* Center cell * 2 */
            WORD center_x = (BOARD_W - 1);
            WORD center_y = (BOARD_H - 1);

            WORD diffx = ABS((x * 2) - center_x) + 1;
            WORD diffy = ABS((y * 2) - center_y) + 1;
            if (diffx > width || diffy > height)
            {
                /* Outside box */
                c.floor.main  = FLOOR_NONE;
                c.obj.main = OBJECT_NONE;
            }
            else if (diffx == width || diffy == width)
            {
                /* Boundary */
                c.floor.main  = FLOOR_NORMAL;
                c.obj.main = OBJECT_WALL;
            }
            else
            {
                /* Inside box */
                c.floor.main  = FLOOR_NORMAL;
                c.obj.main = OBJECT_NONE;
            }
            wh->board[0].cells[y][x] = c;
        }
    }
    return(wh);
}

LONG play(struct warehouse *wh)
{
    WORD x, y;

    for (y = 0; y < BOARD_H; y++)
    {
        for (x = 0; x < BOARD_W; x++)
        {
            printf("%c", '0' + wh->board[0].cells[y][x].floor.main);
        }
        printf("\n");
    }
}

void freeWarehouse(struct warehouse *wh)
{
    FreeMem(wh, sizeof(*wh));
}

main()
{
    struct warehouse *wh;

    if (wh = initWarehouse(16, 8))
    {
        play(wh);
        freeWarehouse(wh);
    }
}
