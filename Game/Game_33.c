
#include "Main.h"
#include "Misc.h"

#include <intuition/intuition.h>
#include <graphics/rpattr.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>

#include <stdio.h>
#include "debug.h"

#define ESC_KEY  0x45

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define CENTER_X 64
#define CENTER_Y 64
#define WIDTH    192
#define HEIGHT   128
#define MENU_X   0
#define MENU_Y   128

#define SELWIDTH  4
#define SELHEIGHT 4

#define COPPOS1  44
#define COPPOS2  172

enum
{
    USER_SIG,
    COPPER_SIG,
    SIGNALS
};

void drawBoard(WORD x1, WORD y1, WORD x2, WORD y2)
{
    WORD x, y;

    for (y = y1; y <= y2; y++)
    {
        for (x = x1; x <= x2; x++)
        {
            if (x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
                drawTile(gfx, 0, 16, bitmaps[1], x << 4, y << 4, 16, 16);
            else
                drawTile(gfx, 0, 0, bitmaps[1], x << 4, y << 4, 16, 16);
        }
    }
}

void drawGrid(struct Window *w)
{
    struct RastPort *rp = w->RPort;
    WORD i;

    SetAPen(rp, 13);
    for (i = 0; i <= SELWIDTH; i++)
    {
        Move(rp, 0, i * 17);
        Draw(rp, w->Width - 1, i * 17);
    }
    for (i = 0; i <= SELHEIGHT; i++)
    {
        Move(rp, i * 17, 1);
        Draw(rp, i * 17, w->Height - 2);
    }
}

void highlightGrid(struct Window *w, WORD itemx, WORD itemy, WORD color)
{
    struct RastPort *rp = w->RPort;
    itemx *= 17;
    itemy *= 17;

    SetAPen(rp, color);

    Move(rp, itemx, itemy);
    Draw(rp, itemx + 17, itemy);
    Draw(rp, itemx + 17, itemy + 17);
    Draw(rp, itemx, itemy + 17);
    Draw(rp, itemx, itemy + 1);
}

void drawSelection(struct Window *w)
{
    struct RastPort *rp = w->RPort;
    WORD i, j;
    rp->BitMap = bitmaps[1];

    drawGrid(w);
    for (i = 0; i < SELHEIGHT; i++)
    {
        for (j = 0; j < SELWIDTH; j++)
        {
            BltBitMapRastPort(gfx, j << 4, i << 4, rp, (j * 17) + 1, (i * 17) + 1, 16, 16, 0xc0);
        }
    }

    rp->BitMap = bitmaps[0];

    BltBitMapRastPort(bitmaps[1], w->LeftEdge, w->TopEdge, rp, 0, 0, w->Width, w->Height, 0xc0);
}

int runGame(void)
{
    struct Window *w;

    if (w = openWindow(0, 0, screen->Width, screen->Height))
    {
        struct RastPort *rp = w->RPort;
        ULONG signals[SIGNALS] =
        {
            1L << w->UserPort->mp_SigBit,
            1L << cop_data.signal
        }, total, result;
        BOOL done = FALSE;
        WORD pos;

        drawBoard(0, 0, BOARD_WIDTH - 1, BOARD_HEIGHT - 1);
        /* drawTile(gfx, MENU_X, MENU_Y, bitmaps[1], CENTER_X, CENTER_Y, WIDTH, HEIGHT); */

        SetSignal(0L, signals[COPPER_SIG]);
        Wait(signals[COPPER_SIG]);
        pos = cop_data.vpos;

        drawTile(bitmaps[1], 0, 0, bitmaps[0], 0, 0, screen->Width, screen->Height);

        total = signals[USER_SIG] | signals[COPPER_SIG];

        while (!done)
        {
            result = Wait(total);

            if (result & signals[COPPER_SIG])
            {
            }

            if (result & signals[USER_SIG])
            {
                struct IntuiMessage *msg;
                while (msg = GT_GetIMsg(w->UserPort))
                {
                    ULONG class = msg->Class;
                    WORD code = msg->Code;
                    WORD mx = msg->MouseX;
                    WORD my = msg->MouseY;
                    APTR iaddr = msg->IAddress;

                    GT_ReplyIMsg(msg);
                    if (class == IDCMP_RAWKEY)
                    {
                        if (code == ESC_KEY)
                        {
                            done = TRUE;
                        }
                    }
                    else if (class == IDCMP_MOUSEBUTTONS)
                    {
                        if (code == IECODE_RBUTTON)
                        {
                            struct Requester req;
                            struct Window *rw;

                            InitRequester(&req);
                            Request(&req, w);

                            if (rw = openWindow(mx - (SELWIDTH << 3), my - (SELHEIGHT << 3), (SELWIDTH * 17) + 1, (SELHEIGHT * 17) + 1))
                            {
                                BOOL done = FALSE;
                                WORD prevx = mx / 17, prevy = my / 17;
                                drawSelection(rw);
                                while (!done)
                                {
                                    struct IntuiMessage *msg;

                                    WaitPort(rw->UserPort);
                                    while (msg = GT_GetIMsg(rw->UserPort))
                                    {
                                        ULONG class = msg->Class;
                                        WORD code = msg->Code;
                                        WORD mx = msg->MouseX, my = msg->MouseY;

                                        GT_ReplyIMsg(msg);

                                        if (class == IDCMP_MOUSEBUTTONS)
                                        {
                                            if (code == IECODE_LBUTTON)
                                            {
                                                done = TRUE;
                                            }
                                            else if (code == IECODE_RBUTTON)
                                            {
                                                done = TRUE;
                                            }
                                        }
                                        else if (class == IDCMP_MOUSEMOVE)
                                        {
                                            if (mx >= 0 && mx < rw->Width - 1 && my >= 0 && my < rw->Height - 1)
                                            {
                                                mx /= 17;
                                                my /= 17;

                                                if (mx != prevx || my != prevy)
                                                {
                                                    highlightGrid(rw, prevx, prevy, 12);
                                                    highlightGrid(rw, mx, my, 14);
                                                    prevx = mx;
                                                    prevy = my;
                                                }
                                            }
                                            else
                                            {
                                                highlightGrid(rw, prevx, prevy, 12);
                                            }
                                        }
                                    }
                                }
                                CloseWindow(rw);
                            }
                            EndRequest(&req, w);
                        }
                    }
                    else if (class == IDCMP_REFRESHWINDOW)
                    {
                        struct Rectangle bounds;
                        WORD x1, y1, x2, y2;

                        BeginRefresh(w);

                        GetRPAttrs(w->RPort, RPTAG_DrawBounds, &bounds, TAG_DONE);

                        x1 = bounds.MinX & 0xfff0;
                        y1 = bounds.MinY & 0xfff0;

                        x2 = bounds.MaxX | 0x000f;
                        y2 = bounds.MaxY | 0x000f;

                        drawBoard(x1 >> 4, y1 >> 4, x2 >> 4, y2 >> 4);

                        SetSignal(0L, signals[COPPER_SIG]);
                        Wait(signals[COPPER_SIG]);
                        pos = cop_data.vpos;

                        drawTile(bitmaps[1], x1, y1, bitmaps[0], x1, y1, x2 - x1 + 1, y2 - y1 + 1);

                        EndRefresh(w, TRUE);
                    }
                }
            }
        }
        CloseWindow(w);
    }
    return(0);
}
