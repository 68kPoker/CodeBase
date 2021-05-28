
/* Game.c: Main game code */

/* $Id: Game.c,v 1.2 12/.0/.0 .1:.1:.1 Robert Exp $ */

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

#define SNDCHAN     0 /* Channel for sound effects */

#define SPEED       10 /* Movement delay */

#define NAME        64 /* Buffer len */

#define LEFT_KEY    0x4f
#define RIGHT_KEY   0x4e
#define UP_KEY      0x4c
#define DOWN_KEY    0x4d

#define ID_MAGA MAKE_ID('M','A','G','A')
#define ID_NAGL MAKE_ID('N','A','G','L')
#define ID_PLAN MAKE_ID('P','L','A','N')
#define ID_STAN MAKE_ID('S','T','A','N')

struct gameInit gameInit;
WORD currentOption = 0, lastOptions = 0;

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
    struct Cell *c = &b->board[1][1];
    c->kind = OBJECT_KIND;
    c->subKind = HERO_OBJECT;
    c->floor = NORMAL_FLOOR;
}

/* Init board info */
VOID initBoardInfo(struct gameState *gs)
{
    gs->keys   = 0; /* Reset inventory */
    gs->placed = 0;
    gs->points = gs->totalPoints;
}

VOID completeLevel(struct gameState *gs)
{
    gs->totalPoints = gs->points;
}

/* Determine basic board properties (calc boxes) */
BOOL scanBoard(struct Board *b, struct gameState *gs)
{
    WORD x, y;
    WORD found = 0;

    gs->boxes = 0;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            struct Cell *c = &b->board[y][x];

            if (c->kind == OBJECT_KIND)
            {
                if (c->subKind == HERO_OBJECT)
                {
                    gs->heroX = x;
                    gs->heroY = y;
                    found++;
                }
                else if (c->subKind == BOX_OBJECT)
                {
                    gs->boxes++;
                }
            }
        }
    }
    D(bug("Boxes = %d, found = %d\n", gs->boxes, found));
    return(gs->boxes > 0 && found == 1);
}

void drawMessage(struct gameInit *gi, struct RastPort *rp, STRPTR text, WORD x, WORD y)
{
    SetABPenDrMd(rp, 1, 0, JAM1);
    Move(rp, x, y + rp->Font->tf_Baseline);
    Text(rp, text, strlen(text));

    SetABPenDrMd(rp, 11, 0, JAM1);
    Move(rp, x - 1, y - 1 + rp->Font->tf_Baseline);
    Text(rp, text, strlen(text));
}

LONG standardMessage(struct gameInit *gi, struct Window *reqWin)
{
    struct copperData *cd = &gi->copdata;
    BOOL done = FALSE;

    BltBitMap(gi->gfx, 128, 128, gi->bm[1], 64, 64, 192, 128, 0xc0, 0xff, NULL);

    reqWin->RPort->BitMap = gi->bm[1];

    drawMessage(gi, reqWin->RPort, "Gratulacje!", 64, 24);
    drawMessage(gi, reqWin->RPort, "Przeszedîeô poziom!", 64, 64);

    SetSignal(0L, 1L << cd->signal);

    Wait(1L << cd->signal);
    drawTile(gi->bm[1], 64, 64, gi->bm[0], 64, 64, 192, 128);

    AddGList(reqWin, gi->ri.gads, -1, -1, NULL);
    RefreshGList(gi->ri.gads, reqWin, NULL, -1);

    while (!done)
    {
        struct IntuiMessage *msg;

        WaitPort(reqWin->UserPort);

        while (msg = (struct IntuiMessage *)GetMsg(reqWin->UserPort))
        {
            if (msg->Class == IDCMP_GADGETUP)
            {
                /* Check GID */
                done = TRUE;
            }
            else if (msg->Class == IDCMP_RAWKEY)
            {
                if (msg->Code == 0x40 || msg->Code == 0x44)
                {
                    /* OK */
                    done = TRUE;
                }
                else if (msg->Code == 0x45)
                {
                    /* Cancel */
                    done = TRUE;
                }
            }
            ReplyMsg((struct Message *)msg);
        }
    }
}

LONG levelSelection(struct gameInit *gi, struct Window *reqWin)
{
    struct copperData *cd = &gi->copdata;
    BOOL done = FALSE;
    UBYTE text[64];
    struct Gadget prevGad, nextGad;
    WORD level = gi->state.level;
    LONG result = FALSE;

    if (level > gi->state.maxLevel)
    {
        level = gi->state.maxLevel;
    }

    initButton(&prevGad, NULL, RID_PREV, 64, 40, gi->wi.img + IMG_PREV, gi->wi.img + IMG_PREV_PRESSED);
    initButton(&nextGad, NULL, RID_NEXT, 128, 40, gi->wi.img + IMG_NEXT, gi->wi.img + IMG_NEXT_PRESSED);

    BltBitMap(gi->gfx, 128, 128, gi->bm[1], 64, 64, 192, 128, 0xc0, 0xff, NULL);

    reqWin->RPort->BitMap = gi->bm[1];

    sprintf(text, "Poziom %d", level);
    drawMessage(gi, reqWin->RPort, "Wybierz poziom", 64, 24);
    drawMessage(gi, reqWin->RPort, text, 64, 64);

    SetSignal(0L, 1L << cd->signal);

    Wait(1L << cd->signal);
    drawTile(gi->bm[1], 64, 64, gi->bm[0], 64, 64, 192, 128);

    AddGList(reqWin, gi->ri.gads, -1, -1, NULL);
    AddGadget(reqWin, &prevGad, -1);
    AddGadget(reqWin, &nextGad, -1);
    RefreshGList(gi->ri.gads, reqWin, NULL, -1);
    RefreshGList(&prevGad, reqWin, NULL, -1);
    RefreshGList(&nextGad, reqWin, NULL, -1);

    reqWin->RPort->BitMap = gi->bm[0];

    while (!done)
    {
        struct IntuiMessage *msg;

        WaitPort(reqWin->UserPort);

        while (msg = (struct IntuiMessage *)GetMsg(reqWin->UserPort))
        {
            if (msg->Class == IDCMP_GADGETUP)
            {
                struct Gadget *gad = (struct Gadget *)msg->IAddress;
                /* Check GID */
                if (gad->GadgetID == RID_NEXT)
                {
                    WORD maxLevel = gi->state.maxLevel;

                    if (level < maxLevel)
                        level++;
                    sprintf(text, "Poziom %d", level);
                    SetAPen(reqWin->RPort, 20);
                    RectFill(reqWin->RPort, 94, 63, 112, 74);
                    drawMessage(gi, reqWin->RPort, text, 64, 64);
                }
                else if (gad->GadgetID == RID_PREV)
                {
                    WORD maxLevel = gi->state.maxLevel;

                    if (level > 1)
                        level--;
                    sprintf(text, "Poziom %d", level);
                    SetAPen(reqWin->RPort, 20);
                    RectFill(reqWin->RPort, 94, 63, 112, 74);
                    drawMessage(gi, reqWin->RPort, text, 64, 64);
                }
                else if (gad->GadgetID == RID_CLOSE || gad->GadgetID == RID_OPT2)
                {
                    done = TRUE;
                    result = FALSE;
                }
                else if (gad->GadgetID == RID_OPT1)
                {
                    done = TRUE;
                    result = TRUE;
                }
            }
            else if (msg->Class == IDCMP_RAWKEY)
            {
                if (msg->Code == 0x40 || msg->Code == 0x44)
                {
                    /* OK */
                    done = TRUE;
                    result = TRUE;
                }
                else if (msg->Code == 0x45)
                {
                    /* Cancel */
                    done = TRUE;
                    result = FALSE;
                }
            }
            ReplyMsg((struct Message *)msg);
        }
    }

    RemoveGadget(reqWin, &prevGad);
    RemoveGadget(reqWin, &nextGad);

    if (result)
    {
        gi->state.level = level;
    }

    return(result);
}

LONG dispMessageBox(struct gameInit *gi, STRPTR opt1, STRPTR opt2, LONG (*drawContents)(struct gameInit *gi, struct Window *r))
{
    struct Window *reqWin;
    struct Requester req;
    LONG result = FALSE;

    initReq(&gi->ri, &gi->wi, opt1, opt2);

    InitRequester(&req);
    Request(&req, gi->w);

    if (reqWin = openReqWindow(gi->w))
    {
        result = drawContents(gi, reqWin);
        CloseWindow(reqWin);
    }

    EndRequest(&req, gi->w);
    return(result);
}

BOOL moveHero(struct gameState *gs, struct gameInit *gi, WORD dx, WORD dy)
{
    BOOL pushbox = FALSE;
    struct Board *b = &gs->board;

    WORD heroX = gs->heroX, heroY = gs->heroY;

    struct Cell *c = &b->board[heroY][heroX], *dest = c + (dy * BOARD_WIDTH) + dx;
    struct Cell *past = dest + (dy * BOARD_WIDTH) + dx;
    WORD tilex, tiley;
    WORD x, y;
    WORD snd = SAMPLE_DIG;

    SetSignal(0L, 1L << gi->copdata.signal);
    Wait(1L << gi->copdata.signal);

    WORD destdir, sign, diff;

    if (dx == -1)
        destdir = 0;
    else if (dy == -1)
        destdir = 2;
    else if (dx == 1)
        destdir = 4;
    else if (dy == 1)
        destdir = 6;


    diff = destdir - gs->curDir;

    switch (diff)
    {
        case 0:
            sign = 0;
            break;
        case 2:
        case -6:
        case -4:

            sign = 1;
            break;
        case 6:
        case -2:
        case 4:
            sign = -1;
            break;
    }

    x = heroX;
    y = heroY;


    while (gs->curDir != destdir)
    {
        gs->curDir += sign;

        if (gs->curDir < 0)
        {
            gs->curDir += 8;
        }

        if (gs->curDir >= 8)
        {
            gs->curDir -= 8;
        }

        switch (gs->curDir)
        {
            case 4:
                tilex = 5;
                tiley = 8 + 2;
                break;

            case 5:
                tilex = 5;
                tiley = 8 + 3;
                break;

            case 0:
                tilex = 3;
                tiley = 8 + 2;
                break;

            case 1:
                tilex = 3;
                tiley = 8 + 1;
                break;

            case 6:
                tilex = 4;
                tiley = 8 + 3;
                break;

            case 7:
                tilex = 3;
                tiley = 8 + 3;
                break;

            case 2:
                tilex = 4;
                tiley = 8 + 1;
                break;

            case 3:
                tilex = 5;
                tiley = 8 + 1;
                break;

        }


        BltBitMap(gi->gfx, tilex << 4, tiley << 4, gi->bm[0], x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
        SetSignal(0L, 1L << gi->copdata.signal);
        Wait(1L << gi->copdata.signal);
        SetSignal(0L, 1L << gi->copdata.signal);
        Wait(1L << gi->copdata.signal);
    }

    if (dest->kind == WALL_KIND)
    {
        if (dest->subKind != DOOR_WALL || gs->keys == 0)
        {
            return(FALSE);
        }
        snd = SAMPLE_FRUIT;
        gs->keys--;

        playSample(gi->ioa, gi->samples + snd, SNDCHAN);

        dest->kind = FLOOR_KIND;
        dest->subKind = NORMAL_FLOOR;

        tilex = dest->subKind;
        tiley = 8 + dest->kind;

        x = heroX + dx;
        y = heroY + dy;

        BltBitMap(gi->gfx, tilex << 4, tiley << 4, gi->bm[0], x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);

        updateStatus(gi->w->RPort, b, &gi->state);

        return(FALSE);
    }

    if (dest->kind == ITEM_KIND)
    {
        if (dest->subKind == KEY_ITEM)
        {
            gs->keys++;
            snd = SAMPLE_KEY;
        }
        else if (dest->subKind == FRUIT_ITEM)
        {
            snd = SAMPLE_FRUIT;
            gs->points += 250;
        }
    }

    if (dest->kind == OBJECT_KIND)
    {
        if (dest->subKind != BOX_OBJECT)
        {
            return(FALSE);
        }

        if (past->kind != FLOOR_KIND)
        {
            return(FALSE);
        }

        snd = SAMPLE_BOX;

        past->kind = OBJECT_KIND;
        past->subKind = dest->subKind;

        tilex = past->subKind;
        tiley = 8 + past->kind;

        x = heroX + dx + dx;
        y = heroY + dy + dy;

        if (past->floor == FLAGSTONE_FLOOR)
        {
            tilex = PLACED_OBJECT;
            gs->placed++;
        }

        if (dest->floor == FLAGSTONE_FLOOR)
        {
            gs->placed--;
        }

        if (past->floor == MUD_FLOOR)
        {
            past->subKind = FILLED_FLOOR;
            past->floor = FILLED_FLOOR;
            past->kind = FLOOR_KIND;
            snd = SAMPLE_FRUIT;
        }

        pushbox = TRUE;
    }

    if (dest->kind == FLOOR_KIND && dest->subKind == MUD_FLOOR)
    {
        return(FALSE);
    }

    dest->kind = OBJECT_KIND;
    dest->subKind = c->subKind;

    c->kind = FLOOR_KIND;
    c->subKind = c->floor;

    tilex = c->subKind;
    tiley = 8 + c->kind;

    x = heroX;
    y = heroY;

    BltBitMap(gi->gfx, tilex << 4, tiley << 4, gi->bm[0], x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);

    tilex = dest->subKind;
    tiley = 8 + dest->kind;

    tilex = c->subKind;
    tiley = 8 + c->kind;

    BltBitMap(gi->gfx, tilex << 4, tiley << 4, gi->bm[0], x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);

    x += dx;
    y += dy;

        switch (gs->curDir)
        {
            case 4:
                tilex = 5;
                tiley = 8 + 2;
                break;

            case 5:
                tilex = 5;
                tiley = 8 + 3;
                break;

            case 0:
                tilex = 3;
                tiley = 8 + 2;
                break;

            case 1:
                tilex = 3;
                tiley = 8 + 1;
                break;

            case 6:
                tilex = 4;
                tiley = 8 + 3;
                break;

            case 7:
                tilex = 3;
                tiley = 8 + 3;
                break;

            case 2:
                tilex = 4;
                tiley = 8 + 1;
                break;

            case 3:
                tilex = 5;
                tiley = 8 + 1;
                break;

        }

    BltBitMap(gi->gfx, tilex << 4, tiley << 4, gi->bm[0], x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);

    if (pushbox)
    {
        tilex = past->subKind;
        tiley = 8 + past->kind;

        x = heroX + dx + dx;
        y = heroY + dy + dy;

        BltBitMap(gi->gfx, tilex << 4, tiley << 4, gi->bm[0], x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
    }

    gs->heroX += dx;
    gs->heroY += dy;

    if (snd != SAMPLE_DIG)
        playSample(gi->ioa, gi->samples + snd, SNDCHAN);

    if (gi->state.points > 0)
    {
        gi->state.points--;
    }

    SetSignal(0L, 1L << gi->copdata.signal);
    Wait(1L << gi->copdata.signal);

    updateStatus(gi->w->RPort, b, &gi->state);

    if (gs->placed > 0 && gs->placed == gs->boxes)
    {
        /* Komunikat o przejôciu poziomu */
        dispMessageBox(gi, "OK", "Kontynuuj", standardMessage);
    }

    return(TRUE);
}

WORD testBoard(struct gameState *gs, struct gameInit *gi)
{
    struct Board *b = &gs->board;
    struct IntuiMessage *msg;
    BOOL done = FALSE;
    LONG prevSeconds = 0;
    enum
    {
        LEFT,
        RIGHT,
        UP,
        DOWN
    };
    UWORD dir = 0;
    WORD counter = 0;
    ULONG signals[2];

    BOOL pause = FALSE;

    signals[0] = 1L << gi->w->UserPort->mp_SigBit;
    signals[1] = 1L << gi->copdata.signal;

    while (msg = (struct IntuiMessage *)GetMsg(gi->w->UserPort))
    {
        ReplyMsg((struct Message *)msg);
    }

    RemoveGList(gi->w, gi->wi.gads, -1);
    initTexts(gi->wi.text);
    AddGList(gi->w, gi->wi.gads, -1, -1, NULL);
    RefreshGList(gi->wi.gads, gi->w, NULL, -1);

    while (!done)
    {
        ULONG result = Wait(signals[0]|signals[1]);

        if (result & signals[1])
        {
            if (gs->placed > 0 && gs->placed == gs->boxes)
            {
                return(RESULT_COMPLETED);
            }

            if (pause)
            {
                if (counter < SPEED)
                {
                    if (++counter == SPEED)
                    {
                        pause = FALSE;
                    }
                }
            }

            if (!pause && (dir & (1 << LEFT)) == (1 << LEFT))
            {
                moveHero(gs, gi, -1, 0);
                pause = TRUE;
                counter = 0;
            }
            else if (!pause && (dir & (1 << RIGHT)) == (1 << RIGHT))
            {
                moveHero(gs, gi, 1, 0);
                pause = TRUE;
                counter = 0;
            }
            else if (!pause && (dir & (1 << UP)) == (1 << UP))
            {
                moveHero(gs, gi, 0, -1);
                pause = TRUE;
                counter = 0;
            }
            else if (!pause && (dir & (1 << DOWN)) == (1 << DOWN))
            {
                moveHero(gs, gi, 0, 1);
                pause = TRUE;
                counter = 0;
            }
        }

        if (result & signals[0])
        {
            while (msg = (struct IntuiMessage *)GetMsg(gi->w->UserPort))
            {
                ULONG class = msg->Class;
                WORD code = msg->Code, mx = msg->MouseX, my = msg->MouseY;
                APTR iaddr = msg->IAddress;
                LONG seconds = msg->Seconds;

                ReplyMsg((struct Message *)msg);

                if (class == IDCMP_GADGETUP)
                {
                    struct Gadget *gad = (struct Gadget *)iaddr;

                    if (gad->GadgetID == GID_CLOSE)
                    {
                        return(RESULT_QUIT);
                    }
                    else if (gad->GadgetID == GID_MENU1)
                    {
                        return(RESULT_START);
                    }
                    else if (gad->GadgetID == GID_MENU4)
                    {
                        if (dispMessageBox(gi, "OK", "Anuluj", levelSelection))
                        {
                            return(RESULT_SELECT);
                        }
                    }
                    else if (gad->GadgetID == GID_MENU2)
                    {
                        return(RESULT_LOAD);
                    }
                    else if (gad->GadgetID == GID_MENU3)
                    {
                        return(RESULT_RESTART);
                    }
                    else if (gad->GadgetID == GID_MENU5)
                    {
                        return(RESULT_EDIT);
                    }
                }
                else if (class == IDCMP_RAWKEY)
                {
                    if (code == LEFT_KEY)
                    {
                        dir |= 1 << LEFT;
                    }
                    else if (code == RIGHT_KEY)
                    {
                        dir |= 1 << RIGHT;
                    }
                    else if (code == UP_KEY)
                    {
                        dir |= 1 << UP;
                    }
                    else if (code == DOWN_KEY)
                    {
                        dir |= 1 << DOWN;
                    }
                    else if (code == (LEFT_KEY|IECODE_UP_PREFIX))
                    {
                        dir &= ~(1 << LEFT);
                    }
                    else if (code == (RIGHT_KEY|IECODE_UP_PREFIX))
                    {
                        dir &= ~(1 << RIGHT);
                    }
                    else if (code == (UP_KEY|IECODE_UP_PREFIX))
                    {
                        dir &= ~(1 << UP);
                    }
                    else if (code == (DOWN_KEY|IECODE_UP_PREFIX))
                    {
                        dir &= ~(1 << DOWN);
                    }
                    prevSeconds = seconds;

                    if (!pause && (dir & (1 << LEFT)) == (1 << LEFT))
                    {
                        moveHero(gs, gi, -1, 0);
                        pause = TRUE;
                        counter = 0;
                    }
                    else if (!pause && (dir & (1 << RIGHT)) == (1 << RIGHT))
                    {
                        moveHero(gs, gi, 1, 0);
                        pause = TRUE;
                        counter = 0;
                    }
                    else if (!pause && (dir & (1 << UP)) == (1 << UP))
                    {
                        moveHero(gs, gi, 0, -1);
                        pause = TRUE;
                        counter = 0;
                    }
                    else if (!pause && (dir & (1 << DOWN)) == (1 << DOWN))
                    {
                        moveHero(gs, gi, 0, 1);
                        pause = TRUE;
                        counter = 0;
                    }

                    /* while (msg = (struct IntuiMessage *)GetMsg(gi->w->UserPort))
                    {
                        ReplyMsg((struct Message *)msg);
                    }*/
                }
                else if (class == IDCMP_REFRESHWINDOW)
                {
                    struct Rectangle bounds;
                    struct Window *w = gi->w;
                    struct copperData *cd = &gi->copdata;

                    BeginRefresh(w);
                    GetRPAttrs(w->RPort, RPTAG_DrawBounds, &bounds, TAG_DONE);

                    drawBoard(gi, w->RPort, b, gi->bm[1], gi->gfx, bounds.MinX >> 4, bounds.MinY >> 4, bounds.MaxX >> 4, bounds.MaxY >> 4);

                    SetSignal(0L, 1L << cd->signal);
                    Wait(1L << cd->signal);

                    drawTile(gi->bm[1], bounds.MinX, bounds.MinY, gi->bm[0], bounds.MinX, bounds.MinY, bounds.MaxX - bounds.MinX + 1, bounds.MaxY - bounds.MinY + 1);
                    EndRefresh(w, TRUE);
                }
            }
        }
    }
}

WORD loadBoard(STRPTR name, struct gameState *gs)
{
    struct IFFHandle *iff;
    WORD result = LOAD_FAILURE;
    struct Board *b = &gs->board;

    D(bug("Loading %s\n", name));

    if (iff = openIFF(name, IFFF_READ))
    {
        if (!PropChunk(iff, ID_MAGA, ID_NAGL))
        {
            if (!PropChunk(iff, ID_MAGA, ID_STAN))
            {
                if (!StopChunk(iff, ID_MAGA, ID_PLAN))
                {
                    if (!StopOnExit(iff, ID_MAGA, ID_FORM))
                    {
                        LONG err = ParseIFF(iff, IFFPARSE_SCAN);
                        if (err == 0 || err == IFFERR_EOC || err == IFFERR_EOF)
                        {
                            struct StoredProperty *sp;
                            if (gs && (sp = FindProp(iff, ID_MAGA, ID_STAN)))
                            {
                                gs->version = ((struct gameState *)sp->sp_Data)->version;
                                if (gs->version == VERSION)
                                {
                                    result = STATE_LOADED;
                                    D(bug("State read: %d\n", gs->level));
                                }
                                else
                                {
                                    result = LOAD_FAILURE;
                                    gs->level = 1;
                                }
                            }
                            if (sp = FindProp(iff, ID_MAGA, ID_NAGL))
                            {
                                if (ReadChunkBytes(iff, b->board, sizeof(b->board)) == sizeof(b->board))
                                {
                                    result = LEVEL_LOADED;
                                    D(bug("Game version: %d\n", b->version));
                                }
                            }
                        }
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(result);
}

BOOL saveBoard(STRPTR name, struct gameState *gs)
{
    struct IFFHandle *iff;
    BOOL result = FALSE;
    struct Board *b = &gs->board;

    if (iff = openIFF(name, IFFF_WRITE))
    {
        if (!PushChunk(iff, ID_MAGA, ID_FORM, IFFSIZE_UNKNOWN))
        {
            if (gs)
            {
                if (!PushChunk(iff, ID_MAGA, ID_STAN, sizeof(*gs)))
                {
                    gs->version = VERSION;
                    D(bug("Writing state (level %d)\n", gs->level));
                    if (WriteChunkBytes(iff, gs, sizeof(*gs)) == sizeof(*gs))
                    {
                        if (!PopChunk(iff))
                        {
                            result = TRUE;
                        }
                    }
                }
            }
            else
                result = TRUE;
            if (result && (!PushChunk(iff, ID_MAGA, ID_NAGL, sizeof(ULONG))))
            {
                if (WriteChunkBytes(iff, &b->version, sizeof(ULONG)) == sizeof(ULONG))
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
                                        result = TRUE;
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

void printLevel(struct Window *w, struct windowInfo *wi, struct gameState *gs)
{
    ULONG lock;

    lock = LockIBase(0);
    sprintf(wi->text[GID_MENU2].IText, "Edytor %03d", gs->level);
    UnlockIBase(lock);

    RefreshGList(wi->gads + GID_MENU2, w, NULL, 1);
}

WORD editBoard(struct gameInit *gi)
{
    struct gameState *gs = &gi->state;
    struct Window *w = gi->w;
    struct copperData *cd = &gi->copdata;
    struct BitMap **bm = gi->bm;
    struct BitMap *gfx = gi->gfx;
    struct windowInfo *wi = &gi->wi;
    struct reqInfo *ri = &gi->ri;

    ULONG signals[ 2 ] = { 0 };
    BOOL done = FALSE;
    WORD tilex = 0, tiley = 1 + 8;
    struct Requester req;

    BOOL paint = FALSE;
    WORD oldx = -1, oldy = -1;

    ULONG total;

    signals[0] = 1L << w->UserPort->mp_SigBit;
    signals[1] = 1L << cd->signal;

    total = signals[0]|signals[1];

    RemoveGList(w, wi->gads, -1);
    initEditTexts(wi->text);
    AddGList(w, wi->gads, -1, -1, NULL);
    RefreshGList(wi->gads, w, NULL, -1);

    InitRequester(&req);

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

                    if (gad->GadgetID == GID_CLOSE)
                    {
                        return(RESULT_QUIT);
                    }
                    else if (gad->GadgetID == GID_MENU1)
                    {
                        /*struct Window *menu;

                        Request(&req, w);

                        if (menu = openMenuWindow(w, 0, 64, 80, NULL))
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
*/
                        EndRequest(&req, w);
                    }
                    else if (gad->GadgetID == GID_MENU2)
                    {
                        UBYTE name[NAME];

                        sprintf(name, "Data1/Levels/Level%03d.lev", gs->level);

                        if (loadBoard(name, gs))
                        {
                            drawBoard(gi, w->RPort, &gs->board, gi->bm[1], gi->gfx, 0, 1, 19, 14);

                            SetSignal(0L, 1L << gi->copdata.signal);
                            Wait(1L << gi->copdata.signal);
                            drawTile(gi->bm[1], 0, 1, gi->bm[0], 0, 1, 320, 224);
                        }
                    }
                    else if (gad->GadgetID == GID_MENU3)
                    {
                        UBYTE name[NAME];

                        sprintf(name, "Data1/Levels/Level%03d.lev", gs->level);

                        if (saveBoard(name, gs))
                        {
                        }
                    }
                    else if (gad->GadgetID == GID_MENU4)
                    {
                        /* return(RESULT_QUIT); */
                    }
                    else if (gad->GadgetID == GID_MENU5)
                    {
                        return(RESULT_PLAY);
                    }
                }
                else if (class == IDCMP_REFRESHWINDOW)
                {
                    struct Rectangle bounds;

                    BeginRefresh(w);
                    GetRPAttrs(w->RPort, RPTAG_DrawBounds, &bounds, TAG_DONE);

                    drawBoard(gi, w->RPort, &gs->board, bm[1], gfx, bounds.MinX >> 4, bounds.MinY >> 4, bounds.MaxX >> 4, bounds.MaxY >> 4);

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
                            struct Cell *c = &gs->board.board[my >> 4][mx >> 4];
                            BltBitMap(gfx, tilex << 4, tiley << 4, bm[0], mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0, 0xff, NULL);
                            c->kind = tiley - 8;
                            c->subKind = tilex;

                            if (c->kind == FLOOR_KIND)
                                c->floor = c->subKind;
                            else
                                c->floor = NORMAL_FLOOR;

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
                        struct Cell *c = &gs->board.board[my >> 4][mx >> 4];
                        c->kind = tiley - 8;
                        c->subKind = tilex;

                        if (c->kind == FLOOR_KIND)
                            c->floor = c->subKind;
                        else
                            c->floor = NORMAL_FLOOR;

                        BltBitMap(gfx, tilex << 4, tiley << 4, bm[0], mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0, 0xff, NULL);
                        oldx = mx >> 4;
                        oldy = my >> 4;
                    }
                }
            }
        }
    }

    saveBoard("T:Temp.bak", gs);
    return(RESULT_QUIT);
}

/* Funkcja main */

int main(void)
{
    extern void mainMenu(void);

    if (initGame(&gameInit))
    {
        /* Otwieramy menu gîówne gry */

        mainMenu();

        freeGame(&gameInit);
        return(RETURN_OK);
    }
    return(RETURN_FAIL);

}

/* Menu gîówne gry */

WORD startGame();
WORD loadGame();
WORD beginGame();
WORD handleMenu(void);
void addMenuItem(WORD item);

void mainMenu(void)
{
    /* Dodajemy opcje, wyôwietlamy menu i uruchamiamy obsîugë menu */
    WORD result;

    BltBitMapRastPort(gameInit.gfx, 0, 128, gameInit.w->RPort, 0, 0, 192, 128, 0xc0);

    do
    {
        currentOption = 0;

        addMenuItem(GAME_OPTIONS);
        addMenuItem(BEGIN_GAME);
        addMenuItem(LOAD_GAME);
        addMenuItem(SHOW_PROGRESS); /* Postëp w przechodzeniu gry */
        addMenuItem(SHOW_HISCORE);
        addMenuItem(SHOW_AUTHORS);
        addMenuItem(QUIT_GAME);

        switch (result = handleMenu())
        {
            case QUIT_GAME:
                break;

            case BEGIN_GAME:
                startGame();
                beginGame();
                break;

            case LOAD_GAME:
                loadGame();
                beginGame();
                break;

            case SHOW_PROGRESS:
                break;

            case SHOW_HISCORE:
                break;

            case SHOW_AUTHORS:
                break;

            case GAME_OPTIONS:
                break;
        }
    }
    while (result != QUIT_GAME);
}

void addMenuItem(WORD item)
{
    /* Tutaj dodajemy opcjë do menu */
    initMenuText(gameInit.intuiTexts + currentOption, itemStrings[item]);
    initMenuButton(gameInit.menuItems + currentOption, gameInit.intuiTexts + currentOption, item, 80, 8 + (currentOption << 4), gameInit.img);

    if (currentOption > 0)
    {
        gameInit.menuItems[currentOption - 1].NextGadget = &gameInit.menuItems[currentOption];
    }

    currentOption++;
}

void showMenu(void)
{
    /* Tutaj wyôwietlamy bieûâce menu */

    if (currentOption < lastOptions)
    {
        BltBitMapRastPort(gameInit.gfx, 80, 128 + 8 + (currentOption << 4), gameInit.w->RPort, 80, 8 + (currentOption << 4), 80, (lastOptions - currentOption) << 4, 0xc0);
    }
    lastOptions = currentOption;

    RefreshGList(gameInit.menuItems, gameInit.w, NULL, -1);
}

WORD handleMenu(void)
{
    WORD result = FALSE;
    struct IntuiMessage *msg;
    WORD option = -1;
    /* Tutaj obsîugujemy bieûâce menu */

    AddGList(gameInit.w, gameInit.menuItems, -1, -1, NULL);

    showMenu();

    while (option == -1)
    {
        WaitPort(gameInit.w->UserPort);

        while (msg = (struct IntuiMessage *)GetMsg(gameInit.w->UserPort))
        {
            ULONG class = msg->Class;
            struct Gadget *gad = (struct Gadget *)msg->IAddress;

            ReplyMsg((struct Message *)msg);

            if (class == IDCMP_GADGETUP)
            {
                option = gad->GadgetID;
            }
        }
    }

    RemoveGList(gameInit.w, gameInit.menuItems, -1);
    return(option);
}

/* Rozpocznij od poziomu nr 1 */
WORD startGame()
{
    initBoard(&gameInit.state.board);
}

/* Wczytaj stan gry */
WORD loadGame()
{
    if (!loadBoard("Data1/Levels/Level001.lev", &gameInit.state))
    {
        startGame();
    }
}

WORD beginGame()
{
    WORD option;

    drawBoard(&gameInit, gameInit.backw->RPort, &gameInit.state.board, gameInit.bm[1], gameInit.gfx, 0, 0, 19, 15);

    SetSignal(0L, 1L << gameInit.copdata.signal);
    Wait(1L << gameInit.copdata.signal);

    BltBitMapRastPort(gameInit.bm[1], 0, 0, gameInit.backw->RPort, 0, 0, 320, 256, 0xc0);

    do
    {
        currentOption = 0;

        addMenuItem(PLAY_GAME);
        addMenuItem(EDIT_LEVEL);
        addMenuItem(PREV_LEVEL);
        addMenuItem(NEXT_LEVEL);
        addMenuItem(EXIT_TO_MENU);

        switch (option = handleMenu())
        {
            default:
        }
    }
    while (option != EXIT_TO_MENU);
    return(option);
}
