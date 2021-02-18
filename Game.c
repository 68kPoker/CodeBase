
/* Game.c: Main game code */

/* $Id: Game.c,v 1.1 12/.0/.1 .2:.4:.0 Robert Exp $ */

#include <exec/interrupts.h>
#include <libraries/iffparse.h>
#include <graphics/rpattr.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"
#include "IFF.h"
#include "Window.h"
#include "Tile.h"
#include "Game.h"

void initBoard(struct Board *b)
{
    WORD x, y;
    struct Cell floor = { FLOOR_KIND, NORMAL_FLOOR, NORMAL_FLOOR };
    struct Cell wall = { WALL_KIND, NORMAL_WALL };

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            if (x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
                b->board[y][x] = wall;
            else
                b->board[y][x] = floor;
        }
    }
}

void mainLoop(struct Window *w, struct copperData *cd, struct BitMap *bm[], struct BitMap *gfx)
{
    ULONG signals[] =
    {
        1L << w->UserPort->mp_SigBit,
        1L << cd->signal
    };
    BOOL done = FALSE;
    WORD tilex = 0, tiley = 1 + 8;
    struct Requester req;
    struct Board board = { 0 };
    BOOL paint = FALSE;
    WORD oldx = -1, oldy = -1;

    ULONG total = signals[0]|signals[1];

    InitRequester(&req);
    initBoard(&board);

    drawBoard(&board, bm[1], gfx, 0, 0, 19, 14);
    SetSignal(0L, 1L << cd->signal);
    Wait(1L << cd->signal);
    drawTile(bm[1], 0, 0, bm[0], 0, 0, 320, 240);

    while (!done)
    {
        ULONG result = Wait(signals[0]);

        if (result & signals[0])
        {
            struct IntuiMessage *msg;

            while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
            {
                ULONG class = msg->Class;
                WORD code = msg->Code, mx = msg->MouseX, my = msg->MouseY;
                APTR iaddr = msg->IAddress;

                ReplyMsg((struct Message *)msg);

                if (class == IDCMP_GADGETUP)
                {
                    struct Gadget *gad = (struct Gadget *)iaddr;
                    if (gad->GadgetID == GID_MENU1)
                    {
                        done = TRUE;
                    }
                    else if (gad->GadgetID == GID_MENU3)
                    {
                        struct Window *menu;

                        Request(&req, w);
                        if (menu = openMenuWindow(w, 96, 80))
                        {
                            struct IntuiMessage *im;

                            BltBitMap(gfx, 0, 16, bm[1], menu->LeftEdge, menu->TopEdge, 16, 16, 0xc0, 0xff, NULL);
                            BltBitMap(gfx, 32, 16, bm[1], menu->LeftEdge + 80, menu->TopEdge, 16, 16, 0xc0, 0xff, NULL);
                            BltBitMap(gfx, 0, 0, bm[1], menu->LeftEdge + 16, menu->TopEdge, 64, 16, 0xc0, 0xff, NULL);

                            BltBitMap(gfx, 0, 128, bm[1], menu->LeftEdge, menu->TopEdge + 16, 96, 64, 0xc0, 0xff, NULL);

                            SetSignal(0L, 1L << cd->signal);
                            Wait(1L << cd->signal);
                            drawTile(bm[1], menu->LeftEdge, menu->TopEdge, bm[0], menu->LeftEdge, menu->TopEdge, menu->Width, menu->Height);
                            Move(menu->RPort, 20, 4 + menu->RPort->Font->tf_Baseline);
                            SetAPen(menu->RPort, 4);
                            SetDrMd(menu->RPort, JAM1);
                            Text(menu->RPort, "Kafelek", 7);

                            SetAPen(menu->RPort, 10);
                            Move(menu->RPort, 0, 15);
                            Draw(menu->RPort, 0, menu->Height - 1);
                            SetAPen(menu->RPort, 0);
                            Draw(menu->RPort, menu->Width - 1, menu->Height - 1);
                            Draw(menu->RPort, menu->Width - 1, 16);

                            WaitPort(menu->UserPort);

                            while (im = (struct IntuiMessage *)GetMsg(menu->UserPort))
                            {
                                WORD mx = im->MouseX, my = im->MouseY;
                                if (my >= 16)
                                {
                                    tiley = 8 + (my >> 4) - 1;
                                    tilex = mx >> 4;
                                }
                                ReplyMsg((struct Message *)im);
                            }

                            CloseWindow(menu);
                        }
                        EndRequest(&req, w);
                    }
                }
                else if (class == IDCMP_REFRESHWINDOW)
                {
                    struct Rectangle bounds;

                    BeginRefresh(w);
                    GetRPAttrs(w->RPort, RPTAG_DrawBounds, &bounds, TAG_DONE);

                    drawBoard(&board, bm[1], gfx, bounds.MinX >> 4, bounds.MinY >> 4, bounds.MaxX >> 4, bounds.MaxY >> 4);

                    SetSignal(0L, 1L << cd->signal);
                    Wait(1L << cd->signal);

                    drawTile(bm[1], bounds.MinX, bounds.MinY, bm[0], bounds.MinX, bounds.MinY, bounds.MaxX - bounds.MinX + 1, bounds.MaxY - bounds.MinY + 1);
                    EndRefresh(w, TRUE);
                }
                else if (class == IDCMP_MOUSEBUTTONS)
                {
                    if (code == IECODE_LBUTTON)
                    {
                        if (my < 240)
                        {
                            BltBitMap(gfx, tilex << 4, tiley << 4, bm[0], mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0, 0xff, NULL);
                            struct Cell *c = &board.board[my >> 4][mx >> 4];
                            c->kind = tiley - 8;
                            c->subKind = tilex;
                            paint = TRUE;
                            oldx = mx >> 4;
                            oldy = my >> 4;
                        }
                    }
                    else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                    {
                        paint = FALSE;
                    }
                }
                else if (class == IDCMP_MOUSEMOVE)
                {
                    if (paint && my < 240 && (oldx != (mx >> 4) || oldy != (my >> 4)))
                    {
                        struct Cell *c = &board.board[my >> 4][mx >> 4];
                        c->kind = tiley - 8;
                        c->subKind = tilex;
                        BltBitMap(gfx, tilex << 4, tiley << 4, bm[0], mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0, 0xff, NULL);
                        oldx = mx >> 4;
                        oldy = my >> 4;
                    }
                }
            }
        }
    }
}

int main(void)
{
    struct Interrupt is;
    struct copperData cd;
    struct windowInfo wi;
    struct TextFont *tf;
    struct BitMap *bm[2];

    if (bm[0] = allocBitMap())
    {
        if (bm[1] = allocBitMap())
        {
            struct Screen *s;

            if (s = openScreen(bm[0], &tf))
            {
                struct IFFHandle *iff;
                if (iff = openIFF("Data/Graphics.iff", IFFF_READ))
                {
                    if (scanILBM(iff))
                    {
                        if (loadCMAP(iff, s))
                        {
                            struct BitMap *gfx;
                            if (gfx = loadBitMap(iff))
                            {
                                struct Window *w;
                                initWindow(&wi, gfx);

                                if (w = openBDWindow(s, &wi))
                                {
                                    if (addCopperList(&s->ViewPort))
                                    {
                                        if (addCopperInt(&is, &cd, &s->ViewPort))
                                        {
                                            mainLoop(w, &cd, bm, gfx);
                                            remCopperInt(&is);
                                        }
                                    }
                                    CloseWindow(w);
                                }
                                freeWindow(&wi);
                                FreeBitMap(gfx);
                            }
                        }
                    }
                    closeIFF(iff);
                }
                CloseScreen(s);
                CloseFont(tf);
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    return(0);
}
