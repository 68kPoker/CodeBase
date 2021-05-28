
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfx.h>

#include <clib/graphics_protos.h>

#define bitMapOffset(bm, x, y) (((y)*(bm)->BytesPerRow)+(((x)>>4)<<1))
#define blitModulo(bm, w) \
    ((bm)->Depth > 1 ? ((bm)->Planes[1] - (bm)->Planes[0]) : (bm)->BytesPerRow) - ((((w)+15)>>4)<<1);

enum { A, B, C, D };

union channel
{
    struct
    {
        PLANEPTR ptr;
        WORD mod;
    } addr;
    UWORD data;
};

extern __far struct Custom custom;

/* Universal Blitter routine */
void blit(UWORD con0, UWORD con1, union channel chan[], UWORD first, UWORD last, UWORD height, UWORD width)
{
    struct Custom *cust = &custom;

    OwnBlitter();
    WaitBlit();

    cust->bltcon0 = con0;
    cust->bltcon1 = con1;

    con0 & SRCA ? (cust->bltapt = chan[A].addr.ptr) : (cust->bltadat = chan[A].data);
    con0 & SRCB ? (cust->bltbpt = chan[B].addr.ptr) : (cust->bltbdat = chan[B].data);
    con0 & SRCC ? (cust->bltcpt = chan[C].addr.ptr) : (cust->bltcdat = chan[C].data);
    con0 & DEST ? (cust->bltdpt = chan[D].addr.ptr) : (cust->bltddat = chan[D].data);

    if (con0 & SRCA) (cust->bltamod = chan[A].addr.mod);
    if (con0 & SRCB) (cust->bltbmod = chan[B].addr.mod);
    if (con0 & SRCC) (cust->bltcmod = chan[C].addr.mod);
    if (con0 & DEST) (cust->bltdmod = chan[D].addr.mod);

    cust->bltafwm = first;
    cust->bltalwm = last;

    /* ECS required */
    cust->bltsizv = height;
    cust->bltsizh = width;

    DisownBlitter();
}

/* Usage: interleaved tile drawing */
void blitTile(struct BitMap *tile_bm, WORD tile_x, WORD tile_y, struct BitMap *dest_bm, WORD dest_x, WORD dest_y, UWORD width, UWORD height)
{
    struct channel chan[4];
    UBYTE depth = tile_bm->Depth;

    chan[A].addr.ptr = tile_bm->Planes[0] + bitMapOffset(tile_bm, tile_x, tile_y);
    chan[D].addr.ptr = dest_bm->Planes[0] + bitMapOffset(dest_bm, dest_x, dest_y);
    chan[A].addr.mod = blitModulo(tile_bm, width);
    chan[D].addr.mod = blitModulo(dest_bm, width);

    blit(SRCA|DEST|A_TO_D, 0, chan, 0xffff, 0xffff, height * depth, (width + 15) >> 4);
}
