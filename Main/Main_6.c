
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/datatypes_protos.h>

#include "GameRoot.h"

BOOL initGame(struct gameRoot *gr)
{
    if (gr->screen = AllocMem(sizeof(*gr->screen), MEMF_PUBLIC))
    {
        if (initScreen(gr, gr->screen))
        {
            if (gr->control = AllocMem(sizeof(*gr->control), MEMF_PUBLIC))
            {
                if (initControl(gr, gr->control))
                {
                    if (gr->window = AllocMem(sizeof(*gr->window), MEMF_PUBLIC))
                    {
                        if (initWindow(gr, gr->window))
                        {
                            if (addPanel(gr, gr->window))
                            {
                                if (gr->graphics = AllocMem(sizeof(*gr->graphics), MEMF_PUBLIC))
                                {
                                    if (initGraphics(gr, gr->graphics, "Dane/Magazyn.pic"))
                                    {
                                        return(TRUE);
                                    }
                                    FreeMem(gr->graphics, sizeof(*gr->graphics));
                                }
                            }
                            closeWindow(gr, gr->window);
                        }
                        FreeMem(gr->window, sizeof(*gr->window));
                    }
                    closeControl(gr, gr->control);
                }
                FreeMem(gr->control, sizeof(*gr->control));
            }
            closeScreen(gr, gr->screen);
        }
        FreeMem(gr->screen, sizeof(*gr->screen));
    }
    return(FALSE);
}

VOID closeGame(struct gameRoot *gr)
{
    closeGraphics(gr, gr->graphics);
    FreeMem(gr->graphics, sizeof(*gr->graphics));
    closeWindow(gr, gr->window);
    FreeMem(gr->window, sizeof(*gr->window));
    closeControl(gr, gr->control);
    FreeMem(gr->control, sizeof(*gr->control));
    closeScreen(gr, gr->screen);
    FreeMem(gr->screen, sizeof(*gr->screen));
}

LONG playGame(struct gameRoot *gr)
{
    ULONG masks[INPUTS] = { 0 }, totalMask = 0L, resultMask;
    WORD i;
    struct gameScreen *screen = gr->screen;
    BOOL drawn = FALSE;
    WORD frame = 1;
    struct RastPort *rp = &screen->intuiScreen->RastPort;
    struct MsgPort *up = gr->control->window->UserPort;
    struct gameWindow *active = gr->window;
    struct gameMessage gm;

    masks[SCREEN_WRITE] = 1L << screen->writePort->mp_SigBit;
    masks[SCREEN_CHANGE] = 1L << screen->changePort->mp_SigBit;
    masks[WINDOW_IDCMP] = 1L << up->mp_SigBit;

    for (i = 0; i < INPUTS; i++)
        totalMask |= masks[i];

    gr->done = FALSE;

    while (!gr->done)
    {
        if (drawn)
            resultMask = Wait(totalMask);
        else
        {
            drawn = TRUE;
            resultMask = masks[SCREEN_CHANGE];
        }

        if (resultMask & masks[SCREEN_WRITE])
        {
            /* Write screen here */
            if (!screen->safeToWrite)
                while (!GetMsg(screen->writePort))
                    WaitPort(screen->writePort);

            screen->safeToWrite = TRUE;

            rp->BitMap = screen->bitmaps[frame];

            screen->frame = frame;

            gm.type = WRITE_MESSAGE;

            active->handler(gr, active, &gm);
        }
        if (resultMask & masks[SCREEN_CHANGE])
        {
            /* Change screen here */

            if (!screen->safeToChange)
                while (!GetMsg(screen->changePort))
                    WaitPort(screen->changePort);

            screen->safeToChange = TRUE;

            WaitBlit();

            while (!ChangeScreenBuffer(screen->intuiScreen, screen->buffers[frame]))
                WaitTOF();

            frame ^= 1;

            screen->safeToWrite = screen->safeToChange = FALSE;
        }
        if (resultMask & masks[WINDOW_IDCMP])
        {
            /* Handle intuition messages here */
            struct IntuiMessage *msg;

            while (msg = GT_GetIMsg(up))
            {
                ULONG class = msg->Class;
                UWORD code = msg->Code;
                WORD mx = msg->MouseX;
                WORD my = msg->MouseY;

                gm.type = IDCMP_MESSAGE;
                gm.msg.intuiMsg = msg;

                active->handler(gr, active, &gm);

                GT_ReplyIMsg(msg);
            }
        }
    }
}

static VOID setPorts(struct gameScreen *gs, struct ScreenBuffer *sb, LONG frame)
{
    sb->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = gs->writePort;
    sb->sb_DBufInfo->dbi_UserData1   = (APTR)frame;
    sb->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = gs->changePort;
    sb->sb_DBufInfo->dbi_UserData2   = (APTR)frame;
}

BOOL initScreen(struct gameRoot *gr, struct gameScreen *gs)
{
    if (gs->bitmaps[0] = AllocBitMap(RAS_WIDTH, RAS_HEIGHT, RAS_DEPTH, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (gs->bitmaps[1] = AllocBitMap(RAS_WIDTH, RAS_HEIGHT, RAS_DEPTH, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            if (gs->intuiScreen = OpenScreenTags(NULL,
                SA_BitMap,      gs->bitmaps[0],
                SA_Left,        0,
                SA_Top,         0,
                SA_Width,       SCR_WIDTH,
                SA_Height,      SCR_HEIGHT,
                SA_Depth,       RAS_DEPTH,
                SA_DisplayID,   SCR_MODEID,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_Draggable,   FALSE,
                SA_ShowTitle,   FALSE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                TAG_DONE))
            {
                if (gs->writePort = CreateMsgPort())
                {
                    if (gs->changePort = CreateMsgPort())
                    {
                        if (gs->buffers[0] = AllocScreenBuffer(gs->intuiScreen, gs->bitmaps[0], 0))
                        {
                            if (gs->buffers[1] = AllocScreenBuffer(gs->intuiScreen, gs->bitmaps[1], 0))
                            {
                                setPorts(gs, gs->buffers[0], 0);
                                setPorts(gs, gs->buffers[1], 1);
                                gs->safeToWrite = gs->safeToChange = TRUE;
                                return(TRUE);
                            }
                            FreeScreenBuffer(gs->intuiScreen, gs->buffers[0]);
                        }
                        DeleteMsgPort(gs->changePort);
                    }
                    DeleteMsgPort(gs->writePort);
                }
                CloseScreen(gs->intuiScreen);
            }
            FreeBitMap(gs->bitmaps[1]);
        }
        FreeBitMap(gs->bitmaps[0]);
    }
    return(FALSE);
}

VOID closeScreen(struct gameRoot *gr, struct gameScreen *gs)
{
    if (!gs->safeToChange)
        while (!GetMsg(gs->changePort))
            WaitPort(gs->changePort);

    if (!gs->safeToWrite)
        while (!GetMsg(gs->writePort))
            WaitPort(gs->writePort);

    FreeScreenBuffer(gs->intuiScreen, gs->buffers[1]);
    FreeScreenBuffer(gs->intuiScreen, gs->buffers[0]);
    DeleteMsgPort(gs->changePort);
    DeleteMsgPort(gs->writePort);
    CloseScreen(gs->intuiScreen);
    FreeBitMap(gs->bitmaps[1]);
    FreeBitMap(gs->bitmaps[0]);
}

BOOL initControl(struct gameRoot *gr, struct gameControl *gc)
{
    struct Screen *screen = gr->screen->intuiScreen;

    if (gc->window = OpenWindowTags(NULL,
        WA_CustomScreen,    screen,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           screen->Width,
        WA_Height,          screen->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_ReportMouse,     TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_FLAGS,
        TAG_DONE))
    {
        return(TRUE);
    }
    return(FALSE);
}

VOID closeControl(struct gameRoot *gr, struct gameControl *gc)
{
    CloseWindow(gc->window);
}

VOID loadColors(struct gameRoot *gr, struct gameGraphics *gfx)
{
    WORD i;
    struct ColorMap *cm = gr->screen->intuiScreen->ViewPort.ColorMap;
    ULONG *colors = gfx->colors;

    for (i = 0; i < gfx->count; i++)
    {
        SetRGB32CM(cm, i, colors[0], colors[1], colors[2]);
        colors += 3;
    }
    MakeScreen(gr->screen->intuiScreen);
    RethinkDisplay();
}

BOOL initGraphics(struct gameRoot *gr, struct gameGraphics *gfx, STRPTR name)
{
    Object *o;

    if (gfx->o = o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    gr->screen->intuiScreen,
        PDTA_Remap,     FALSE,
        TAG_DONE))
    {
        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        GetDTAttrs(o,
            PDTA_BitMap,    &gfx->brbitmap,
            PDTA_CRegs,     &gfx->colors,
            PDTA_NumColors, &gfx->count,
            TAG_DONE);

        loadColors(gr, gfx);
        return(TRUE);
    }
    return(FALSE);
}

VOID closeGraphics(struct gameRoot *gr, struct gameGraphics *gfx)
{
    DisposeDTObject(gfx->o);
}

ULONG mainHandler(struct gameRoot *gr, struct gameWindow *gw, struct gameMessage *gm)
{
    static WORD drawTile = -1, drawX, drawY;
    static BOOL repeat = FALSE;

    if (gm->type == IDCMP_MESSAGE)
    {
        ULONG class = gm->msg.intuiMsg->Class;
        UWORD code = gm->msg.intuiMsg->Code;
        WORD mx = gm->msg.intuiMsg->MouseX;
        WORD my = gm->msg.intuiMsg->MouseY;
        struct Window *w = gm->msg.intuiMsg->IDCMPWindow;
        struct Gadget *gad = (struct Gadget *)gm->msg.intuiMsg->IAddress;

        if (class == IDCMP_RAWKEY)
        {
            if (code == ESC_KEY)
                gr->done = TRUE;
        }
        else if (class == IDCMP_GADGETDOWN)
        {
            WORD tilex = (mx - gad->LeftEdge) >> 4;
            WORD tiley = (my - gad->TopEdge) >> 4;
            WORD tile = (tiley * (gad->Width >> 4)) + tilex;

            switch (gad->GadgetID)
            {
                case GID_BRICKS:
                    gr->tile = tile;
                    break;
                case GID_MENU:
                    if (tile == 0)
                        gr->done = TRUE;
                    else if (tile == 19)
                        ScreenToBack(gr->screen->intuiScreen);
                    break;
                case GID_BOARD:
                    /* Do poprawki! */
                    if (drawTile == -1)
                    {
                        drawTile = gr->tile;
                        drawX = tilex;
                        drawY = tiley;
                    }
                    break;
            }
        }
    }
    else if (gm->type == WRITE_MESSAGE)
    {
        struct Gadget *gad = gw->gadgets->gadget;
        struct RastPort *rp = gw->layer->rp;
        UBYTE text[5] = "0000";
        static WORD counter = 0;
        static BOOL drawn[2] = { FALSE };
        WORD frame = gr->screen->frame;
        rp->BitMap = gr->screen->bitmaps[frame];

        if (!drawn[frame])
        {
            while (gad)
            {
                BltBitMapRastPort(gr->graphics->brbitmap, gad->LeftEdge, gad->TopEdge, rp, gad->LeftEdge, gad->TopEdge, gad->Width, gad->Height, 0xc0);
                gad = gad->NextGadget;
            }
        }

        if (drawTile != -1 || repeat)
        {
            BltBitMapRastPort(gr->graphics->brbitmap, (drawTile % 2) << 4, 32 + ((drawTile / 2) << 4), rp, 32 + (drawX << 4), 32 + (drawY << 4), 16, 16, 0xc0);
            if (repeat)
            {
                drawTile = -1;
                repeat = FALSE;
            }
            else
                repeat = TRUE;
        }
    }
    return(0);
}

BOOL initWindow(struct gameRoot *gr, struct gameWindow *gw)
{
    gw->layer = gr->control->window->WLayer;
    gw->pred = NULL;
    gw->succ = NULL;

    gw->handler = mainHandler;
    gw->gadgets = NULL;

    return(TRUE);
}

BOOL addPanel(struct gameRoot *gr, struct gameWindow *gw)
{
    struct gameGadget *gg[3];
    struct newGadget ng;

    if (gg[0] = AllocMem(sizeof(*gg[0]), MEMF_PUBLIC))
    {
        ng.left   = 0;
        ng.top    = MENU_HEIGHT;
        ng.width  = PANEL_WIDTH;
        ng.height = PANEL_HEIGHT;
        ng.ID     = GID_BRICKS;
        ng.userData = NULL;
        if (initGadget(gr, gg[0], gw, &ng))
        {
            if (gg[1] = AllocMem(sizeof(*gg[1]), MEMF_PUBLIC))
            {
                ng.left   = PANEL_WIDTH;
                ng.top    = MENU_HEIGHT;
                ng.width  = BOARD_WIDTH;
                ng.height = BOARD_HEIGHT;
                ng.ID     = GID_BOARD;
                if (initGadget(gr, gg[1], gw, &ng))
                {
                    if (gg[2] = AllocMem(sizeof(*gg[2]), MEMF_PUBLIC))
                    {
                        ng.left   = 0;
                        ng.top    = 0;
                        ng.width  = MENU_WIDTH;
                        ng.height = MENU_HEIGHT;
                        ng.ID     = GID_MENU;
                        if (initGadget(gr, gg[2], gw, &ng))
                        {
                            AddGList(gr->control->window, gw->gadgets->gadget, -1, -1, NULL);
                            return(TRUE);
                        }
                        FreeMem(gg[2], sizeof(*gg[2]));
                    }
                    closeGadget(gr, gg[2]);
                }
                FreeMem(gg[1], sizeof(*gg[1]));
            }
            closeGadget(gr, gg[1]);
        }
        FreeMem(gg[0], sizeof(*gg[0]));
    }
    return(FALSE);
}

VOID closeWindow(struct gameRoot *gr, struct gameWindow *gw)
{
    struct gameGadget *gg, *nextgg;

    if (gw->gadgets)
    {
        RemoveGList(gr->control->window, gw->gadgets->gadget, -1);
    }

    gg = gw->gadgets;
    while (gg)
    {
        nextgg = gg->succ;
        closeGadget(gr, gg);
        FreeMem(gg, sizeof(*gg));
        gg = nextgg;
    }
}

BOOL initGadget(struct gameRoot *gr, struct gameGadget *gg, struct gameWindow *gw, struct newGadget *ng)
{
    if (gg->gadget = AllocMem(sizeof(*gg->gadget), MEMF_PUBLIC))
    {
        struct Gadget *gad = gg->gadget;

        if (gw->gadgets)
            gad->NextGadget = gw->gadgets->gadget;
        else
            gad->NextGadget = NULL;
        gad->LeftEdge       = ng->left;
        gad->TopEdge        = ng->top;
        gad->Width          = ng->width;
        gad->Height         = ng->height,
        gad->Flags          = GFLG_GADGHNONE;
        gad->Activation     = GACT_IMMEDIATE;
        gad->GadgetType     = GTYP_BOOLGADGET;
        gad->GadgetText     = NULL;
        gad->GadgetRender   = NULL;
        gad->SelectRender   = NULL;
        gad->MutualExclude  = 0;
        gad->SpecialInfo    = NULL;
        gad->GadgetID       = ng->ID;
        gad->UserData       = ng->userData;

        if (gw->gadgets)
            gw->gadgets->pred = gg;
        gg->succ = gw->gadgets;
        gg->pred = NULL;
        gw->gadgets = gg;

        return(TRUE);
    }
    return(FALSE);
}

VOID closeGadget(struct gameRoot *gr, struct gameGadget *gg)
{
    FreeMem(gg->gadget, sizeof(*gg->gadget));
}
