
/* Game.c: Main game code */

/* $Id: Game.c,v 1.1 12/.0/.1 .2:.4:.0 Robert Exp $ */

#include <stdio.h>

#include <dos/dos.h>
#include <exec/interrupts.h>
#include <libraries/iffparse.h>
#include <graphics/rpattr.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>

#include "debug.h"

#include "Init.h"

#include "Tile.h"
#include "Game.h"

#define SNDCHAN 0 /* Channel for sound effects */

#define ID_MAGA MAKE_ID('M','A','G','A')
#define ID_NAGL MAKE_ID('N','A','G','L')
#define ID_PLAN MAKE_ID('P','L','A','N')

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

BOOL loadBoard(STRPTR name, struct Board *b, struct boardHeader *bh)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name, IFFF_READ))
    {
        if (!PropChunk(iff, ID_MAGA, ID_NAGL))
        {
            if (!StopChunk(iff, ID_MAGA, ID_PLAN))
            {
                if (!ParseIFF(iff, IFFPARSE_SCAN))
                {
                    struct StoredProperty *sp;
                    if (sp = FindProp(iff, ID_MAGA, ID_NAGL))
                    {
                        *bh = *(struct boardHeader *)sp->sp_Data;
                        if (ReadChunkBytes(iff, b->board, sizeof(b->board)) == sizeof(b->board))
                        {
                            closeIFF(iff);
                            return(TRUE);
                        }
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(FALSE);
}

BOOL saveBoard(STRPTR name, struct Board *b, struct boardHeader *bh)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name, IFFF_WRITE))
    {
        if (!PushChunk(iff, ID_MAGA, ID_FORM, IFFSIZE_UNKNOWN))
        {
            if (!PushChunk(iff, ID_MAGA, ID_NAGL, sizeof(*bh)))
            {
                if (WriteChunkBytes(iff, bh, sizeof(*bh)) == sizeof(*bh))
                {
                    if (!PopChunk(iff))
                    {
                        if (!PushChunk(iff, ID_MAGA, ID_PLAN, sizeof(b->board)))
                        {
                            if (WriteChunkBytes(iff, b->board, sizeof(b->board)) == sizeof(b->board))
                            {
                                if (!PopChunk(iff))
                                {
                                    if (!PopChunk(iff))
                                    {
                                        closeIFF(iff);
                                        return(TRUE);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(FALSE);
}

void printLevel(struct Window *w, struct windowInfo *wi, struct boardHeader *bh)
{
    ULONG lock;

    lock = LockIBase(0);
    sprintf(wi->text[GID_MENU2].IText, "Edytor %03d", bh->level);
    UnlockIBase(lock);

    RefreshGList(wi->gads + GID_MENU2, w, NULL, 1);
}

void mainLoop(struct gameInit *gi)
{
    struct Window *w = gi->w;
    struct copperData *cd = &gi->copdata;
    struct BitMap **bm = gi->bm;
    struct BitMap *gfx = gi->gfx;
    struct windowInfo *wi = &gi->wi;
    struct menuInfo *mi = &gi->mi;

    ULONG signals[ 2 ] = { 0 };
    BOOL done = FALSE;
    WORD tilex = 0, tiley = 1 + 8;
    struct Requester req;
    struct Board board = { 0 };
    struct boardHeader header = { VERSION, 1 };

    BOOL paint = FALSE;
    WORD oldx = -1, oldy = -1;

    ULONG total;

    signals[0] = 1L << w->UserPort->mp_SigBit;
    signals[1] = 1L << cd->signal;

    total = signals[0]|signals[1];

    InitRequester(&req);

    if (!loadBoard("Data1/Levels/Level001.lev", &board, &header))
    {
        initBoard(&board);
    }

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
                        playSample(gi->ioa, gi->samples + SAMPLE_BOX, SNDCHAN);
                        done = TRUE;
                    }
                    else if (gad->GadgetID == GID_MENU2)
                    {
                        struct Window *menu;

                        Request(&req, w);

                        playSample(gi->ioa, gi->samples + SAMPLE_BOX, SNDCHAN);

                        if (menu = openMenuWindow(w, 64, 64, 80, NULL))
                        {
                            struct IntuiMessage *im;

                            menu->WScreen->BitMap = *bm[1];

                            AddGList(menu, mi->gads, -1, -1, NULL);
                            RefreshGList(mi->gads, menu, NULL, -1);

                            BltBitMap(gfx, 0, 32, bm[1], menu->LeftEdge, menu->TopEdge, 64, 16, 0xc0, 0xff, NULL);

                            Move(menu->RPort, 4, 4 + menu->RPort->Font->tf_Baseline);
                            SetAPen(menu->RPort, 4);
                            SetDrMd(menu->RPort, JAM1);
                            Text(menu->RPort, "Edytor", 6);

                            menu->WScreen->BitMap = *bm[0];

                            SetSignal(0L, 1L << cd->signal);
                            Wait(1L << cd->signal);

                            drawTile(bm[1], menu->LeftEdge, menu->TopEdge, bm[0], menu->LeftEdge, menu->TopEdge, menu->Width, menu->Height);

                            WaitPort(menu->UserPort);

                            while (im = (struct IntuiMessage *)GetMsg(menu->UserPort))
                            {
                                ULONG class = im->Class;
                                APTR iaddr = im->IAddress;

                                /* WORD mx = im->MouseX, my = im->MouseY; */
                                ReplyMsg((struct Message *)im);

                                if (class == IDCMP_GADGETUP)
                                {
                                    struct Gadget *gad = (struct Gadget *)iaddr;
                                    if (gad->GadgetID == MID_SAVE)
                                    {
                                        UBYTE name[16];
                                        sprintf(name, "Data1/Levels/Level%03d.lev", header.level);
                                        saveBoard(name, &board, &header);
                                    }
                                    else if (gad->GadgetID == MID_RESTORE)
                                    {
                                        UBYTE name[16];
                                        sprintf(name, "Data1/Levels/Level%03d.lev", header.level);
                                        loadBoard(name, &board, &header);

                                        drawBoard(&board, bm[1], gfx, 0, 0, 19, 14);
                                        SetSignal(0L, 1L << cd->signal);
                                        Wait(1L << cd->signal);
                                        drawTile(bm[1], 0, 0, bm[0], 0, 0, 320, 240);
                                    }
                                    else if (gad->GadgetID == MID_PREV)
                                    {
                                        if (header.level > 1)
                                        {
                                            UBYTE name[16];

                                            header.level--;
                                            printLevel(w, wi, &header);
                                            sprintf(name, "Data1/Levels/Level%03d.lev", header.level);

                                            if (!loadBoard(name, &board, &header))
                                            {
                                                initBoard(&board);
                                            }

                                            drawBoard(&board, bm[1], gfx, 0, 0, 19, 14);
                                            SetSignal(0L, 1L << cd->signal);
                                            Wait(1L << cd->signal);
                                            drawTile(bm[1], 0, 0, bm[0], 0, 0, 320, 240);
                                        }
                                    }
                                    else if (gad->GadgetID == MID_NEXT)
                                    {
                                        if (header.level < 10)
                                        {
                                            UBYTE name[16];

                                            header.level++;
                                            printLevel(w, wi, &header);
                                            sprintf(name, "Data1/Levels/Level%03d.lev", header.level);

                                            if (!loadBoard(name, &board, &header))
                                            {
                                                initBoard(&board);
                                            }

                                            drawBoard(&board, bm[1], gfx, 0, 0, 19, 14);
                                            SetSignal(0L, 1L << cd->signal);
                                            Wait(1L << cd->signal);
                                            drawTile(bm[1], 0, 0, bm[0], 0, 0, 320, 240);
                                        }
                                    }
                                }
                            }

                            CloseWindow(menu);
                        }


                        EndRequest(&req, w);
                    }
                    else if (gad->GadgetID == GID_MENU3)
                    {
                        struct Window *menu;

                        Request(&req, w);

                        playSample(gi->ioa, gi->samples + SAMPLE_DIG, SNDCHAN);
                        if (menu = openMenuWindow(w, 128, 64, 80, NULL))
                        {
                            struct IntuiMessage *im;

                            BltBitMap(gfx, 0, 32, bm[1], menu->LeftEdge, menu->TopEdge, 64, 16, 0xc0, 0xff, NULL);

                            BltBitMap(gfx, 0, 128, bm[1], menu->LeftEdge, menu->TopEdge + 16, 64, 64, 0xc0, 0xff, NULL);

                            SetSignal(0L, 1L << cd->signal);
                            Wait(1L << cd->signal);
                            drawTile(bm[1], menu->LeftEdge, menu->TopEdge, bm[0], menu->LeftEdge, menu->TopEdge, menu->Width, menu->Height);
                            Move(menu->RPort, 4, 4 + menu->RPort->Font->tf_Baseline);
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
                            struct Cell *c = &board.board[my >> 4][mx >> 4];
                            BltBitMap(gfx, tilex << 4, tiley << 4, bm[0], mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0, 0xff, NULL);
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

    saveBoard("T:Temp.bak", &board, &header);
}

int main(void)
{
    struct gameInit *initData;

    if (initData = AllocMem(sizeof(*initData), MEMF_PUBLIC|MEMF_CLEAR))
    {
        if (initGame(initData))
        {
            mainLoop(initData);
            freeGame(initData);
        }
        FreeMem(initData, sizeof(*initData));
    }
    return(RETURN_OK);
}
