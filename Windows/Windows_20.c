
/* $Id: Windows.c,v 1.1 12/.0/.2 .1:.1:.2 Robert Exp Locker: Robert $ */

#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <intuition/intuition.h>
#include <graphics/rpattr.h>

#include <clib/alib_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>
#include <clib/diskfont_protos.h>

#include "Basic.h"

typedef void( *draw   )( struct window *w, struct RastPort     *rp  );
typedef void( *handle )( struct window *w, struct IntuiMessage *msg );

struct window
{
    struct Node      node; /* From bottom-most to top-most */
    struct Rectangle rect;
    draw             draw;
    handle           handle;

    struct Rectangle prev[ 2 ];

    BOOL update;
    WORD mx, my;
    BOOL drag;
};

void drawWindow( struct window *w, struct RastPort *rp )
{
    struct window *v;

    struct Region *aux, *prev;

    if (aux = NewRegion() )
    {
        OrRectRegion( aux, &w->rect );
        for( v = ( struct window * )w->node.ln_Succ; v->node.ln_Succ != NULL; v = ( struct window * )v->node.ln_Succ )
        {
            ClearRectRegion( aux, &v->rect );
        }
        prev = InstallClipRegion( rp->Layer, aux );
        w->draw( w, rp );
        InstallClipRegion( rp->Layer, prev );
        DisposeRegion( aux );
    }
}

void openWindow( struct List *list, struct window *w, struct Rectangle *rect, draw draw, handle handle, struct RastPort *rp )
{
    AddTail( list, &w->node );

    w->rect   = *rect;
    w->draw   = draw;
    w->handle = handle;
    /* draw( w, rp ); */
    w->prev[ 0 ] = w->prev[ 1 ] = *rect;
}

void refreshWindows( struct window *w, struct RastPort *rp, struct Region *reg )
{
    struct window *v;
    struct Region *aux, *prev;

    if (aux = NewRegion() )
    {
        for( v = ( struct window * )w->node.ln_Pred; v->node.ln_Pred != NULL; v = ( struct window * )v->node.ln_Pred )
        {
            ClearRegion( aux );
            OrRectRegion( aux, &v->rect );
            AndRegionRegion( reg, aux );
            ClearRectRegion( reg, &v->rect );

            prev = InstallClipRegion( rp->Layer, aux );
            v->draw( v, rp );
            InstallClipRegion( rp->Layer, prev );
        }
        DisposeRegion( aux );
    }
}

void closeWindow( struct window *w, struct RastPort *rp )
{
    struct Region *reg;

    if( reg = NewRegion() )
    {
        OrRectRegion( reg, &w->rect );
        refreshWindows( w, rp, reg );
        DisposeRegion( reg );
    }
}

void moveWindow( struct window *w, struct RastPort *rp, UWORD frame )
{
    struct Region *reg;

    if( reg = NewRegion() )
    {
        OrRectRegion( reg, &w->prev[ frame ] );
        ClearRectRegion( reg, &w->rect );
        w->draw( w, rp );
        refreshWindows( w, rp, reg );

        DisposeRegion( reg );

        w->prev[ frame ] = w->rect;
    }
}

void myDraw2( struct window *w, struct RastPort *rp )
{
    struct BitMap *gfx = (struct BitMap *)rp->RP_User;

    if (w->drag)
    {
/*        BltBitMapRastPort(rp->BitMap, w->prev[0].MinX, w->prev[0].MinY, rp, w->rect.MinX, w->rect.MinY, 112, 128, 0xc0); */

        BltBitMapRastPort(gfx, 84, 1, rp, w->rect.MinX + 16, w->rect.MinY, 80, 16, 0xc0);
    }
    else
        BltBitMapRastPort(gfx, 1, 1, rp, w->rect.MinX + 16, w->rect.MinY, 80, 16, 0xc0);
    BltBitMapRastPort(gfx, 1, 39, rp, w->rect.MinX, w->rect.MinY, 16, 16, 0xc0);
    BltBitMapRastPort(gfx, 39, 39, rp, w->rect.MinX + 96, w->rect.MinY, 16, 16, 0xc0);
    BltBitMapRastPort(gfx, 1, 115, rp, w->rect.MinX, w->rect.MinY + 16, 112, 112, 0xc0);

    SetDrMd(rp, JAM1);
/*
        SetAPen(rp, 27);

        Move(rp, w->rect.MinX + 16 + 18, w->rect.MinY + 6 + (rp->Font->tf_Baseline));
        Text(rp, "KAFELEK", 7);
*/
        SetAPen(rp, 17);
        Move(rp, w->rect.MinX + 16 + 17, w->rect.MinY + 5 + (rp->Font->tf_Baseline));
        Text(rp, "KAFELEK", 7);

/*    SetAPen(rp, 18);
    Move(rp, w->rect.MinX, w->rect.MinY + 16);
    Draw(rp, w->rect.MinX, w->rect.MaxY);
    Draw(rp, w->rect.MaxX, w->rect.MaxY);
    Draw(rp, w->rect.MaxX, w->rect.MinY + 16);*/
/*
    SetAPen(rp, 20);
    SetOutlinePen(rp, 1);
    RectFill(rp, w->rect.MinX, w->rect.MinY + 16, w->rect.MaxX, w->rect.MaxY);
    */
}

void myDraw( struct window *w, struct RastPort *rp )
{
    static UBYTE pen = 20;
    struct BitMap *gfx = (struct BitMap *)rp->RP_User;
    WORD i, j;
    STRPTR texts[] =
    {
        "MAGAZYN",
        "EDYTOR ",
        "KAFELEK",
        " OPCJE "
    };

    if (w->update)
    {
        BltBitMapRastPort(gfx, 20, 58, rp, w->mx & 0xfff0, w->my & 0xfff0, 16, 16, 0xc0);
        w->update = FALSE;
        return;
    }

    for (i = 0; i < 4; i++)
    {
        BltBitMapRastPort(gfx, 1, 1, rp, w->rect.MinX + (i * 80), w->rect.MinY, 80, 16, 0xc0);
        SetAPen(rp, 27);
        SetDrMd(rp, JAM1);
        Move(rp, w->rect.MinX + (i * 80) + 18, w->rect.MinY + 6 + (rp->Font->tf_Baseline));
        Text(rp, texts[i], 7);
        SetAPen(rp, 17);
        Move(rp, w->rect.MinX + (i * 80) + 17, w->rect.MinY + 5 + (rp->Font->tf_Baseline));
        Text(rp, texts[i], 7);
    }

    struct Rectangle clip;

    GetRPAttrs(rp, RPTAG_DrawBounds, &clip, TAG_DONE);

    for (i = clip.MinY >> 4; i <= ((clip.MaxY + 15) >> 4); i++)
    {
        for (j = clip.MinX >> 4; j <= ((clip.MaxX + 15) >> 4); j++)
        {
            if (i >= 1)
            {
                if (i == 1 || i == 15 || j == 0 || j == 19)
                {
                    BltBitMapRastPort(gfx, 20, 58, rp, j << 4, i << 4, 16, 16, 0xc0);
                }
                else
                {
                    BltBitMapRastPort(gfx, 1, 58, rp, j << 4, i << 4, 16, 16, 0xc0);
                }
            }
        }
    }
}

int main( void )
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };
    struct TextFont *tf;
    struct TextAttr ta =
    {
        "ld.font",
        8,
        FS_NORMAL,
        FPF_DISKFONT|FPF_DESIGNED
    };

    tf = OpenDiskFont(&ta);


    if( s = OpenScreenTags( NULL,
        SA_DClip,   &dclip,
        SA_Depth,   5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,   TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Interleaved, TRUE,
        SA_Font,    &ta,
        TAG_DONE ) )
    {
        struct Window *w;

        if( w = OpenWindowTags( NULL,
            WA_CustomScreen,    s,
            WA_Left,    0,
            WA_Top,     0,
            WA_Width,   s->Width,
            WA_Height,  s->Height,
            WA_Backdrop,    TRUE,
            WA_Borderless,  TRUE,
            WA_Activate,    TRUE,
            WA_RMBTrap,     TRUE,
            WA_IDCMP,       IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
            WA_ReportMouse, TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE ) )
        {
            struct BitMap *gfx;
            struct ColorMap *cm = s->ViewPort.ColorMap;

            if (gfx = loadILBM("Dane/Magazyn.iff", cm))
            {
                BOOL done = FALSE, drag = FALSE, paint = FALSE;
                WORD lastx = 0, lasty = 0;
                MakeScreen(s);
                RethinkDisplay();

                w->RPort->RP_User = (APTR)gfx;

                struct List list;
                struct Rectangle rect = { 0, 0, 319, 255 };
                struct RastPort *rp = w->RPort;
                struct window win[ 2 ] = { 0 };

                NewList( &list );

                openWindow( &list, &win[ 0 ], &rect, myDraw, NULL, rp );
                rect.MinX = 32;
                rect.MinY = 32;
                rect.MaxX = 32 + 112 - 1;
                rect.MaxY = 32 + 128 - 1;
                openWindow( &list, &win[ 1 ], &rect, myDraw2, NULL, rp );
                drawWindow( &win[ 0 ], rp );
                drawWindow( &win[ 1 ], rp );


                while (!done)
                {
                    struct IntuiMessage *msg;
                    /* WaitPort(w->UserPort); */
                    WaitTOF();

                    moveWindow( &win[ 1 ], rp, 0 );

                    while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
                    {
                        ULONG class = msg->Class;
                        UWORD code = msg->Code;
                        WORD mx = msg->MouseX, my = msg->MouseY;

                        ReplyMsg((struct Message *)msg);

                        if (class == IDCMP_MOUSEBUTTONS)
                        {
                            if (code == IECODE_LBUTTON)
                            {
                                if (mx >= win[ 1 ].rect.MinX && mx <= win[ 1 ].rect.MaxX && my >= win[ 1 ].rect.MinY && my <= win[1].rect.MaxY)
                                {
                                    drag = TRUE;
                                    win[1].drag = TRUE;
                                    lastx = mx - win[1].rect.MinX;
                                    lasty = my - win[1].rect.MinY;
                                    moveWindow( &win[ 1 ], rp, 0 );
                                }
                                else
                                {
                                    paint = TRUE;
                                    win[ 0 ].update = TRUE;
                                    win[ 0 ].mx = mx;
                                    win[ 0 ].my = my;
                                    drawWindow(&win[ 0 ], rp);
                                }
                            }
                            else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                            {
                                if (drag)
                                {
                                    win[ 1 ].rect.MinX = msg->MouseX - lastx;
                                    win[ 1 ].rect.MinY = msg->MouseY - lasty;
                                    win[ 1 ].rect.MaxX = msg->MouseX - lastx + 111;
                                    win[ 1 ].rect.MaxY = msg->MouseY - lasty + 127;


                                    drag = FALSE;
                                    win[1].drag = FALSE;
                                    moveWindow( &win[ 1 ], rp, 0 );
                                }
                                else if (paint)
                                {
                                    paint = FALSE;
                                }
                            }
                            else if (code == IECODE_RBUTTON)
                            {
                                done = TRUE;
                            }
                        }
                        else if (class == IDCMP_MOUSEMOVE)
                        {
                            if (drag)
                            {
                                win[ 1 ].rect.MinX = msg->MouseX - lastx;
                                win[ 1 ].rect.MinY = msg->MouseY - lasty;
                                win[ 1 ].rect.MaxX = msg->MouseX - lastx + 111;
                                win[ 1 ].rect.MaxY = msg->MouseY - lasty + 127;

                            }
                            else if (paint)
                            {
                                win[ 0 ].update = TRUE;
                                win[ 0 ].mx = mx;
                                win[ 0 ].my = my;
                                drawWindow(&win[ 0 ], rp);
                            }
                        }

                    }
                }
                closeWindow( &win[ 1 ], rp );

                WaitPort( w->UserPort );
                FreeBitMap(gfx);
            }
            CloseWindow( w );
        }
        CloseScreen( s );
    }

    CloseFont(tf);
    return( 0 );
}
