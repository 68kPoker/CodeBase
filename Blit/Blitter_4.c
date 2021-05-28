
/* $Id$ */

/* Ten plik zawiera podstawowe funkcje rysujâce kafle
 * jedno- i dwuwarstwowe oraz BOBy.
 */

#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>

#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#include "Screen.h"
#include "Blitter.h"

extern __far struct Custom custom; /* Ukîady specjalizowane */
extern void hookEntry();

struct blitMessage
{
    struct Layer *layer;
    struct Rectangle bounds;
    LONG offsetX, offsetY;
};

struct blitInfo
{
    struct BitMap *srcBitMap;
    WORD srcX, srcY;
    WORD destX, destY;
    WORD bltWidth, bltHeight;
};

/* drawTile: Rysowanie kafli jednowarstwowych.
 * Uwaga: wspóîrzëdne i rozmiar X sâ dosuwane do najbliûszej
 * wartoôci podzielnej przez 16.
 */

VOID drawTile(struct BitMap *srcBitMap, WORD srcX, WORD srcY, struct BitMap *destBitMap, WORD destX, WORD destY, WORD bltWidth, WORD bltHeight)
{
    WORD srcWordX     = Word(srcX); /* Wspóîrzëdne w kaflach */
    WORD destWordX    = Word(destX);
    WORD bltWordWidth = Word(bltWidth);

    WORD srcBPR  = srcBitMap->BytesPerRow;
    WORD destBPR = destBitMap->BytesPerRow;

    UBYTE destDepth = destBitMap->Depth, nPlane;

    LONG srcOffset  = (srcBPR * srcY) + (srcWordX << 1);
    LONG destOffset = (destBPR * destY) + (destWordX << 1);

    struct Custom *custPtr = &custom;

    srcBPR -= bltWordWidth << 1;
    destBPR -= bltWordWidth << 1;

    OwnBlitter();

    for (nPlane = 0; nPlane < destDepth; nPlane++)
    {
        WaitBlit();
        custPtr->bltcon0 = A_TO_D|SRCA|DEST;
        custPtr->bltcon1 = 0;
        custPtr->bltapt  = srcBitMap->Planes[nPlane] + srcOffset;
        custPtr->bltdpt  = destBitMap->Planes[nPlane] + destOffset;
        custPtr->bltamod = srcBPR;
        custPtr->bltdmod = destBPR;
        custPtr->bltafwm = 0xffff;
        custPtr->bltalwm = 0xffff;
        custPtr->bltsize = (bltHeight << 6)|bltWordWidth;
    }

    DisownBlitter();
}

/* drawTileLayers: Rysowanie kafli dwuwarstwowych.
 * Uwaga: wspóîrzëdne i rozmiar X sâ dosuwane do najbliûszej
 * wartoôci podzielnej przez 16.
 */

VOID drawTileLayers(struct BitMap *srcBitMap, WORD srcX, WORD srcY, struct BitMap *backBitMap, WORD backX, WORD backY, struct BitMap *destBitMap, WORD destX, WORD destY, PLANEPTR maskPlane, WORD mskX, WORD mskY, WORD maskBPR, WORD bltWidth, WORD bltHeight)
{
    WORD mskWordX     = Word(mskX);
    WORD srcWordX     = Word(srcX); /* Wspóîrzëdne w kaflach */
    WORD backWordX    = Word(backX);
    WORD destWordX    = Word(destX);
    WORD bltWordWidth = Word(bltWidth);

    WORD srcBPR  = srcBitMap->BytesPerRow;
    WORD backBPR = backBitMap->BytesPerRow;
    WORD destBPR = destBitMap->BytesPerRow;

    UBYTE destDepth = destBitMap->Depth, nPlane;

    LONG mskOffset  = (maskBPR * mskY) + (mskWordX << 1);
    LONG srcOffset  = (srcBPR * srcY) + (srcWordX << 1);
    LONG backOffset = (backBPR * backY) + (backWordX << 1);
    LONG destOffset = (destBPR * destY) + (destWordX << 1);

    struct Custom *custPtr = &custom;

    maskBPR -= bltWordWidth << 1;
    srcBPR -= bltWordWidth << 1;
    backBPR -= bltWordWidth << 1;
    destBPR -= bltWordWidth << 1;

    OwnBlitter();

    for (nPlane = 0; nPlane < destDepth; nPlane++)
    {
        WaitBlit();
        custPtr->bltcon0 = ABC|ABNC|NABC|NANBC|SRCA|SRCB|SRCC|DEST;
        custPtr->bltcon1 = 0;
        custPtr->bltapt  = maskPlane + mskOffset;
        custPtr->bltbpt  = srcBitMap->Planes[nPlane] + srcOffset;
        custPtr->bltcpt  = backBitMap->Planes[nPlane] + backOffset;
        custPtr->bltdpt  = destBitMap->Planes[nPlane] + destOffset;
        custPtr->bltamod = maskBPR;
        custPtr->bltbmod = srcBPR;
        custPtr->bltcmod = backBPR;
        custPtr->bltdmod = destBPR;
        custPtr->bltafwm = 0xffff;
        custPtr->bltalwm = 0xffff;
        custPtr->bltsize = (bltHeight << 6)|bltWordWidth;
    }

    DisownBlitter();
}

__saveds void hookTileRastPort(struct Hook *hook, struct RastPort *rPort, struct blitMessage *blitMsg)
{
    struct Rectangle *rect = &blitMsg->bounds, destRect;
    struct blitInfo *bi = (struct blitInfo *)hook->h_Data;
    struct Rectangle blitRect;
    WORD srcX, srcY;
    WORD width = rect->MaxX - rect->MinX + 1;
    WORD height = rect->MaxY - rect->MinY + 1;

    struct Window *win = (struct Window *)rPort->RP_User;
    struct screenInfo *scrInfo = (struct screenInfo *)win->WScreen->UserData;

    blitRect.MinX = MAX(blitMsg->offsetX, bi->destX);
    blitRect.MinY = MAX(blitMsg->offsetY, bi->destY);
    blitRect.MaxX = MIN(blitMsg->offsetX + width - 1, bi->destX + bi->bltWidth - 1);
    blitRect.MaxY = MIN(blitMsg->offsetY + height - 1, bi->destY + bi->bltHeight - 1);

    if (blitRect.MinX > blitRect.MaxX || blitRect.MinY > blitRect.MaxY)
    {
        return;
    }

    srcX = bi->srcX + blitRect.MinX - bi->destX;
    srcY = bi->srcY + blitRect.MinY - bi->destY;
    destRect.MinX = rect->MinX + blitRect.MinX - blitMsg->offsetX;
    destRect.MinY = rect->MinY + blitRect.MinY - blitMsg->offsetY;
    width  = blitRect.MaxX - blitRect.MinX + 1;
    height = blitRect.MaxY - blitRect.MinY + 1;

    destRect.MaxX = destRect.MinX + width - 1;
    destRect.MaxY = destRect.MinY + height - 1;

    OrRectRegion(scrInfo->syncRegion[scrInfo->toggleFrame], &destRect);

    drawTile(bi->srcBitMap, srcX, srcY, rPort->BitMap, destRect.MinX, destRect.MinY, width, height);
}

void drawTileRastPort(struct BitMap *srcBitMap, WORD srcX, WORD srcY, struct RastPort *destRPort, WORD destX, WORD destY, WORD bltWidth, WORD bltHeight)
{
    struct blitInfo bi;
    struct Hook hook = { 0 };
    struct Window *win = (struct Window *)destRPort->RP_User;

    if (!win)
    {
        return;
    }

    /* struct Rectangle *clip = (struct Rectangle *)destRPort->RP_User; */

    bi.srcBitMap = srcBitMap;
    bi.srcX = srcX;
    bi.srcY = srcY;
    bi.destX = destX;
    bi.destY = destY;
    bi.bltWidth = bltWidth;
    bi.bltHeight = bltHeight;

    hook.h_Data = (APTR)&bi;
    hook.h_Entry = (ULONG(*)())hookEntry;
    hook.h_SubEntry = (ULONG(*)())hookTileRastPort;

    DoHookClipRects(&hook, destRPort, NULL);
}
