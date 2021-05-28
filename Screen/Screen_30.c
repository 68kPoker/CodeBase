
/* $Id$ */

/* Ustalenie tryb ekranu, otwarcie ekranu, obsîuga podwójnego buforowania,
 * synchronizacja buforów.
 */

#include <stdio.h>
#include <intuition/screens.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"
#include "Blitter.h"

extern __far struct Custom custom; /* Ukîady specjalizowane */

extern VOID myCopperIs(VOID);

/* Ustalenie trybu ekranu dla danej rozdzielczoôci. Monitor pobierany
 * jest z domyôlnego ekranu publicznego. Nastëpnie wyliczany jest
 * tryb ekranu.
 */

ULONG obtainModeID(UWORD width, UWORD height, UBYTE depth)
{
    struct Screen *pubScreen;

    if (pubScreen = LockPubScreen(NULL))
    {
        ULONG monitorID = GetVPModeID(&pubScreen->ViewPort) & MONITOR_ID_MASK;
        ULONG modeID;
        printf("MonitorID = $%X\n", monitorID);

        if ((modeID = BestModeID(
            BIDTAG_MonitorID, monitorID,
            BIDTAG_NominalWidth, width,
            BIDTAG_NominalHeight, height,
            BIDTAG_Depth, depth,
            TAG_DONE)) != INVALID_ID)
        {
            printf("ModeID: $%X\n", modeID);
            UnlockPubScreen(NULL, pubScreen);
            return(modeID);
        }
        else
            printf("Couldn't find best mode ID!\n");
        UnlockPubScreen(NULL, pubScreen);
    }
    else
        printf("Couldn't lock default public screen!\n");
    return(INVALID_ID);
}

/* Ustalenie parametrów trybu ekranu */

BOOL getModeIDInfo(ULONG modeID, struct Rectangle *dClip)
{
    DisplayInfoHandle dih;
    struct DimensionInfo dimInfo;

    if (dih = FindDisplayInfo(modeID))
    {
        if (GetDisplayInfoData(dih, (UBYTE *)&dimInfo, sizeof(dimInfo), DTAG_DIMS, modeID) > 0)
        {
            *dClip = dimInfo.Nominal;
            printf("Display Info:\n");
            printf("Nominal: %d %d\n", dClip->MaxX - dClip->MinX + 1, dClip->MaxY - dClip->MinY + 1);
            return(TRUE);
        }
        else
            printf("Couldn't get display info data!\n");
    }
    else
        printf("Couldn't find display info!\n");
    return(FALSE);
}

/* Alokacja bitmap i otwarcie ekranu */

struct Screen *openScreen(STRPTR title, ULONG modeID, UBYTE depth, struct screenInfo *scrInfo)
{
    struct Rectangle dClip;

    if (getModeIDInfo(modeID, &dClip))
    {
        WORD rasWidth = dClip.MaxX - dClip.MinX + 1;
        WORD rasHeight = dClip.MaxY - dClip.MinY + 1;

        if (scrInfo->scrBitMaps[0] = AllocBitMap(rasWidth, rasHeight, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            if (scrInfo->scrBitMaps[1] = AllocBitMap(rasWidth, rasHeight, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
            {
                struct Screen *screen;

                if (scrInfo->screen = screen = OpenScreenTags(NULL,
                    SA_DClip, &dClip,
                    SA_Depth, depth,
                    SA_DisplayID, modeID,
                    SA_Quiet, TRUE,
                    SA_Title, title,
                    SA_Exclusive, TRUE,
                    SA_BackFill, LAYERS_NOBACKFILL,
                    SA_BitMap, scrInfo->scrBitMaps[0],
                    SA_ShowTitle, FALSE,
                    TAG_DONE))
                {
                    if (scrInfo->dbufInfo = AllocDBufInfo(&screen->ViewPort))
                    {
                        scrInfo->safeToWrite = TRUE;
                        scrInfo->toggleFrame = 1;
                        if (scrInfo->safePort = CreateMsgPort())
                        {
                            scrInfo->dbufInfo->dbi_SafeMessage.mn_ReplyPort = scrInfo->safePort;
                            scrInfo->dClip = dClip;
                            /* screen->RastPort.RP_User = (APTR)&scrInfo->dClip; */

                            screen->UserData = (APTR)scrInfo;
                            return(screen);
                        }
                        FreeDBufInfo(scrInfo->dbufInfo);
                    }
                    CloseScreen(screen);
                }
                FreeBitMap(scrInfo->scrBitMaps[1]);
            }
            FreeBitMap(scrInfo->scrBitMaps[0]);
        }
    }
    return(NULL);
}

/* Dodaj obsîugë Coppera */

BOOL addCopper(struct screenInfo *scrInfo, UBYTE pri)
{
    struct UCopList *ucl;

    scrInfo->copperIs.is_Node.ln_Name = "Game";
    scrInfo->copperIs.is_Node.ln_Pri = pri;
    scrInfo->copperIs.is_Code = myCopperIs;
    scrInfo->copperIs.is_Data = &scrInfo->copper;

    scrInfo->copper.task = FindTask(NULL);
    scrInfo->copper.viewPort = &scrInfo->screen->ViewPort;

    if ((scrInfo->copper.signal = AllocSignal(-1)) != -1)
    {
        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
        {
            const WORD copInsCount = 6;

            CINIT(ucl, copInsCount);
            CWAIT(ucl, 0, 0);
            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
            CEND(ucl);

            AddIntServer(INTB_COPER, &scrInfo->copperIs);

            Forbid();
            scrInfo->screen->ViewPort.UCopIns = ucl;
            Permit();

            RethinkDisplay();
            return(TRUE);
        }
        FreeSignal(scrInfo->copper.signal);
    }
    return(FALSE);
}

BOOL addRegions(struct screenInfo *scrInfo)
{
    if (scrInfo->syncRegion[0] = NewRegion())
    {
        if (scrInfo->syncRegion[1] = NewRegion())
        {
            return(TRUE);
        }
        DisposeRegion(scrInfo->syncRegion[0]);
    }
    return(FALSE);
}

VOID syncScreen(struct screenInfo *scrInfo)
{
    struct RegionRectangle *regRect;
    struct Rectangle *rect;
    WORD frame = scrInfo->toggleFrame;
    WORD baseX, baseY, rectX, rectY;
    WORD width, height;
    struct Region *update;

    if (update = NewRegion())
    {
        OrRegionRegion(scrInfo->syncRegion[frame ^ 1], update);
        XorRegionRegion(scrInfo->syncRegion[frame], update);
        AndRegionRegion(scrInfo->syncRegion[frame ^ 1], update);
        AndRectRegion(update, &scrInfo->dClip);

        rect = &update->bounds;
        baseX = rect->MinX;
        baseY = rect->MinY;

        for (regRect = update->RegionRectangle; regRect != NULL; regRect = regRect->Next)
        {
            rect = &regRect->bounds;
            rectX = rect->MinX;
            rectY = rect->MinY;
            width = rect->MaxX - rectX + 1;
            height = rect->MaxY - rectY + 1;

            rectX += baseX;
            rectY += baseY;

            drawTile(scrInfo->scrBitMaps[frame ^ 1], rectX, rectY, scrInfo->scrBitMaps[frame], rectX, rectY, width, height);
        }
        ClearRegion(scrInfo->syncRegion[frame ^ 1]);
        DisposeRegion(update);
    }
}

BOOL remRegions(struct screenInfo *scrInfo)
{
    DisposeRegion(scrInfo->syncRegion[1]);
    DisposeRegion(scrInfo->syncRegion[0]);
}

VOID remCopper(struct screenInfo *scrInfo)
{
    RemIntServer(INTB_COPER, &scrInfo->copperIs);
    FreeSignal(scrInfo->copper.signal);
}

/* Zamkniëcie ekranu */

VOID closeScreen(struct Screen *screen)
{
    struct screenInfo *scrInfo = (struct screenInfo *)screen->UserData;

    /* Sprawdzamy czy odebrano wiadomoôci */

    if (!scrInfo->safeToWrite)
    {
        while (!GetMsg(scrInfo->safePort))
        {
            WaitPort(scrInfo->safePort);
        }
    }
    DeleteMsgPort(scrInfo->safePort);
    FreeDBufInfo(scrInfo->dbufInfo);
    CloseScreen(screen);
    FreeBitMap(scrInfo->scrBitMaps[1]);
    FreeBitMap(scrInfo->scrBitMaps[0]);
}
