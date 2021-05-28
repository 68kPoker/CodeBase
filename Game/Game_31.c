
/* $Log$ */

#include <stdio.h>

#include <dos/dos.h>
#include <intuition/intuition.h>
#include <libraries/iffparse.h>
#include <graphics/rpattr.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/iffparse_protos.h>

#include "Windows.h"
#include "IFF.h"
#include "Copper.h"
#include "Gadgets.h"

#include "debug.h"

enum
{
    LEFT,
    RIGHT,
    UP,
    DOWN
};

#define LEFT_FLAG  1 << LEFT
#define RIGHT_FLAG 1 << RIGHT
#define UP_FLAG    1 << UP
#define DOWN_FLAG  1 << DOWN

BOOL allocScreenBuffers(struct Screen *s, struct ScreenBuffer *sb[], struct MsgPort **mp, UBYTE *safe, UBYTE *frame);
VOID freeScreenBuffers(struct Screen *s, struct ScreenBuffer *sb[], struct MsgPort *mp, BOOL safe);

VOID updateCursor(struct Window *bw, WORD cx, WORD cy);
VOID drawPalette(struct Window *bw, WORD min, WORD max);

VOID safeToDraw(struct Screen *s);

struct TextAttr ta =
{
    "centurion.font", 9, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED
};

struct Screen *openScreen(STRPTR title, struct screenData *sd)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };
    struct Screen    *s;
    VOID prepScreen(struct Screen *s);

    const ULONG      modeID = LORES_KEY;
    const UBYTE      depth  = 5;

    if (sd->font = OpenDiskFont(&ta))
    {
        s = OpenScreenTags(NULL,
            SA_Font,        &ta,
            SA_DClip,       &dclip,
            SA_Depth,       depth,
            SA_DisplayID,   modeID,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_Draggable,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Title,       title,
            SA_Behind,      TRUE,
            SA_Interleaved, TRUE,
            TAG_DONE);

        if (s == NULL)
        {
            printf("Couldn't open screen!\n");
        }
        else
        {
            if (allocScreenBuffers(s, sd->sb, &sd->mp, &sd->safe, &sd->frame))
            {
                if (addCopperList(&s->ViewPort))
                {
                    if (addCopperInt(&sd->is, &sd->cd, &s->ViewPort))
                    {
                        if (sd->update[0] = NewRegion())
                        {
                            if (sd->update[1] = NewRegion())
                            {
                                s->UserData = (APTR)sd;
                                prepScreen(s);
                                return(s);
                            }
                            DisposeRegion(sd->update[0]);
                        }
                        remCopperInt(&sd->is);
                    }
                }
                freeScreenBuffers(s, sd->sb, sd->mp, sd->safe);
            }
            CloseScreen(s);
        }
        CloseFont(sd->font);
    }
    return(NULL);
}

VOID closeScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    DisposeRegion(sd->update[1]);
    DisposeRegion(sd->update[0]);

    remCopperInt(&sd->is);
    freeScreenBuffers(s, sd->sb, sd->mp, sd->safe);
    CloseScreen(s);
    CloseFont(sd->font);
}

BOOL allocScreenBuffers(struct Screen *s, struct ScreenBuffer *sb[], struct MsgPort **mp, UBYTE *safe, UBYTE *frame)
{
    if (sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
    {
        if (sb[1] = AllocScreenBuffer(s, NULL, 0))
        {
            if (*mp = CreateMsgPort())
            {
                sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = *mp;
                sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = *mp;

                *safe  = TRUE;
                *frame = 0;

                return(TRUE);
            }
            FreeScreenBuffer(s, sb[1]);
        }
        FreeScreenBuffer(s, sb[0]);
    }
    return(FALSE);
}

VOID freeScreenBuffers(struct Screen *s, struct ScreenBuffer *sb[], struct MsgPort *mp, BOOL safe)
{
    if (!safe)
    {
        while (!GetMsg(mp))
        {
            WaitPort(mp);
        }
    }
    DeleteMsgPort(mp);
    FreeScreenBuffer(s, sb[1]);
    FreeScreenBuffer(s, sb[0]);
}

struct Window *openWindow(WORD kind, struct Screen *s, struct winData *wd)
{
    struct Window *w = NULL;
    struct screenData *sd = (struct screenData *)s->UserData;
    struct Rectangle r;
    wd->kind = kind;

    switch (kind)
    {
        case WIN_BOARD:
            w = OpenWindowTags(NULL,
                WA_CustomScreen,    s,
                WA_Left,            0,
                WA_Top,             0,
                WA_Width,           320,
                WA_Height,          256,
                WA_AutoAdjust,      FALSE,
                WA_Backdrop,        TRUE,
                WA_Borderless,      TRUE,
                WA_RMBTrap,         TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_ReportMouse,     TRUE,
                WA_BackFill,        LAYERS_NOBACKFILL,
                WA_IDCMP,           IDCMP_REFRESHWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW,
                TAG_DONE);

            break;

        case WIN_PANEL:
            w = OpenWindowTags(NULL,
                WA_CustomScreen,    s,
                WA_Left,            0,
                WA_Top,             240,
                WA_Width,           320,
                WA_Height,          16,
                WA_AutoAdjust,      FALSE,
                WA_Backdrop,        TRUE,
                WA_Borderless,      TRUE,
                WA_RMBTrap,         TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_BackFill,        LAYERS_NOBACKFILL,
                WA_IDCMP,           IDCMP_REFRESHWINDOW|IDCMP_GADGETUP,
                TAG_DONE);

        case WIN_STATUS:
            break;
        case WIN_REQ:
            w = OpenWindowTags(NULL,
                WA_CustomScreen,    s,
                WA_Left,            64,
                WA_Top,             64,
                WA_Width,           192,
                WA_Height,          128,
                WA_AutoAdjust,      FALSE,
                WA_Backdrop,        FALSE,
                WA_Borderless,      TRUE,
                WA_RMBTrap,         TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_ReportMouse,     TRUE,
                WA_BackFill,        LAYERS_NOBACKFILL,
                WA_Activate,        TRUE,
                WA_IDCMP,           IDCMP_REFRESHWINDOW|IDCMP_GADGETUP,
                TAG_DONE);
        default:
    }

    if (w == NULL)
    {
        printf("Couldn't open window!\n");
    }
    else
    {
        w->UserData = (APTR)wd;
        r.MinX = w->LeftEdge;
        r.MinY = w->TopEdge;
        r.MaxX = r.MinX + w->Width - 1;
        r.MaxY = r.MinY + w->Height - 1;

        OrRectRegion(sd->update[sd->frame], &r);
    }
    return(w);
}

VOID closeWindow(struct Window *w)
{
    CloseWindow(w);
}

VOID drawTile(struct Window *bw, WORD dstx, WORD dsty, WORD srcx, WORD srcy, BOOL single)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct screenData *sd = (struct screenData *)bw->WScreen->UserData;
    struct Rectangle bounds = { dstx << 4, dsty << 4, (dstx << 4) + 15, (dsty << 4) + 15 };

    safeToDraw(bw->WScreen);
    BltBitMapRastPort(bd->gfx, srcx << 4, srcy << 4, bw->RPort, dstx << 4, dsty << 4, 16, 16, 0xc0);

    if (single)
        OrRectRegion(sd->update[sd->frame], &bounds);
}

VOID drawFrame(struct Window *w, WORD cx, WORD cy)
{
    struct RastPort *rp = w->RPort;
    struct screenData *sd = (struct screenData *)w->WScreen->UserData;
    struct Rectangle bounds = { cx << 4, cy << 4, (cx << 4) + 15, (cy << 4) + 15 };

    Move(rp, cx << 4, cy << 4);
    Draw(rp, (cx << 4) + 15, cy << 4);
    Draw(rp, (cx << 4) + 15, (cy << 4) + 15);
    Draw(rp, cx << 4, (cy << 4) + 15);
    Draw(rp, cx << 4, (cy << 4) + 1);

    OrRectRegion(sd->update[sd->frame], &bounds);
}

VOID drawCursor(struct Window *bw)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct RastPort *rp = bw->RPort;
    WORD cx = bd->cursorX, cy = bd->cursorY;

    if (cy >= 1)
    {
        drawTile(bw, cx, cy, bd->currentTile.subKind, bd->currentTile.kind, TRUE);

        SetAPen(rp, 28);
    }
    else if (cx - 1 != bd->pal)
    {
        SetAPen(rp, 30);
    }
    else
    {
        SetAPen(rp, 19);
    }

    drawFrame(bw, cx, cy);

    bd->cursor = TRUE;
}

VOID discardCursor(struct Window *bw)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct RastPort *rp = bw->RPort;
    WORD cx = bd->cursorX, cy = bd->cursorY;
    TILE *t = &bd->board[cy][cx];

    if (bd->cursor)
    {
        if (cy >= 1)
        {
            drawTile(bw, cx, cy, t->subKind, t->kind, TRUE);
        }
        else if (cx - 1 != bd->pal)
        {
            drawPalette(bw, cx - 1, cx - 1);
        }
        else
        {
            SetAPen(bw->RPort, 19);
            drawFrame(bw, cx, 0);
        }
        bd->cursor = FALSE;
    }
}

VOID safeToDraw(struct Screen *s)
{
    VOID prepScreen(struct Screen *s);
    struct screenData *sd = (struct screenData *)s->UserData;

    if (!sd->safe)
    {
        while (!GetMsg(sd->mp))
        {
            WaitPort(sd->mp);
        }
        sd->safe = TRUE;
        /* prepScreen(s); */
    }
}

VOID updateScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;
    WORD frame = sd->frame;
    struct Region **update = sd->update;
    struct RegionRectangle *rr;
    struct Rectangle *rect = &update[frame ^ 1]->bounds;
    struct BitMap *srcbm = sd->sb[frame ^ 1]->sb_BitMap,
        *destbm = sd->sb[frame]->sb_BitMap;
    struct Rectangle b = { 0, 0, s->Width - 1, s->Height - 1 };

    OrRegionRegion(update[frame], update[frame ^ 1]);
    XorRegionRegion(update[frame], update[frame ^ 1]);

    AndRectRegion(update[frame ^ 1], &b);

    for (rr = update[frame ^ 1]->RegionRectangle; rr != NULL; rr = rr->Next)
    {
        struct Rectangle *r = &rr->bounds;
        WORD x = rect->MinX + r->MinX;
        WORD y = rect->MinY + r->MinY;
        WORD width = r->MaxX - r->MinX + 1;
        WORD height = r->MaxY - r->MinY + 1;

        BltBitMap(srcbm, x, y, destbm, x, y, width, height, 0xc0, 0xff, NULL);
        /* printf( "[%3d %3d %3d %3d]\n", x, y, width, height); */
    }
    ClearRegion(update[frame ^ 1]);
}

VOID prepScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;
    struct Window *w;

    WORD frame = sd->frame ^= 1;
    s->RastPort.BitMap = sd->sb[frame]->sb_BitMap;
    for (w = s->FirstWindow; w != NULL; w = w->NextWindow)
    {
        w->RPort->BitMap = sd->sb[frame]->sb_BitMap;
    }
}

VOID changeScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    safeToDraw(s);

    if (sd->safe)
    {
        UBYTE frame = sd->frame;
        updateScreen(s);
        WaitBlit();
        while (!ChangeScreenBuffer(s, sd->sb[frame]))
        {
            WaitTOF();
        }
        sd->safe = FALSE;
        prepScreen(s);
    }
}

VOID drawPalette(struct Window *bw, WORD min, WORD max)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    WORD x;
    struct position
    {
        WORD x, y;
    } pos[] =
    {
        { 0, 0 },
        { 0, 1 },
        { 0, 2 },
        { 1, 2 },
        { 1, 0 },
        { 1, 1 },
        { 0, 3 }
    };

    for (x = min; x <= max; x++)
    {
        drawTile(bw, x + 1, 0, pos[x].x, pos[x].y, FALSE);
        if (x == bd->pal)
        {
            SetAPen(bw->RPort, 19);
            drawFrame(bw, x + 1, 0);
        }
    }
}

VOID clearBoard(struct Window *bw)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    WORD x, y;
    TILE *t;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            t = &bd->board[y][x];
            if (x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
            {
                t->kind = TILE_WALL;
                t->subKind = WALL_NORMAL;
            }
            else
            {
                t->kind = TILE_FLOOR;
                t->subKind =  FLOOR_NORMAL;
            }
        }
    }
}

VOID drawBoard(struct Window *bw)
{
    WORD x, y;
    TILE *t;
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct Rectangle bounds;
    struct screenData *sd = (struct screenData *)bw->WScreen->UserData;

    GetRPAttrs(bw->RPort, RPTAG_DrawBounds, &bounds, TAG_DONE);

    OrRectRegion(sd->update[sd->frame], &bounds);

    for (y = 1; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            t = &bd->board[y][x];

            drawTile(bw, x, y, t->subKind, t->kind, FALSE);
        }
    }
}

BOOL scanBoard(struct Window *bw)
{
    WORD x, y;
    TILE *t;
    struct boardData *bd = (struct boardData *)bw->UserData;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            t = &bd->board[y][x];

            if (t->kind == TILE_OBJECT && t->subKind == OBJ_BOX)
            {
                bd->boxCount++;
            }
        }
    }
}

VOID initBoard(struct Window *bw)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct screenData *sd = (struct screenData *)bw->WScreen->UserData;

    bd->cursorX = bd->cursorY = 1;
    bd->currentTile.kind = TILE_OBJECT;
    bd->currentTile.subKind = OBJ_HERO;
    bd->currentTile.floor = FLOOR_NORMAL;

    /* bw->RPort->BitMap = sd->sb[sd->frame]->sb_BitMap; */

    safeToDraw(bw->WScreen);

    bd->pal = PAL_HERO;

    drawPalette(bw, 0, PALS - 1);
    drawBoard(bw);

    bd->cursor = FALSE;
    bd->paste = FALSE;
    bd->dir = 0;
    bd->angle = 0;
    bd->points = 0;
    bd->placed = 0;
    bd->x = bd->y = 1;

    /* updateCursor(bw, bw->MouseX >> 4, bw->MouseY >> 4); */
}

VOID convertBoard(struct boardData *bd)
{
    WORD x, y;
    TILE *t;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            t = &bd->board[y][x];
        }
    }
}

BOOL loadBoard(STRPTR name, struct Window *bw, struct Window *pw)
{
    struct IFFHandle *iff;
    BOOL result = FALSE;
    struct boardData *bd = (struct boardData *)bw->UserData;

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
                            if (sp = FindProp(iff, ID_MAGA, ID_STAN))
                            {
                                D(bug("STAN chunk found.\n"));
                            }
                            if (sp = FindProp(iff, ID_MAGA, ID_NAGL))
                            {
                                ULONG version = *(ULONG *)sp->sp_Data;
                                D(bug("NAGL chunk found.\n"));
                                D(bug("Game version: %d\n", version));

                                if (version == 1)
                                {
                                    ULONG actual;
                                    actual = ReadChunkBytes(iff, bd->board, sizeof(bd->board));
                                    D(bug("Read %d bytes.\n", actual));
                                    convertBoard(bd);
                                    result = TRUE;
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

VOID initPanel(struct Window *pw)
{
    struct panelData *pd = (struct panelData *)pw->UserData;
    struct BitMap *gfx = pd->gfx;
    WORD i;

    cutImage(pd->images + IMG_BUTTON, gfx, 0, 64, 64, 16);
    cutImage(pd->images + IMG_PUSHED, gfx, 0, 80, 64, 16);

    initText(pd->texts + PAN_NEWGAME, "Nowa gra");
    initText(pd->texts + PAN_LOADGAME, "Wczytaj grë");
    initText(pd->texts + PAN_SAVEGAME, "Zapisz gra");
    initText(pd->texts + PAN_RESTART, "Restartuj");
    initText(pd->texts + PAN_QUIT, "Koniec");

    initButton(pd->gads + PAN_NEWGAME, pd->texts + PAN_NEWGAME, PAN_NEWGAME, 64 * 0, 0, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);
    initButton(pd->gads + PAN_LOADGAME, pd->texts + PAN_LOADGAME, PAN_LOADGAME, 64 * 1, 0, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);
    initButton(pd->gads + PAN_SAVEGAME, pd->texts + PAN_SAVEGAME, PAN_SAVEGAME, 64 * 2, 0, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);
    initButton(pd->gads + PAN_RESTART, pd->texts + PAN_RESTART, PAN_RESTART, 64 * 3, 0, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);
    initButton(pd->gads + PAN_QUIT, pd->texts + PAN_QUIT, PAN_QUIT, 64 * 4, 0, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);

    for (i = 0; i < PAN_BUTTONS - 1; i++)
    {
        pd->gads[i].NextGadget = pd->gads + i + 1;
    }
    pd->gads[i].NextGadget = NULL;
}

VOID initReq(struct Window *rw, struct Window *pw)
{
    struct reqData *rd = (struct reqData *)rw->UserData;
    struct panelData *pd = (struct panelData *)pw->UserData;
    WORD i;

    initText(rd->texts + REQ_OK, "OK");
    initText(rd->texts + REQ_CANCEL, "Anuluj");

    initButton(rd->gads + REQ_OK, rd->texts + REQ_OK, REQ_OK, 16, 112, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);
    initButton(rd->gads + REQ_CANCEL, rd->texts + REQ_CANCEL, REQ_CANCEL, 112, 112, pd->images + IMG_BUTTON, pd->images + IMG_PUSHED);

    for (i = 0; i < REQ_BUTTONS - 1; i++)
    {
        rd->gads[i].NextGadget = rd->gads + i + 1;
    }
    rd->gads[i].NextGadget = NULL;
}

VOID freePanel(struct Window *pw)
{
    struct panelData *pd = (struct panelData *)pw->UserData;

    RemoveGList(pw, pd->gads, -1);

    freeImage(pd->images + IMG_PUSHED);
    freeImage(pd->images + IMG_BUTTON);
}

VOID updateCursor(struct Window *bw, WORD cx, WORD cy)
{
    struct boardData *bd = (struct boardData *)bw->UserData;

    if (cx >= 1 && cx <= BOARD_WIDTH - 2 && cy <= BOARD_HEIGHT - 2)
    {
        if (cy >= 1 || cx <= PALS)
        {
            if (!bd->cursor || cx != bd->cursorX || cy != bd->cursorY)
            {
                if (bd->cursor)
                {
                    discardCursor(bw);
                }
                bd->cursorX = cx;
                bd->cursorY = cy;
                drawCursor(bw);
                return;
            }
        }
        else
            discardCursor(bw);
    }
    else
        discardCursor(bw);
}

VOID rotateHero(struct Window *bw, WORD angle, WORD dx, WORD dy)
{
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct screenData *sd = (struct screenData *)bw->WScreen->UserData;
    WORD destdir, sign, diff;

    if (dx == -1)
        destdir = 0;
    else if (dy == -1)
        destdir = 2;
    else if (dx == 1)
        destdir = 4;
    else if (dy == 1)
        destdir = 6;

    diff = destdir - angle;

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

    while (angle != destdir)
    {
        WORD tilex, tiley;

        angle += sign;

        if (angle < 0)
        {
            angle += 8;
        }

        if (angle >= 8)
        {
            angle -= 8;
        }

        switch (angle)
        {
            case 4:
                tilex = 5;
                tiley = 2;
                break;

            case 5:
                tilex = 5;
                tiley = 3;
                break;

            case 0:
                tilex = 3;
                tiley = 2;
                break;

            case 1:
                tilex = 3;
                tiley = 1;
                break;

            case 6:
                tilex = 4;
                tiley = 3;
                break;

            case 7:
                tilex = 3;
                tiley = 3;
                break;

            case 2:
                tilex = 4;
                tiley = 1;
                break;

            case 3:
                tilex = 5;
                tiley = 1;
                break;

        }

        drawTile(bw, bd->x, bd->y, tilex, tiley, TRUE);

        SetSignal(0L, 1L << sd->cd.signal);
        Wait(1L << sd->cd.signal);
        SetSignal(0L, 1L << sd->cd.signal);
        Wait(1L << sd->cd.signal);
    }
    bd->angle = angle;
}

BOOL moveHero(struct Window *bw, WORD dx, WORD dy)
{
    struct boardData *bd = (struct boardData *)bw->UserData;

    BOOL pushbox = FALSE;

    WORD x = bd->x, y = bd->y; /* Hero position */

    TILE *c = &bd->board[y][x], *dest = &bd->board[y + dy][x + dx];

    TILE *past = &bd->board[y + dy + dy][x + dx + dx];

    /* WORD snd = SAMPLE_DIG; */

    rotateHero(bw, bd->angle, dx, dy);

    if (dest->kind == TILE_WALL)
    {
        if (dest->subKind != WALL_DOOR || bd->keys == 0)
        {
            return(FALSE);
        }
        /* snd = SAMPLE_FRUIT; */
        bd->keys--;

        /* playSample(gi->ioa, gi->samples + snd, SNDCHAN); */

        dest->kind = TILE_FLOOR;
        dest->subKind = FLOOR_NORMAL;

        BltBitMapRastPort(bd->gfx, dest->subKind << 4, dest->kind << 4, bw->RPort, (x + dx) << 4, (y + dy) << 4, 16, 16, 0xc0);

        /* updateStatus(gi->w->RPort, b, &gi->state); */

        return(FALSE);
    }

    if (dest->kind == TILE_ITEM)
    {
        if (dest->subKind == ITEM_KEY)
        {
            bd->keys++;
            /* snd = SAMPLE_KEY; */
        }
        else if (dest->subKind == ITEM_FRUIT)
        {
            /* snd = SAMPLE_FRUIT; */
            bd->points += 250;
        }
    }

    if (dest->kind == TILE_OBJECT)
    {
        if (dest->subKind != OBJ_BOX)
        {
            return(FALSE);
        }

        if (past->kind != TILE_FLOOR && !(past->kind == TILE_WALL && past->subKind == WALL_MUD))
        {
            return(FALSE);
        }

        /* snd = SAMPLE_BOX; */

        if (past->floor == FLOOR_FLAGSTONE)
        {
            /* tilex = OBJ_PLACEDBOX; */
            bd->placed++;
        }

        if (dest->floor == FLOOR_FLAGSTONE)
        {
            bd->placed--;
        }

        if (past->kind == TILE_WALL && past->subKind == WALL_MUD)
        {
            past->subKind = FLOOR_FILLEDMUD;
            past->floor = FLOOR_FILLEDMUD;
            past->kind = TILE_FLOOR;
            pushbox = TRUE;

            /* snd = SAMPLE_FRUIT; */
        }
        else
        {
            past->kind = TILE_OBJECT;
            past->subKind = dest->subKind;
            pushbox = TRUE;
        }
    }

    dest->kind = TILE_OBJECT;
    dest->subKind = c->subKind;

    c->kind = TILE_FLOOR;
    c->subKind = c->floor;

    drawTile(bw, x, y, c->subKind, c->kind, TRUE);

    WORD tilex, tiley;

    switch (bd->angle)
    {
        case 4:
            tilex = 5;
            tiley = 2;
            break;

        case 5:
            tilex = 5;
            tiley = 3;
            break;

        case 0:
            tilex = 3;
            tiley = 2;
            break;

        case 1:
            tilex = 3;
            tiley = 1;
            break;

        case 6:
            tilex = 4;
            tiley = 3;
            break;

        case 7:
            tilex = 3;
            tiley = 3;
            break;

        case 2:
            tilex = 4;
            tiley = 1;
            break;

        case 3:
            tilex = 5;
            tiley = 1;
            break;
    }


    drawTile(bw, x + dx, y + dy, tilex, tiley, TRUE);
    BltBitMapRastPort(bd->gfx, tilex << 4, tiley << 4, bw->RPort, (x + dx) << 4, (y + dy) << 4, 16, 16, 0xc0);

    if (pushbox)
    {
        drawTile(bw, x + dx + dx, y + dy + dy, past->subKind, past->kind, TRUE);
    }

    bd->x += dx;
    bd->y += dy;

    /*if (snd != SAMPLE_DIG)
        playSample(gi->ioa, gi->samples + snd, SNDCHAN); */

    if (bd->points > 0)
    {
        bd->points--;
    }

    /* updateStatus(gi->w->RPort, b, &gi->state); */


/*
    if (info->placed > 0 && info->placed == info->boxes)
    {
        dispMessageBox(gi, "OK", "Kontynuuj", standardMessage);
    }
*/

    return(TRUE);
}

LONG handleBoard(struct Window *bw)
{
    struct IntuiMessage *msg;
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct Screen *s = bw->WScreen;
    struct screenData *sd = (struct screenData *)s->UserData;
    WORD frame = sd->frame;
    WORD dir = 0;

    while (msg = (struct IntuiMessage *)GetMsg(bw->UserPort))
    {
        ULONG class = msg->Class;
        WORD code = msg->Code;
        WORD mx = msg->MouseX;
        WORD my = msg->MouseY;

        WORD cx = mx >> 4, cy = my >> 4;

        ReplyMsg((struct Message *)msg);

        if (class == IDCMP_RAWKEY)
        {
            if (code == LEFT_KEY)
            {
                dir |= LEFT_FLAG;
            }
            else if (code == RIGHT_KEY)
            {
                dir |= RIGHT_FLAG;
            }
            else if (code == UP_KEY)
            {
                dir |= UP_FLAG;
            }
            else if (code == DOWN_KEY)
            {
                dir |= DOWN_FLAG;
            }
            if (code == (LEFT_KEY|IECODE_UP_PREFIX))
            {
                dir &= ~LEFT_FLAG;
            }
            else if (code == (RIGHT_KEY|IECODE_UP_PREFIX))
            {
                dir &= ~RIGHT_FLAG;
            }
            else if (code == (UP_KEY|IECODE_UP_PREFIX))
            {
                dir &= ~UP_FLAG;
            }
            else if (code == (DOWN_KEY|IECODE_UP_PREFIX))
            {
                dir &= ~DOWN_FLAG;
            }
            else if (code == ESC_KEY)
            {
                return(WINACT_ESC);
            }
            bd->dir = dir;
        }
        else if (class == IDCMP_MOUSEMOVE)
        {
            updateCursor(bw, mx >> 4, my >> 4);

            if (bd->paste && !(cx == bd->x && cy == bd->y))
            {
                TILE *t = &bd->currentTile;

                bd->board[cy][cx] = *t;
            }
        }
        else if (class == IDCMP_REFRESHWINDOW)
        {
            safeToDraw(s);
            BeginRefresh(bw);
            frame = sd->frame;
            bw->RPort->BitMap = sd->sb[frame]->sb_BitMap;

            drawBoard(bw);

            EndRefresh(bw, TRUE);

            /* changeScreen(s); */
            frame = sd->frame;
        }
        else if (class == IDCMP_ACTIVEWINDOW)
        {
            updateCursor(bw, mx >> 4, my >> 4);
        }
        else if (class == IDCMP_INACTIVEWINDOW)
        {
            discardCursor(bw);
        }
        else if (class == IDCMP_MOUSEBUTTONS)
        {
            if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
            {
                bd->paste = FALSE;
            }
            else if (code == IECODE_LBUTTON)
            {
                if (cy >= 1 && !(cx == bd->x && cy == bd->y))
                {
                    /* Paste */
                    TILE *t = &bd->currentTile;

                    bd->board[cy][cx] = *t;
                    bd->paste = TRUE;

                    if (bd->pal == PAL_HERO)
                    {
                        TILE *prev = &bd->board[bd->y][bd->x];
                        prev->kind = TILE_FLOOR;
                        prev->subKind = prev->floor;
                        drawTile(bw, bd->x, bd->y, prev->subKind, prev->kind, TRUE);
                        bd->x = cx;
                        bd->y = cy;
                    }
                }
                else if (cy == 0 && cx >= 1 && cx <= PALS && cx - 1 != bd->pal)
                {
                    /* Select */
                    TILE *t = &bd->currentTile;
                    WORD prevPal = bd->pal;

                    bd->pal = cx - 1;
                    drawPalette(bw, prevPal, prevPal);
                    drawPalette(bw, bd->pal, bd->pal);

                    switch (cx - 1)
                    {
                        case PAL_FLAGSTONE:
                            t->floor = FLOOR_FLAGSTONE;
                            break;
                        case PAL_MUD:
                            t->floor = FLOOR_FILLEDMUD;
                            break;
                        default:
                            t->floor = FLOOR_NORMAL;
                            break;
                    }
                    switch (cx - 1)
                    {
                        case PAL_FLOOR:
                        case PAL_FLAGSTONE:
                            t->kind = TILE_FLOOR;
                            t->subKind = t->floor;
                            break;
                        case PAL_BOX:
                        case PAL_HERO:
                            t->kind = TILE_OBJECT;
                            break;
                        case PAL_FRUIT:
                            t->kind = TILE_ITEM;
                            break;
                        case PAL_WALL:
                        case PAL_MUD:
                            t->kind = TILE_WALL;
                            break;
                    }
                    switch (cx - 1)
                    {
                        case PAL_BOX:
                            t->subKind = OBJ_BOX;
                            break;
                        case PAL_FRUIT:
                            t->subKind = ITEM_FRUIT;
                            break;
                        case PAL_HERO:
                            t->subKind = OBJ_HERO;
                            break;
                        case PAL_WALL:
                            t->subKind = WALL_NORMAL;
                            break;
                        case PAL_MUD:
                            t->subKind = WALL_MUD;
                    }
                }
            }
        }
    }
    return(WINACT_NONE);
}

void drawMessage(struct Window *w, STRPTR text, WORD x, WORD y)
{
    struct RastPort *rp = w->RPort;

    SetABPenDrMd(rp, 1, 0, JAM1);
    Move(rp, x, y + rp->Font->tf_Baseline);
    Text(rp, text, strlen(text));

    SetABPenDrMd(rp, 10, 0, JAM1);
    Move(rp, x - 1, y - 1 + rp->Font->tf_Baseline);
    Text(rp, text, strlen(text));
}

enum
{
    BOARDSIG,
    PANELSIG,
    REQSIG,
    COPPERSIG,
    SAFESIG,
    SIGNALS
};

VOID mainLoop(struct Window *bw, struct Window *pw)
{
    BOOL done = FALSE;
    ULONG signals[SIGNALS], total;
    struct Window *rw;
    static struct reqData rd;
    struct boardData *bd = (struct boardData *)bw->UserData;
    struct screenData *sd = (struct screenData *)bw->WScreen->UserData;
    WORD frame = sd->frame;
    struct panelData *pd = (struct panelData *)pw->UserData;
    BOOL pause = FALSE;
    WORD counter = 0; /* Frame counter */

    signals[BOARDSIG] = 1L << bw->UserPort->mp_SigBit;
    signals[PANELSIG] = 1L << pw->UserPort->mp_SigBit;
    signals[REQSIG] = 0L;
    signals[COPPERSIG] = 1L << sd->cd.signal; /* Copper */
    signals[SAFESIG] = 1L << sd->mp->mp_SigBit;

    total = signals[BOARDSIG]|signals[PANELSIG]|signals[REQSIG]|signals[COPPERSIG]|signals[SAFESIG];

    if (!(rw = openWindow(WIN_REQ, bw->WScreen, &rd.win)))
    {
        return;
    }

    total |= signals[REQSIG] = 1L << rw->UserPort->mp_SigBit;


/*    SetSignal(0L, 1L << sd->cd.signal);
    Wait(1L << sd->cd.signal); */

    BltBitMapRastPort(bd->gfx, 0, 128, rw->RPort, 0, 0, rw->Width, rw->Height, 0xc0);
    drawMessage(rw, "Magazyn wersja 1.3", 64, 24);
    initReq(rw, pw);

    /*  changeScreen(bw->WScreen); */
    AddGList(rw, rd.gads, -1, -1, NULL);
    RefreshGList(rd.gads, rw, NULL, -1);

    struct Rectangle bounds = { rw->LeftEdge, rw->TopEdge, rw->LeftEdge + rw->Width - 1, rw->TopEdge + rw->Height - 1 };

    OrRectRegion(sd->update[sd->frame], &bounds);

    AddGList(pw, pd->gads, -1, -1, NULL);
    RefreshGList(pd->gads, pw, NULL, -1);


    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals[BOARDSIG])
        {
            done = (handleBoard(bw) == WINACT_ESC);
        }

        if (result & signals[SAFESIG])
        {
            safeToDraw(bw->WScreen);
        }

        if (result & signals[PANELSIG])
        {

        }

        if (result & signals[REQSIG])
        {
            closeWindow(rw);
            rw = NULL;
            total &= ~signals[REQSIG];
            signals[REQSIG] = 0L;
        }

        if (result & signals[COPPERSIG])
        {
            BYTE dx = 0, dy = 0;

            changeScreen(bw->WScreen);
            if (pause)
            {
                if (counter < DELAY)
                {
                    if (++counter == DELAY)
                    {
                        pause = FALSE;
                    }
                }
            }

            if (!pause)
            {
                WORD dir = bd->dir;

                if ((dir & LEFT_FLAG) == LEFT_FLAG)
                    dx = -1;
                if ((dir & RIGHT_FLAG) == RIGHT_FLAG)
                    dx = 1;
                if ((dir & UP_FLAG) == UP_FLAG)
                    dy = -1;
                if ((dir & DOWN_FLAG) == DOWN_FLAG)
                    dy = 1;

                if (dx || dy)
                {
                    moveHero(bw, dx, dy);
                    pause = TRUE;
                    counter = 0;
                }
            }
        }
    }

    if (rw)
        closeWindow(rw);
}

struct BitMap *loadGraphics(STRPTR name, struct Screen *s)
{
    struct IFFHandle *iff = NULL;
    struct BitMap *gfx = NULL;

    if (iff = openIFF(name, IFFF_READ))
    {
        if (scanILBM(iff))
        {
            if (loadCMAP(iff, s))
            {
                gfx = loadBitMap(iff);
            }
        }
        closeIFF(iff);
    }
    return(gfx);
}

int main(void)
{
    struct Screen *s;
    static struct boardData bd;
    static struct screenData sd;
    static struct panelData pd;

    if (s = openScreen("Magazyn", &sd))
    {
        struct Window *bw, *pw;

        s->RastPort.BitMap = sd.sb[sd.frame]->sb_BitMap;

        if (bw = openWindow(WIN_BOARD, s, &bd.win))
        {
            if (pw = openWindow(WIN_PANEL, s, &pd.win))
            {
                struct BitMap *gfx;

                if (bd.gfx = pd.gfx = gfx = loadGraphics("Data1/Gfx/Template.iff", s))
                {
                    ScreenToFront(s);
                    ActivateWindow(bw);
                    if (!loadBoard("States/State.iff", bw, pw))
                    {
                        clearBoard(bw);
                    }
                    initBoard(bw);
                    initPanel(pw);

                    mainLoop(bw, pw);


                    freePanel(pw);
                    FreeBitMap(gfx);
                }
                closeWindow(pw);
            }
            closeWindow(bw);
        }
        closeScreen(s);
    }
    return(RETURN_OK);
}
