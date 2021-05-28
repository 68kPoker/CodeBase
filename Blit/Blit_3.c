
#include <graphics/gfx.h>
#include <clib/graphics_protos.h>

__far extern struct Custom custom;


BOOL allocBitMaps(struct BitMap *bitmaps[], WORD width, WORD height, UBYTE depth)
{
    if (bitmaps[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (bitmaps[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            return(TRUE);
        }
        FreeBitMap(bitmaps[0]);
    }
    return(FALSE);
}

VOID freeBitMaps(struct BitMap *bitmaps[])
{
    FreeBitMap(bitmaps[1]);
    FreeBitMap(bitmaps[0]);
}

void iconBlit(struct BitMap *gfx, struct BitMap *bm, WORD depth, LONG offset, LONG destoffset, WORD mod, WORD destmod, WORD width, WORD height)
{
    WORD i;
    struct Custom *cust = &custom;

    OwnBlitter();
    for (i = 0; i < depth; i++)
    {
        WaitBlit();
        cust->bltcon0 = SRCA|DEST|A_TO_D;
        cust->bltcon1 = 0;
        cust->bltapt  = gfx->Planes[i] + offset;
        cust->bltdpt  = bm->Planes[i] + destoffset;
        cust->bltamod = mod;
        cust->bltdmod = destmod;
        cust->bltafwm = 0xffff;
        cust->bltalwm = 0xffff;
        cust->bltsizv = height;
        cust->bltsizh = width;
    }
    DisownBlitter();
}

void bltTemplate(struct BitMap *gfx, WORD left, WORD top, WORD width, WORD height, struct BitMap *bm)
{
    WORD x, y;
    WORD totalwidth = GetBitMapAttr(bm, BMA_WIDTH);
    WORD totalheight = GetBitMapAttr(bm, BMA_HEIGHT);
    WORD depth = GetBitMapAttr(bm, BMA_DEPTH);

    LONG offset = (gfx->BytesPerRow * top) + ((left >> 4) << 1);
    LONG destoffset;
    WORD mod = gfx->BytesPerRow - ((width >> 4) << 1);
    WORD destmod = bm->BytesPerRow - ((width >> 4) << 1);

    for (y = 0; y < totalheight; y += height)
    {
        for (x = 0; x < totalwidth; x += width)
        {
            destoffset = (bm->BytesPerRow * y) + ((x >> 4) << 1);
            iconBlit(gfx, bm, depth, offset, destoffset, mod, destmod, width >> 4, height);
        }
    }
}

__saveds void fastBackFill(register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct backInfo *bi)
{
    struct BitMap *gfx = (struct BitMap *)hook->h_Data;
    struct BitMap *bm = rp->BitMap;
    WORD depth = bm->Depth;
    struct Rectangle *rect = &bi->bounds;
    LONG offset = (gfx->BytesPerRow * bi->y) + ((bi->x >> 4) << 1);
    LONG destoffset = (bm->BytesPerRow * rect->MinY) + ((rect->MinX >> 4) << 1);
    WORD width = (rect->MaxX - rect->MinX + 1) >> 4;
    WORD height = rect->MaxY - rect->MinY + 1;
    WORD mod = gfx->BytesPerRow - (width << 1);
    WORD destmod = bm->BytesPerRow - (width << 1);

    iconBlit(gfx, bm, depth, offset, destoffset, mod, destmod, width, height);
}

__saveds void fastScreenBackFill(register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct backInfo *bi)
{
    struct BitMap *gfx = (struct BitMap *)hook->h_Data;
    struct BitMap *bm = rp->BitMap;
    WORD depth = bm->Depth;
    struct Rectangle *rect = &bi->bounds;
    LONG offset = (gfx->BytesPerRow * rect->MinY) + ((rect->MinX >> 4) << 1);
    LONG destoffset = (bm->BytesPerRow * rect->MinY) + ((rect->MinX >> 4) << 1);
    WORD width = (rect->MaxX - rect->MinX + 1) >> 4;
    WORD height = rect->MaxY - rect->MinY + 1;
    WORD mod = gfx->BytesPerRow - (width << 1);
    WORD destmod = bm->BytesPerRow - (width << 1);

    iconBlit(gfx, bm, depth, offset, destoffset, mod, destmod, width, height);
}
