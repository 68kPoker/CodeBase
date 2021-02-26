
/* Tile.c: Tile drawing functions */

#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <exec/memory.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Tile.h"
#include "Game.h"

#define WordWidth(w) (((w)+15)>>4)

void drawMessage(struct gameInit *gi, struct RastPort *rp, STRPTR text, WORD x, WORD y);

__far extern struct Custom custom;

/* drawTile: Draw 16-pixel width aligned interleaved bitmap */
void drawTile(struct BitMap *gfx, UWORD sx, UWORD sy, struct BitMap *bm, UWORD dx, UWORD dy, UWORD width, UWORD height)
{
    LONG srcoffset = (gfx->BytesPerRow * sy) + ((sx >> 4) << 1);
    LONG dstoffset = (bm->BytesPerRow * dy) + ((dx >> 4) << 1);
    WORD wordWidth = (width + 15) >> 4;
    WORD srcmodulo = gfx->Planes[1] - gfx->Planes[0] - (wordWidth << 1);
    WORD dstmodulo = bm->Planes[1] - bm->Planes[0] - (wordWidth << 1);

    OwnBlitter();

    WaitBlit();

    custom.bltcon0 = SRCA | DEST | A_TO_D;
    custom.bltcon1 = 0;
    custom.bltapt  = gfx->Planes[0] + srcoffset;
    custom.bltdpt  = bm->Planes[0] + dstoffset;
    custom.bltamod = srcmodulo;
    custom.bltdmod = dstmodulo;
    custom.bltafwm = 0xffff;
    custom.bltalwm = 0xffff;
    custom.bltsizv = height * gfx->Depth;
    custom.bltsizh = wordWidth;

    DisownBlitter();
}

void updateStatus(struct RastPort *rp, struct Board *board)
{
    UBYTE text[20];

    sprintf(text, "%05d", board->info.points);

    SetAPen(rp, 20);
    RectFill(rp, 41, 3, 81, 12);
    drawMessage(NULL, rp, text, 42, 4);

    sprintf(text, "%2d", board->info.keys);

    SetAPen(rp, 20);
    RectFill(rp, 121, 3, 161, 12);
    drawMessage(NULL, rp, text, 122, 4);

    sprintf(text, "%2d", board->info.placed);

    SetAPen(rp, 20);
    RectFill(rp, 201, 3, 241, 12);
    drawMessage(NULL, rp, text, 202, 4);
}

void drawBoard(struct RastPort *rp, struct Board *board, struct BitMap *bm, struct BitMap *gfx, WORD sx, WORD sy, WORD ex, WORD ey)
{
    WORD x, y;
    UBYTE text[20];

    struct RastPort auxrp;

    for (y = sy; y <= ey; y++)
    {
        for (x = sx; x <= ex; x++)
        {
            struct Cell *c = &board->board[y][x];

            WORD srcy = 8 + c->kind;
            WORD srcx = c->subKind;

            BltBitMap(gfx, srcx << 4, srcy << 4, bm, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
        }
    }
    BltBitMap(gfx, 0, 96, bm, 0, 0, 320, 16, 0xc0, 0xff, NULL);

    sprintf(text, "Poziom: %d", board->info.level);

    InitRastPort(&auxrp);
    auxrp.BitMap = bm;
    auxrp.Layer = rp->Layer;
    SetFont(&auxrp, rp->Font);

    drawMessage(NULL, &auxrp, text, 258, 4);

    updateStatus(&auxrp, board);
}

/* Cut Image from BitMap */
BOOL cutImage(struct Image *img, struct BitMap *bm, WORD x, WORD y, WORD width, WORD height)
{
    UBYTE depth = GetBitMapAttr(bm, BMA_DEPTH);
    LONG planeSize = WordWidth(width) * height;
    LONG fullSize = planeSize * depth;
    struct BitMap aux;
    WORD bpr = WordWidth(width) << 1;
    WORD i;

    img->LeftEdge = img->TopEdge = 0;
    img->Width = width;
    img->Height = height;
    img->Depth = depth;
    img->PlanePick = 0xff;
    img->PlaneOnOff = 0x00;
    img->NextImage = NULL;

    if (img->ImageData = AllocVec(fullSize * sizeof(UWORD), MEMF_CHIP))
    {
        InitBitMap(&aux, depth, width, height);
        aux.Planes[0] = (PLANEPTR)img->ImageData;
        for (i = 1; i < depth; i++)
        {
            aux.Planes[i] = aux.Planes[i - 1] + (planeSize << 1);
        }
        BltBitMap(bm, x, y, &aux, 0, 0, bpr << 3, height, 0xc0, 0xff, NULL);
        return(TRUE);
    }
    return(FALSE);
}

void freeImage(struct Image *img)
{
    FreeVec(img->ImageData);
}
