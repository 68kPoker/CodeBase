
#include <intuition/intuition.h>

#include "Gadget.h"

#include <stdio.h>
#include "debug.h"

VOID initBoardGadget (CBoardGadget *p, CWindow *w)
{
    CPosSize pos = { 0, 0, 320, 256 };

    p->Base.Window = w;
    p->EditMode = FALSE;
    p->CurX = w->Window->MouseX >> 4;
    p->CurY = w->Window->MouseY >> 4;
    setWall(&p->CurTile);
    getRectangle(&p->Base.Bounds, &pos);
}

VOID editBoard (CBoardGadget *p, struct IntuiMessage *im)
{
    UWORD x = im->MouseX >> 4,
          y = im->MouseY >> 4;

    CTile *t = &p->Board->Tiles [y][x];

    switch (im->Class)
    {
        case IDCMP_MOUSEBUTTONS:
            switch (im->Code)
            {
                case IECODE_LBUTTON:
                    if (withinPerimeter(x, y))
                    {
                        /* Rozpocznij edycjë */
                        p->EditMode = TRUE;

                        /* Wstaw aktualny kafel */
                        *t = p->CurTile;
                    }
                    break;

                case IECODE_LBUTTON | IECODE_UP_PREFIX:
                    /* Zakoïcz edycjë */
                    p->EditMode = FALSE;
                    break;
            }
            break;
        case IDCMP_MOUSEMOVE:
            if (x != p->CurX || y != p->CurY)
            {
                /* Kursor zmieniî pozycjë */

                p->Board->Tiles [p->CurY][p->CurX].Flags |= BOTH_REDRAW;
                p->Board->Tiles [y][x].Flags |= BOTH_REDRAW;

                p->CurX = x;
                p->CurY = y;

                if (p->EditMode)
                {
                    /* Edycja */
                    if (withinPerimeter(x, y))
                    {
                        /* Kursor wewnâtrz obszaru */
                        *t = p->CurTile;
                    }
                }
            }
            break;
    }
}

VOID drawBoard (CBoardGadget *p)
{
    UWORD x, y;
    struct RastPort *rp = p->Base.Window->Window->RPort;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            CTile *t = &p->Board->Tiles [y][x];

            if (t->Flags & BOTH_REDRAW)
            {
                t->Flags &= ~BOTH_REDRAW;

                SetAPen(rp, 3);

                if (x == p->CurX && y == p->CurY)
                {
                    SetAPen(rp, 2);
                }
                if (t->Kind == TK_FLOOR)
                {
                    SetAPen(rp, 0);
                }
                else if (t->Kind == TK_WALL)
                {
                    SetAPen(rp, 1);
                }
                RectFill(rp, x << 4, y << 4, (x << 4) + 15, (y << 4) + 15);
            }
        }
    }
}
