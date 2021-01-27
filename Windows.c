
/* $Id: Windows.c,v 1.1 12/.0/.2 .1:.1:.2 Robert Exp Locker: Robert $ */

#include <graphics/gfx.h>
#include <exec/nodes.h>
#include <intuition/intuition.h>

#include <clib/alib_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

typedef void( *draw   )( struct window *w, struct RastPort     *rp  );
typedef void( *handle )( struct window *w, struct IntuiMessage *msg );

struct window
{
    struct Node      node; /* From bottom-most to top-most */
    struct Rectangle rect;
    draw             draw;
    handle           handle;

    struct Rectangle prev[ 2 ];
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
    draw( w, rp );
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
        refreshWindows( w, rp, reg );
        w->draw( w, rp );
        DisposeRegion( reg );
    }
}

void myDraw( struct window *w, struct RastPort *rp )
{
    static UBYTE pen = 0;
    SetAPen( rp, pen++ );
    SetOutlinePen( rp, 1 );
    RectFill( rp, w->rect.MinX, w->rect.MinY, w->rect.MaxX, w->rect.MaxY );
}

int main( void )
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if( s = OpenScreenTags( NULL,
        SA_DClip,   &dclip,
        SA_Depth,   5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,   TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
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
            WA_IDCMP,       IDCMP_MOUSEBUTTONS,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE ) )
        {
            struct List list;
            struct Rectangle rect = { 0, 0, 319, 255 };
            struct RastPort *rp = w->RPort;
            struct window win[ 2 ];

            NewList( &list );

            openWindow( &list, &win[ 0 ], &rect, myDraw, NULL, rp );
            rect.MinX = 32;
            rect.MinY = 32;
            rect.MaxX = 32 + 64 - 1;
            rect.MaxY = 32 + 64 - 1;
            openWindow( &list, &win[ 1 ], &rect, myDraw, NULL, rp );

            Delay( 200 );

            win[ 1 ].rect.MinX += 32;
            win[ 1 ].rect.MinY += 32;
            win[ 1 ].rect.MaxX += 32;
            win[ 1 ].rect.MaxY += 32;

            moveWindow( &win[ 1 ], rp, 0 );

            Delay( 200 );

            closeWindow( &win[ 1 ], rp );

            WaitPort( w->UserPort );
            CloseWindow( w );
        }
        CloseScreen( s );
    }
    return( 0 );
}
