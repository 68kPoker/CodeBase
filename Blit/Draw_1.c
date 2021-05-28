/*
** $Id$
**
** Draw.c - Here I place various drawing routines which uilize the Blitter.
**
** These are currently:
** - Line drawing,
** - Icon drawing,
** - Two layered icon drawing.
*/

#include <stdio.h>
#include <hardware/blit.h>
#include <hardware/custom.h>
#include <graphics/rastport.h>

#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#define abs(a) ((a)>=0?(a):-(a))
#define min(a,b) ((a)<=(b)?(a):(b))
#define max(a,b) ((a)>=(b)?(a):(b))

struct drawOp {
    WORD x1, y1, x2, y2; /* Drawing coordinates relative to layer */
};

struct drawMessage {
    struct Layer *layer;
    struct Rectangle bounds;
    LONG x, y;
};

__far extern ULONG (*hookEntry)();

__far extern struct Custom custom;

/*
** drawLineBitMap() - Draw line into specified position in standard BitMap.
*/
void drawLineBitMap( struct BitMap *bm, WORD x1, WORD y1, WORD x2, WORD y2, UBYTE mask, BOOL xor )
{
    UBYTE minterm;
    WORD dx, dy, tmp, octant;
    LONG sign;
    WORD bpr = bm->BytesPerRow;
    LONG offset = ( y1 * bpr ) + ( ( x1 >> 4 ) << 1 );
    PLANEPTR plane;
    UBYTE depth = bm->Depth, i;

    /*
    ** Normal or XOR mode.
    */
    if (xor)
        minterm = ABNC | NABC | NANBC;
    else
        minterm = ABC | ABNC | NABC | NANBC;

    /*
    ** Calc distance.
    */
    dx = x2 - x1;
    dy = y2 - y1;

    /*
    ** Normalize distance and calc octant.
    */
    if( dy >= 0 ) {
        if( dx >= 0 ) {
            if( dx >= dy )
                octant = OCTANT1;
            else
                octant = OCTANT2;
        }
        else {
            dx = -dx;
            if( dy >= dx )
                octant = OCTANT3;
            else
                octant = OCTANT4;
        }
    }
    else {
        dy = -dy;
        if( dx >= 0 ) {
            if( dx >= dy )
                octant = OCTANT5;
            else
                octant = OCTANT6;
        }
        else {
            dx = -dx;
            if( dy >= dx )
                octant = OCTANT7;
            else
                octant = OCTANT8;
        }
    }
    if( dx < dy ) {
        tmp = dx;
        dx = dy;
        dy = tmp;
    }

    OwnBlitter();

    for( i = 0; i < depth; i++ ) {
        if( !( mask & ( 1 << i ) ) )
            continue;

        plane = bm->Planes[ i ] + offset;

        WaitBlit();
        custom.bltadat = 0x8000;
        custom.bltbdat = 0xffff;
        custom.bltafwm = 0xffff;
        custom.bltalwm = 0xffff;
        custom.bltamod = 4 * ( dy - dx );
        custom.bltbmod = 4 * dy;
        custom.bltcmod = bpr;
        custom.bltdmod = bpr;
        custom.bltapt  = ( APTR ) ( sign = ( 4 * dy ) - ( 2 * dx ) );
        custom.bltcpt  = plane;
        custom.bltdpt  = plane;
        custom.bltcon0 = ( ( x1 & 15 ) << ASHIFTSHIFT ) | SRCA | SRCC | DEST | minterm;
        custom.bltcon1 = LINEMODE | octant | ( 0 << BSHIFTSHIFT ) | ( sign < 0 ? SIGNFLAG : 0 );
        custom.bltsize = ( ( dx + 1 ) << 6 ) | 2;
    }

    DisownBlitter();
}

/*
** drawLineHook() - Draw line into ClipRect.
*/

__asm void drawLineHook( register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct drawMessage *dm )
{
    struct drawOp *op = ( struct drawOp * ) hook->h_Data;
    struct Rectangle *b = &dm->bounds, is, aux;
    WORD width = b->MaxX - b->MinX + 1;
    WORD height = b->MaxY - b->MinY + 1;
    WORD sx, sy, ex, ey;
    UBYTE mask = rp->Mask;
    BOOL xor = GetDrMd(rp) & COMPLEMENT;

    is.MinX = min( op->x1, op->x2 );
    is.MinY = min( op->y1, op->y2 );
    is.MaxX = max( op->x1, op->x2 );
    is.MaxY = max( op->y1, op->y2 );

    aux.MinX = max( dm->x, is.MinX );
    aux.MinY = max( dm->y, is.MinY );
    aux.MaxX = min( dm->x + width - 1, is.MaxX );
    aux.MaxY = min( dm->y + height - 1, is.MaxY );

    if( aux.MinX > aux.MaxX || aux.MinY > aux.MaxY )
        return;

    sx = aux.MinX;
    ex = aux.MaxX;

    if( ( op->x1 < op->x2 && op->y1 > op->y2 ) ||
        ( op->x1 > op->x2 && op->y1 < op->y2 ) ) {
        sy = aux.MaxY;
        ey = aux.MinY;
    }

    printf("Blit\n");

    drawLineBitMap( rp->BitMap,
        b->MinX + sx - dm->x,
        b->MinY + sy - dm->y,
        b->MinX + ex - dm->x,
        b->MinY + ey - dm->y,
        mask, xor);
    printf("Blit done\n");
}

void drawLine( struct RastPort *rp, WORD x1, WORD y1, WORD x2, WORD y2 )
{
    struct Hook hook = { 0 };
    struct drawOp op;
    struct Rectangle bounds = { 0, 0, 319, 255 };

    op.x1 = x1;
    op.y1 = y1;
    op.x2 = x2;
    op.y2 = y2;

    hook.h_Data = ( APTR ) &op;
    hook.h_Entry = ( HOOKFUNC ) drawLineHook;

    DoHookClipRects( &hook, rp, &bounds );
}
