
#include <intuition/intuition.h>
#include <graphics/rpattr.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#include "Screen.h"
#include "Windows.h"

#define MENUHEIGHT 16

struct Window *openWindow(struct Screen *screen)
{
    struct Window *win;

    if (win = OpenWindowTags(NULL,
        WA_CustomScreen,    screen,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           screen->Width,
        WA_Height,          screen->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
        WA_ReportMouse,     TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_NoCareRefresh,   TRUE,
        TAG_DONE))
    {
        win->RPort->RP_User = (APTR)win;
        return(win);
    }
    return(NULL);
}

struct Window *openMenu(struct Window *base)
{
    struct Window *win;

    if (win = OpenWindowTags(NULL,
        WA_CustomScreen,    base->WScreen,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           base->WScreen->Width,
        WA_Height,          MENUHEIGHT,
        WA_Backdrop,        FALSE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS,
        WA_ReportMouse,     FALSE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        win->RPort->RP_User = (APTR)win;
        return(win);
    }
    return(NULL);

}

VOID updateBoard(struct Window *win, struct boardInfo *board, BOOL redraw)
{
    WORD xPos, yPos;
    struct screenInfo *scrInfo = (struct screenInfo *)win->WScreen->UserData;
    struct RastPort *rastPort = win->RPort;
    WORD toggleFrame = scrInfo->toggleFrame;
    struct Rectangle rect;

    GetRPAttrs(rastPort, RPTAG_DrawBounds, &rect, TAG_DONE);

    for (yPos = rect.MinY >> 4; yPos < (rect.MaxY + 1) >> 4; yPos++)
    {
        for (xPos = rect.MinX >> 4; xPos < (rect.MaxX + 1) >> 4; xPos++)
        {
            struct tileInfo *tile = &board->tileArray[yPos][xPos];
            if (tile->updateFlag || redraw)
            {
                struct Rectangle rect;
                rect.MinX = xPos << 4;
                rect.MinY = yPos << 4;
                rect.MaxX = rect.MinX + 15;
                rect.MaxY = rect.MinY + 15;

                SetAPen(rastPort, 2);

                drawTileRastPort(scrInfo->gfxBitMap, tile->floorGfx << 4, 0, rastPort, rect.MinX, rect.MinY, 16, 16);

                tile->updateFlag = FALSE;
            }
        }
    }
}

BOOL mainLoop(struct Window *win, struct boardInfo *board)
{
    ULONG signalMasks[SIGNAL_COUNT], totalMask;
    BOOL done = FALSE;
    WORD nSig;
    struct screenInfo *scrInfo = (struct screenInfo *)win->WScreen->UserData;
    struct MsgPort *userPort = win->UserPort, *safePort = scrInfo->safePort;
    WORD toggleFrame = 1;
    UBYTE text[5];
    WORD counter = 0;
    BOOL paintMode = FALSE;
    WORD prevX = 0, prevY = 0;
    struct Window *menu = NULL;
    BOOL refresh = FALSE;

    signalMasks[SIGNAL_WINDOW] = 1L << userPort->mp_SigBit;
    signalMasks[SIGNAL_SAFE] = 1L << safePort->mp_SigBit;
    signalMasks[SIGNAL_COPPER] = 1L << scrInfo->copper.signal;

    totalMask = 0L;
    for (nSig = 0; nSig < SIGNAL_COUNT; nSig++)
    {
        totalMask |= signalMasks[nSig];
    }

    while (!done)
    {
        ULONG resultMask = Wait(totalMask);

        if (resultMask & signalMasks[SIGNAL_WINDOW])
        {
            struct IntuiMessage *msg;

            while (msg = GT_GetIMsg(userPort))
            {
                WORD tileX = msg->MouseX >> 4;
                WORD tileY = msg->MouseY >> 4;
                if (msg->Class == IDCMP_RAWKEY)
                {
                    if (msg->Code == ESC_KEY)
                    {
                        done = TRUE;
                    }
                }
                else if (msg->Class == IDCMP_MOUSEBUTTONS)
                {
                    if (msg->Code == IECODE_LBUTTON)
                    {
                        paintMode = TRUE;
                        board->tileArray[tileY][tileX].floorGfx = 1;
                        board->tileArray[tileY][tileX].updateFlag = TRUE;
                        prevX = tileX;
                        prevY = tileY;
                    }
                    else if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                    {
                        paintMode = FALSE;
                    }
                    else if (msg->Code == IECODE_RBUTTON)
                    {
                        if (menu == NULL)
                        {
                            menu = openMenu(win);
                        }
                        else
                        {
                            CloseWindow(menu);
                            menu = NULL;
                            refresh = TRUE;
                        }
                    }
                }
                else if (msg->Class == IDCMP_MOUSEMOVE)
                {
                    if (paintMode && (prevX != tileX || prevY != tileY))
                    {
                        board->tileArray[tileY][tileX].floorGfx = 1;
                        board->tileArray[tileY][tileX].updateFlag = TRUE;
                        prevX = tileX;
                        prevY = tileY;
                    }
                }
                GT_ReplyIMsg(msg);
            }
        }

        if (resultMask & signalMasks[SIGNAL_SAFE])
        {
            struct RastPort *rastPort = win->RPort;
            struct TextFont *font = rastPort->Font;

            rastPort->BitMap = scrInfo->scrBitMaps[toggleFrame];

            if (!scrInfo->safeToWrite)
            {
                while (!GetMsg(safePort))
                {
                    WaitPort(safePort);
                }
                scrInfo->safeToWrite = TRUE;
            }

            if (refresh)
            {
                struct Region *update;
                struct Rectangle rect = { 0, 0, 319, 15 };
                if (update = NewRegion())
                {
                    OrRectRegion(update, &rect);
                    struct Region *prevRegion = InstallClipRegion(win->WLayer, update);
                    updateBoard(win, board, TRUE);
                    InstallClipRegion(win->WLayer, prevRegion);
                    DisposeRegion(update);
                }
                refresh = FALSE;
            }
            else
            {
                updateBoard(win, board, FALSE);
            }

            if (menu)
            {
                menu->RPort->BitMap = rastPort->BitMap;
                drawTileRastPort(scrInfo->gfxBitMap, 0, 0, menu->RPort, 0, 0, menu->Width, menu->Height);
            }

            Move(rastPort, 0, font->tf_Baseline);
            SetAPen(rastPort, 1);
            sprintf(text, "%4d", counter++);
            Text(rastPort, text, 4);

            syncScreen(scrInfo);
        }

        if (resultMask & signalMasks[SIGNAL_COPPER])
        {
            if (scrInfo->safeToWrite)
            {
                WaitBlit();
                ChangeVPBitMap(&scrInfo->screen->ViewPort, scrInfo->scrBitMaps[toggleFrame], scrInfo->dbufInfo);
                scrInfo->safeToWrite = FALSE;
                scrInfo->toggleFrame = toggleFrame ^= 1;
            }
        }
    }
    if (menu)
    {
        CloseWindow(menu);
    }
    return(TRUE);
}
