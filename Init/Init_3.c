
/* Init.c: Game init */

/* $Id: Init.c,v 1.1 12/.0/.0 .1:.1:.4 Robert Exp $ */

#include <stdio.h>
#include "debug.h"

#include "Init.h"
#include "Tile.h"

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

struct Library *IntuitionBase;

BOOL openLibs(void)
{
    if (IntuitionBase = OpenLibrary("intuition.library", 39))
    {
        return(TRUE);
    }
    printf("This program requires Amiga OS3.0+\n");
    return(FALSE);
}

void closeLibs(void)
{
    CloseLibrary(IntuitionBase);
}

/* Init Screen part */
BOOL initGameScreen(struct gameInit *gi)
{
    if (gi->bm[0] = allocBitMap())
    {
        if (gi->bm[1] = allocBitMap())
        {
            if (gi->s = openScreen(gi->bm[0], &gi->tf))
            {
                if (addCopperList(&gi->s->ViewPort))
                {
                    if (addCopperInt(&gi->copis, &gi->copdata, &gi->s->ViewPort))
                    {
                        return(TRUE);
                    }
                }
                CloseScreen(gi->s);
                CloseFont(gi->tf);
            }
            FreeBitMap(gi->bm[1]);
        }
        FreeBitMap(gi->bm[0]);
    }
    return(FALSE);
}

void freeGameScreen(struct gameInit *gi)
{
    remCopperInt(&gi->copis);
    CloseScreen(gi->s);
    CloseFont(gi->tf);
    FreeBitMap(gi->bm[1]);
    FreeBitMap(gi->bm[0]);
}

/* Init game graphics */
BOOL initGameGfx(struct gameInit *gi)
{
    struct IFFHandle *iff;
    BOOL result = FALSE;

    if (iff = openIFF("Data1/Gfx/Graphics.iff", IFFF_READ))
    {
        if (scanILBM(iff))
        {
            if (loadCMAP(iff, gi->s))
            {
                if (gi->gfx = loadBitMap(iff))
                {
                    result = TRUE;
                }
            }
        }
        closeIFF(iff);
    }
    return(result);
}

void freeGameGfx(struct gameInit *gi)
{
    FreeBitMap(gi->gfx);
}

/* Init game sound effects */
BOOL initGameSfx(struct gameInit *gi)
{
    if (gi->ioa = allocChannels())
    {
        if (loadSample("Data1/Sfx/Step.iff", gi->samples + SAMPLE_DIG))
        {
            if (loadSample("Data1/Sfx/Box.iff", gi->samples + SAMPLE_BOX))
            {
                if (loadSample("Data1/Sfx/Coins.iff", gi->samples + SAMPLE_KEY))
                {
                    if (loadSample("Data1/Sfx/Chew.iff", gi->samples + SAMPLE_FRUIT))
                    {
                        return(TRUE);
                    }
                    freeSample(gi->samples + SAMPLE_KEY);
                }
                freeSample(gi->samples + SAMPLE_BOX);
            }
            freeSample(gi->samples + SAMPLE_DIG);
        }
        freeChannels(gi->ioa);
    }
    return(FALSE);
}

void freeGameSfx(struct gameInit *gi)
{
    freeSample(gi->samples + SAMPLE_FRUIT);
    freeSample(gi->samples + SAMPLE_KEY);
    freeSample(gi->samples + SAMPLE_BOX);
    freeSample(gi->samples + SAMPLE_DIG);
    freeChannels(gi->ioa);
}

/* Init game windows */
BOOL initGameWindows(struct gameInit *gi)
{
    if (initWindow(&gi->wi, gi->gfx))
    {
        if (gi->w = openBDWindow(gi->s))
        {
            return(TRUE);
        }
        freeWindow(&gi->wi);
    }
    return(FALSE);
}

void freeGameWindows(struct gameInit *gi)
{
    CloseWindow(gi->w);
    freeWindow(&gi->wi);
}

BOOL initGameBoard(struct gameInit *gi, STRPTR name)
{
    BOOL result;
    struct gameState *gs = &gi->state;

    if ((result = loadBoard(name, gs)) == LOAD_FAILURE)
    {
        initBoard(&gs->board);
        result = TRUE;
    }

    if (!(scanBoard(&gs->board, gs)))
    {
        initBoard(&gs->board);
        result = TRUE;
        scanBoard(&gs->board, gs);
    }
    else
    {
        D(bug("Scan OK (%d %d %d)\n", gs->boxes, gs->heroX, gs->heroY));
    }


    drawBoard(gi, gi->w->RPort, &gs->board, gi->bm[1], gi->gfx, 0, 0, 19, 14);
    SetSignal(0L, 1L << gi->copdata.signal);
    Wait(1L << gi->copdata.signal);
    drawTile(gi->bm[1], 0, 0, gi->bm[0], 0, 0, 320, 240);

    RefreshGList(gi->wi.gads, gi->w, NULL, -1);

    return(result);
}

BOOL initGame(struct gameInit *gi)
{
    if (openLibs())
    {
        if (initGameScreen(gi))
        {
            if (initGameGfx(gi))
            {
                if (initGameSfx(gi))
                {
                    if (initImages(gi->img, gi->gfx))
                    {
                        if (gi->w = openBDWindow(gi->s))
                        {
                            if (gi->backw = openReqWindow(gi->w))
                            {
                                return(TRUE);
                            }
                            CloseWindow(gi->w);
                        }
                        freeImages(gi->img);
                    }
                    freeGameSfx(gi);
                }
                freeGameGfx(gi);
            }
            freeGameScreen(gi);
        }
        closeLibs();
    }
    return(FALSE);
}

void freeGame(struct gameInit *gi)
{
    CloseWindow(gi->backw);
    CloseWindow(gi->w);
    freeImages(gi->img);
    freeGameSfx(gi);
    freeGameGfx(gi);
    freeGameScreen(gi);
    closeLibs();
}
