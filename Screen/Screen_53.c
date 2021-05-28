
/*
** Screen management functions.
** (c)2021 Robert Szacki
**
** $Id$
*/

#include <exec/interrupts.h>
#include <exec/memory.h>

#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <hardware/blit.h>

#include <graphics/view.h>
#include <graphics/gfxmacros.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>

#include "Screen.h"
#include "IFF.h"

UBYTE title[] = "Magazyn";

__far extern struct Custom custom;

extern void myCopperIS();

struct TextAttr ta =
{
    "ld.font",
    8,
    FS_NORMAL,
    FPF_DISKFONT|FPF_DESIGNED
};

struct tilePos
{
    UWORD x, y, width, height;
} tilePos[] =
{
    0, 16, 1, 1,
    16, 16, 1, 1,
    0, 0, 5, 1,
    80, 0, 5, 1,

    0, 128, 1, 1,
    16, 128, 1, 1,
    32, 128, 1, 1,
    48, 128, 1, 1,
    64, 128, 1, 1,
    80, 128, 1, 1,
    96, 128, 1, 1,
    112, 128, 1, 1,
    128, 128, 1, 1
};

/* addCopIS: Add copper interrupt server to a ViewPort */

struct Interrupt *addCopIS(struct ViewPort *vp, WORD *signal)
{
    struct Interrupt *is;

    if (is = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR))
    {
        struct copISData *data;

        if (data = AllocMem(sizeof(*data), MEMF_PUBLIC|MEMF_CLEAR))
        {
            is->is_Code = myCopperIS;
            is->is_Data = (APTR)data;
            is->is_Node.ln_Pri = 0;
            is->is_Node.ln_Name = title;

            if ((data->signal = *signal = AllocSignal(-1)) != -1)
            {
                data->task = FindTask(NULL);
                data->vp = vp;

                AddIntServer(INTB_COPER, is);
                return(is);
            }
            FreeMem(data, sizeof(*data));
        }
        FreeMem(is, sizeof(*is));
    }
    return(NULL);
}

void remCopIS(struct Interrupt *is)
{
    struct copISData *data = (struct copISData *)is->is_Data;

    RemIntServer(INTB_COPER, is);
    FreeSignal(data->signal);
    FreeMem(data, sizeof(*data));
    FreeMem(is, sizeof(*is));
}

/* addUCL: Add user copper-list to a ViewPort */

struct UCopList *addUCL(struct ViewPort *vp)
{
    struct UCopList *ucl;
    const commands = 3;

    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
    {
        CINIT(ucl, commands);
        CWAIT(ucl, 0, 0);
        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
        CEND(ucl);

        Forbid();
        vp->UCopIns = ucl;
        Permit();

        RethinkDisplay();
        return(ucl);
    }
    return(NULL);
}

/* openScreen: Open double-buffered screen with user copper-list */

struct Screen *openScreen(void)
{
    const struct Rectangle dclip = { 0, 0, 319, 255 };
    const ULONG modeID = LORES_KEY;
    const UBYTE depth = 5;
    const ULONG idcmp = IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE;
    struct Screen *s;
    struct Interrupt *is;
    WORD signal;
    struct UCopList *ucl;

    if (s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_DisplayID,   modeID,
        SA_Depth,       depth,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       title,
        SA_Font,        &ta,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        struct ViewPort *vp = &s->ViewPort;

        if (ucl = addUCL(vp))
        {
            if (is = addCopIS(vp, &signal))
            {
                struct screenData *sd;

                if (sd = AllocMem(sizeof(*sd), MEMF_PUBLIC|MEMF_CLEAR))
                {
                    if (sd->mp = CreateMsgPort())
                    {
                        if (sd->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
                        {
                            if (sd->sb[1] = AllocScreenBuffer(s, NULL, 0))
                            {
                                if (sd->w = OpenWindowTags(NULL,
                                    WA_CustomScreen,    s,
                                    WA_Left,            0,
                                    WA_Top,             0,
                                    WA_Width,           s->Width,
                                    WA_Height,          s->Height,
                                    WA_Backdrop,        TRUE,
                                    WA_Borderless,      TRUE,
                                    WA_Activate,        TRUE,
                                    WA_RMBTrap,         TRUE,
                                    WA_SimpleRefresh,   TRUE,
                                    WA_BackFill,        LAYERS_NOBACKFILL,
                                    WA_IDCMP,           idcmp,
                                    WA_ReportMouse,     TRUE,
                                    TAG_DONE))
                                {
                                    sd->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->mp;
                                    sd->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->mp;
                                    sd->safe = TRUE;
                                    sd->frame = 1;

                                    sd->is = is;
                                    sd->signal = signal;

                                    s->UserData = (APTR)sd;
                                    return(s);
                                }
                                FreeScreenBuffer(s, sd->sb[1]);
                            }
                            FreeScreenBuffer(s, sd->sb[0]);
                        }
                        DeleteMsgPort(sd->mp);
                    }
                    FreeMem(sd, sizeof(*sd));
                }
                remCopIS(is);
            }
        }
        CloseScreen(s);
    }
    return(NULL);
}

/* safeToDraw: Ensure it's safe to draw into the buffer */

void safeToDraw(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    if (!sd->safe)
    {
        while (!GetMsg(sd->mp))
        {
            WaitPort(sd->mp);
        }
        sd->safe = TRUE;
    }
}

/* changeScreen: Change screen buffer */

void changeScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    if (sd->safe)
    {
        UWORD frame = sd->frame;

        while (!ChangeScreenBuffer(s, sd->sb[frame]))
        {
            WaitTOF();
        }
        sd->frame = frame ^= 1;
        sd->safe = FALSE;
    }
}

void closeScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    CloseWindow(sd->w);

    if (!sd->safe)
    {
        while (!GetMsg(sd->mp))
        {
            WaitPort(sd->mp);
        }
    }
    FreeScreenBuffer(s, sd->sb[1]);
    FreeScreenBuffer(s, sd->sb[0]);
    DeleteMsgPort(sd->mp);

    remCopIS(sd->is);

    FreeMem(sd, sizeof(*sd));

    CloseScreen(s);
}

void drawTile(struct BitMap *gfx, WORD srcx, WORD srcy, struct BitMap *dest, WORD destx, WORD desty, WORD width, WORD height)
{
    struct Custom *cust = &custom;
    WORD srcbpr = gfx->BytesPerRow;
    WORD destbpr = dest->BytesPerRow;
    WORD i;

    width = (width + 15) >> 4;

    LONG srcoffset = (srcy * srcbpr) + ((srcx >> 4) << 1);
    LONG destoffset = (desty * destbpr) + ((destx >> 4) << 1);

    OwnBlitter();

    for (i = 0; i < dest->Depth; i++)
    {
        WaitBlit();

        cust->bltcon0 = SRCA | DEST | 0xf0;
        cust->bltcon1 = 0;
        cust->bltapt  = gfx->Planes[i] + srcoffset;
        cust->bltdpt  = dest->Planes[i] + destoffset;
        cust->bltamod = srcbpr - (width << 1);
        cust->bltdmod = destbpr - (width << 1);
        cust->bltafwm = 0xffff;
        cust->bltalwm = 0xffff;
        cust->bltsizv = height;
        cust->bltsizh = width;
    }
    DisownBlitter();
}

void draw(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;
    struct RastPort *rp = sd->w->RPort;
    struct TextFont *font = rp->Font;
    UWORD frame = sd->frame;
    static UWORD counter = 0, prev[2] = { 0 };
    WORD x, y;

    rp->BitMap = sd->sb[frame]->sb_BitMap;

    prev[frame] = counter;

    for (y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (x = 0; x < SCREEN_WIDTH; x++)
        {
            UWORD wid = sd->win.array[y][x];
            struct window *w = &sd->win.windows[wid];
            UWORD rx = x - w->left;
            UWORD ry = y - w->top;
            struct tile *t = &w->array[(ry * w->width) + rx];

            if (t->delay > 0)
            {
                if (--t->delay == 0)
                {
                    t->gfx = t->newgfx;
                    t->update = 2;
                }
            }

            if (t->update > 0)
            {
                UWORD gfx = t->gfx - 1;
                t->update--;
                /*
                BltBitMap(sd->gfx, tilePos[gfx].x, tilePos[gfx].y, rp->BitMap, x << 4, y << 4, tilePos[gfx].width << 4, tilePos[gfx].height << 4, 0xc0, 0xff, NULL);
                */
                drawTile(sd->gfx, tilePos[gfx].x, tilePos[gfx].y, rp->BitMap, x << 4, y << 4, tilePos[gfx].width << 4, tilePos[gfx].height << 4);
            }

            if (t->newgfx)
            {
                UWORD gfx = sd->tile - 1;

                drawTile(sd->gfx, tilePos[gfx].x, tilePos[gfx].y, rp->BitMap, x << 4, y << 4, tilePos[gfx].width << 4, tilePos[gfx].height << 4);

                SetAPen(rp, 2);
                Move(rp, x << 4, y << 4);
                Draw(rp, (x << 4) + 15, y << 4);
                Draw(rp, (x << 4) + 15, (y << 4) + 15);
                Draw(rp, x << 4, (y << 4) + 15);
                Draw(rp, x << 4, (y << 4) + 1);
            }
        }
    }

    Move(rp, 40, 4 + font->tf_Baseline);
    SetABPenDrMd(rp, 17, 0, JAM1);
    Text(rp, "CLEAR", 5);

    counter++;
}

/* Main screen event loop */

void screenLoop(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;
    ULONG signals[SIGNAL_SOURCES] = { 0 }, total = 0;
    WORD i;
    BOOL done = FALSE, paint = FALSE;
    UWORD prevx = 0, prevy = 1;

    signals[SAFE_TO_DRAW] = 1L << sd->mp->mp_SigBit;
    signals[COPPER_SYNC] = 1L << sd->signal;
    signals[USER_MESSAGE] = 1L << sd->w->UserPort->mp_SigBit;

    for (i = 0; i < SIGNAL_SOURCES; i++)
    {
        total |= signals[i];
    }

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals[SAFE_TO_DRAW])
        {
            safeToDraw(s);

            draw(s);
        }

        if (result & signals[COPPER_SYNC])
        {
            changeScreen(s);
        }

        if (result & signals[USER_MESSAGE])
        {
            struct MsgPort *mp = sd->w->UserPort;
            struct IntuiMessage *msg;

            while (msg = (struct IntuiMessage *)GetMsg(mp))
            {
                ULONG class = msg->Class;
                UWORD code = msg->Code;
                WORD mx = msg->MouseX >> 4;
                WORD my = msg->MouseY >> 4;

                ReplyMsg((struct Message *)msg);

                if (class == IDCMP_RAWKEY)
                {
                    struct window *w = &sd->win.windows[BACK_WIN];
                    if (code == ESC_KEY)
                    {
                        done = TRUE;
                    }
                    else if (code == LEFT_KEY)
                    {
                        moveHero(&sd->board, -1, 0, w);
                    }
                    else if (code == RIGHT_KEY)
                    {
                        moveHero(&sd->board, 1, 0, w);
                    }
                    else if (code == UP_KEY)
                    {
                        moveHero(&sd->board, 0, -1, w);
                    }
                    else if (code == DOWN_KEY)
                    {
                        moveHero(&sd->board, 0, 1, w);
                    }
                }
                else if (class == IDCMP_MOUSEBUTTONS)
                {
                    if (code == IECODE_LBUTTON)
                    {
                        UWORD wid = sd->win.array[my][mx];
                        struct window *w = &sd->win.windows[wid];
                        struct tile *t;
                        mx -= w->left;
                        my -= w->top;
                        t = &w->array[(my * w->width) + mx];


                        if (w->action[t->action])
                        {
                            while (t->gfx == 0)
                                t--;

                            w->action[t->action](s, w, t, FALSE);
                            w->active = t;
                        }
                        else
                        {
                            t->gfx = sd->tile;
                            t->update = 2;
                            paint = TRUE;
                        }
                    }
                    else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                    {
                        UWORD wid = sd->win.array[my][mx];
                        struct window *w = &sd->win.windows[wid];
                        struct tile *t, *active = w->active;
                        mx -= w->left;
                        my -= w->top;
                        t = &w->array[(my * w->width) + mx];

                        if (active)
                        {
                            while (t->gfx == 0)
                                t--;

                            w->action[active->action](s, w, t, TRUE);

                            if (sd->win.windows[BACK_WIN].close)
                            {
                                done = TRUE;
                            }
                        }
                        paint = FALSE;
                        w->active = NULL;
                    }
                }
                else if (class == IDCMP_MOUSEMOVE)
                {
                    if (my == 0)
                    {
                    }
                    else if (paint && (mx != prevx || my != prevy))
                    {
                        UWORD wid = sd->win.array[my][mx];
                        struct window *w = &sd->win.windows[wid];
                        struct tile *t;
                        mx -= w->left;
                        my -= w->top;
                        t = &w->array[(my * w->width) + mx];

                        if (!w->action[t->action])
                        {
                            t->gfx = sd->tile;
                            t->update = 2;
                            t->newgfx = 1;

                            t = &w->array[(prevy * w->width) + prevx];
                            t->newgfx = 0;
                            t->update = 2;

                            prevx = mx;
                            prevy = my;
                        }
                    }
                    else if (mx != prevx || my != prevy)
                    {
                        UWORD wid = sd->win.array[my][mx];
                        struct window *w = &sd->win.windows[wid];
                        struct tile *t;
                        mx -= w->left;
                        my -= w->top;
                        t = &w->array[(my * w->width) + mx];

                        t->newgfx = 1;

                        t = &w->array[(prevy * w->width) + prevx];

                        t->newgfx = 0;
                        t->update = 2;

                        prevx = mx;
                        prevy = my;
                    }
                }
            }
        }
    }
}

void clearBoard(struct window *w)
{
    WORD x, y;

    for (y = 1; y < SCREEN_HEIGHT; y++)
    {
        for (x = 0; x < SCREEN_WIDTH; x++)
        {
            struct tile *t;
            t = &w->array[(y * w->width) + x];

            if (y == 2 && x == 1)
            {
                t->gfx = HERO_TILE;
            }
            else if (y == 1 || y == SCREEN_HEIGHT - 1 || x == 0 || x == SCREEN_WIDTH - 1)
                t->gfx = WALL_TILE;
            else
                t->gfx = FLOOR_TILE;
            t->update = 2;
        }
    }
}

void closeWindow(struct Screen *s, struct window *w, struct tile *t, BOOL up)
{
    if (!up)
    {
        t->gfx = CLOSE_GAD_PUSHED;
        t->update = 2;
    }
    else
    {
        struct tile *active = w->active;

        active->gfx = CLOSE_GAD;
        active->update = 2;

        if (active == t)
        {
            w->close = TRUE;
        }
    }
}

void openMenu(struct Screen *s, struct window *w, struct tile *t, BOOL up)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    if (!up)
    {
        t->gfx = BIG_BUTTON_PUSHED;
        t->update = 2;
    }
    else
    {
        struct tile *active = w->active;

        active->gfx = BIG_BUTTON;
        active->update = 2;
        if (active == t)
        {
            if (t->aux == 0)
            {
                clearBoard(w);
            }
            else if (t->aux == 1)
            {
                convertBoard(&sd->board, w);
            }
        }
    }
}

void selectTile(struct Screen *s, struct window *w, struct tile *t, BOOL up)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    if (up)
    {
        if (++sd->tile > MAX_TILE)
        {
            sd->tile = FLOOR_TILE;
        }

        t->gfx = sd->tile;
        t->update = 2;
    }
}

void initButton(struct tile *t, UWORD gfx, UWORD action, WORD aux)
{
    WORD i;

    t->gfx = gfx;
    t->action = action;
    t->update = 2;
    t->aux = aux;

    for (i = 0; i < tilePos[gfx].width - 1; i++)
    {
        t++;
        t->action = action;
    }
}

BOOL initArray(struct window *w)
{
    if (w->array = AllocVec(w->width * w->height * sizeof(*w->array), MEMF_PUBLIC|MEMF_CLEAR))
    {
        struct tile *t;
        WORD x, y;

        w->action[CLOSE_ACTION] = closeWindow;
        w->action[OPEN_MENU] = openMenu;
        w->action[SELECT_TILE] = selectTile;
        t = &w->array[(0 * w->width) + 0];
        t->gfx = CLOSE_GAD;
        t->action = CLOSE_ACTION;
        t->update = 2;

        t = &w->array[(0 * w->width) + 1];

        initButton(t, BIG_BUTTON, OPEN_MENU, 0);
        t += 5;
        initButton(t, BIG_BUTTON, OPEN_MENU, 1);
        t += 5;
        initButton(t, BIG_BUTTON, OPEN_MENU, 2);

        t = &w->array[(0 * w->width) + 16];

        t->gfx = WALL_TILE;
        t->action = SELECT_TILE;
        t->update = 2;

        for (y = 1; y < SCREEN_HEIGHT; y++)
        {
            for (x = 0; x < SCREEN_WIDTH; x++)
            {
                t = &w->array[(y * w->width) + x];

                if (y == 2 && x == 1)
                {
                    t->gfx = HERO_TILE;
                }
                else if (y == 1 || y == SCREEN_HEIGHT - 1 || x == 0 || x == SCREEN_WIDTH - 1)
                    t->gfx = WALL_TILE;
                else
                    t->gfx = FLOOR_TILE;

                t->update = 2;
            }
        }
        return(TRUE);
    }
    return(FALSE);
}

BOOL initBackWindow(struct window *w)
{
    w->left = w->top = 0;
    w->width = SCREEN_WIDTH;
    w->height = SCREEN_HEIGHT;

    if (initArray(w))
    {
        return(TRUE);
    }
    return(FALSE);
}

void freeWindow(struct window *w)
{
    FreeVec(w->array);
}

void showBoard(struct gameBoard *gb, struct window *w)
{
    WORD x, y;

    for (y = 1; y < SCREEN_HEIGHT; y++)
    {
        for (x = 0; x < SCREEN_WIDTH; x++)
        {
            struct tile *t = &w->array[(y * w->width) + x];

            t->gfx = getGfx(&gb->board[y][x]);
            t->update = 2;
        }
    }
}

int main(void)
{
    struct Screen *s;
    struct TextFont *font;

    if (font = OpenDiskFont(&ta))
    {
        if (s = openScreen())
        {
            struct screenData *sd = (struct screenData *)s->UserData;

            if (sd->gfx = loadILBM("Dane/Magazyn.iff", s->ViewPort.ColorMap))
            {
                MakeScreen(s);
                RethinkDisplay();

                if (initBackWindow(&sd->win.windows[BACK_WIN]))
                {
                    if (loadBoard(&sd->board, "T:Map.dat"))
                    {
                        showBoard(&sd->board, &sd->win.windows[BACK_WIN]);
                    }

                    sd->tile = WALL_TILE;
                    screenLoop(s);

                    saveBoard(&sd->board, "T:Map.dat");

                    freeWindow(&sd->win.windows[BACK_WIN]);
                }
                FreeBitMap(sd->gfx);
            }
            closeScreen(s);
        }
        CloseFont(font);
    }
    return(0);
}
