
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"
#include "Windows.h"

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
        TAG_DONE))
    {
        return(win);
    }
    return(NULL);
}

VOID updateBoard(struct Window *win, struct boardInfo *board)
{
    WORD xPos, yPos;
    struct screenInfo *scrInfo = (struct screenInfo *)win->WScreen->UserData;
    struct RastPort *rastPort = win->RPort;
    WORD toggleFrame = scrInfo->toggleFrame;

    for (yPos = 0; yPos < BOARD_HEIGHT; yPos++)
    {
        for (xPos = 0; xPos < BOARD_WIDTH; xPos++)
        {
            struct tileInfo *tile = &board->tileArray[yPos][xPos];
            if (tile->updateFlag)
            {
                struct Rectangle rect;
                rect.MinX = xPos << 4;
                rect.MinY = yPos << 4;
                rect.MaxX = rect.MinX + 15;
                rect.MaxY = rect.MinY + 15;

                SetAPen(rastPort, 2);

                drawTileRastPort(scrInfo->gfxBitMap, 0, 0, rastPort, rect.MinX, rect.MinY, 16, 16);
                OrRectRegion(scrInfo->syncRegion[toggleFrame], &rect);

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
                        board->tileArray[tileY][tileX].updateFlag = TRUE;
                        prevX = tileX;
                        prevY = tileY;
                    }
                    else if (msg->Code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                    {
                        paintMode = FALSE;
                    }
                }
                else if (msg->Class == IDCMP_MOUSEMOVE)
                {
                    if (paintMode && (prevX != tileX || prevY != tileY))
                    {
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

            updateBoard(win, board);

            Move(rastPort, 0, font->tf_Baseline);
            SetAPen(rastPort, 1);
            sprintf(text, "%4d", counter++);
            Text(rastPort, text, 4);

            syncScreen(scrInfo);
        }

        if ((resultMask & signalMasks[SIGNAL_COPPER]) && scrInfo->safeToWrite)
        {
            WaitBlit();
            ChangeVPBitMap(&scrInfo->screen->ViewPort, scrInfo->scrBitMaps[toggleFrame], scrInfo->dbufInfo);
            scrInfo->safeToWrite = FALSE;
            scrInfo->toggleFrame = toggleFrame ^= 1;
        }
    }
    return(TRUE);
}
