
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/rpattr.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "Gadgets.h"
#include "Blit.h"

#include "iffp/ilbmapp.h"

#define DEPTH 6

#define IDCMP_FLAGS IDCMP_GADGETDOWN|IDCMP_MOUSEMOVE|IDCMP_MOUSEBUTTONS|IDCMP_REFRESHWINDOW

enum
{
    WALLTILE,
    FLOORTILE,
    HEROTILE,
    BOXTILE,
    FRUITTILE,
    FLAGTILE,
    TILES
};

enum
{
    CLOSEGAD,
    DEPTHGAD,
    PALGAD,
    MENUGAD,
    BOARDGAD,
    GADGETS
};

enum
{
    CLOSEMENU,
    LOADMENU,
    SAVEMENU,
    MENUGADS
};

struct GUI
{
    struct Screen *s;
    struct Window *w;
    struct Gadget glist[ GADGETS ], *active;
    WORD curtile;
    struct ILBMInfo ii;
};

WORD table[16][20] = { 0 };

BOOL initGUI( struct GUI *gui )
{
    WORD x, y;

    for (y = 1; y < 15; y++)
    {
        for (x = 1; x < 19; x++)
        {
            table[y][x] = FLOORTILE;
        }
    }

    if( gui->s = openScreen( "Magazyn", DEPTH, ( ULONG * ) gui->ii.colorrecord ) )
    {
        struct Gadget *glist = gui->glist;
        addGadget( glist, 0,   0, 16, 16 ); /* Screen close gadget */
        addGadget( glist, 304, 0, 16, 16 ); /* Screen depth gadget */
        addGadget( glist, 80,  0, 16 * TILES, 16 ); /* Palette gadget */
        addGadget( glist, 16, 0, 64, 16 ); /* Menu gadget */
        addGadget( glist, 0, 16, 320, 240 ); /* Board gadget */

        if( gui->w = openWindow( 0, 0, 320, 256, gui->s, glist, IDCMP_FLAGS ) )
        {
            struct ScreenBuffer **sb = (struct ScreenBuffer **)gui->s->UserData;

            gui->w->RPort->BitMap = sb[1]->sb_BitMap;

            BltBitMapRastPort( gui->ii.brbitmap, 0, 0, gui->w->RPort, 0, 0, 320, 16, 0xc0 );

            bltBoardRastPort( gui->ii.brbitmap, 0, 16, gui->w->RPort, 0, 16, 320, 240 );

            while (!ChangeScreenBuffer(gui->s, sb[1]))
            {
                WaitTOF();
            }

            return( TRUE );
        }
        closeScreen( gui->s );
    }
    return( FALSE );
}

void closeGUI( struct GUI *gui )
{
    CloseWindow( gui->w );
    closeScreen( gui->s );
}

void handleGUI( struct GUI *gui )
{
    BOOL done = FALSE;
    ULONG signals[] =
    {
        1L << gui->w->UserPort->mp_SigBit
    }, total = signals[ 0 ];

    while( !done )
    {
        ULONG result = Wait( total );

        if( result & signals[ 0 ] )
        {
            struct IntuiMessage *msg;

            while( msg = ( struct IntuiMessage * ) GetMsg( gui->w->UserPort ) )
            {
                if( msg->Class == IDCMP_GADGETDOWN )
                {
                    struct Gadget *gad = (struct Gadget *)msg->IAddress;

                    if( gad->GadgetID == CLOSEGAD )
                    {
                        done = TRUE;
                    }
                    else if( gad->GadgetID == DEPTHGAD )
                    {
                        ScreenToBack( gui->s );
                    }
                    else if( gad->GadgetID == BOARDGAD )
                    {
                        BltBitMapRastPort( gui->ii.brbitmap, gui->curtile << 4, 16, gui->w->RPort, msg->MouseX & 0xfff0, msg->MouseY & 0xfff0, 16, 16, 0xc0);
                        table[msg->MouseY >> 4][msg->MouseX >> 4] = gui->curtile;
                        gui->active = gad;
                    }
                    else if( gad->GadgetID == PALGAD )
                    {
                        gui->curtile = (msg->MouseX - gad->LeftEdge) >> 4;
                    }
                    else if( gad->GadgetID == MENUGAD )
                    {
                        struct Window *menu;
                        static struct Gadget menugads[ MENUGADS ] = { 0 };

                        added = 0;

                        addGadget(menugads, 0, 0, 16, 16);

                        if (menu = openWindow( (320 - 128) / 2, 32, 128, 256 - 64, gui->s, menugads, IDCMP_GADGETDOWN ))
                        {
                            static struct Requester req;

                            InitRequester(&req);
                            Request(&req, gui->w);

                            SetAPen(gui->w->RPort, 1 << 5);
                            SetWriteMask(gui->w->RPort, 1 << 5);
                            SetRast(gui->w->RPort, 1 << 5);

                            bltTileRastPort(gui->ii.brbitmap, 192, 64, menu->RPort, 0, 0, 128, 256 - 64);
                            WaitPort(menu->UserPort);

                            SetRast(gui->w->RPort, 0 << 5);
                            SetWriteMask(gui->w->RPort, 0xff);

                            EndRequest(&req, gui->w);
                            CloseWindow(menu);
                        }
                    }
                }
                else if( msg->Class == IDCMP_MOUSEBUTTONS )
                {
                    if( msg->Code == ( IECODE_LBUTTON|IECODE_UP_PREFIX ) )
                    {
                        gui->active = NULL;
                    }
                }
                else if( msg->Class == IDCMP_MOUSEMOVE )
                {
                    struct Gadget *active = gui->active;
                    if( active )
                    {
                        if( msg->MouseX >= active->LeftEdge &&
                            msg->MouseY >= active->TopEdge  &&
                            msg->MouseX < active->LeftEdge + active->Width &&
                            msg->MouseY < active->TopEdge + active->Height)
                        {
                            BltBitMapRastPort( gui->ii.brbitmap, gui->curtile << 4, 16, gui->w->RPort, msg->MouseX & 0xfff0, msg->MouseY & 0xfff0, 16, 16, 0xc0);
                            table[msg->MouseY >> 4][msg->MouseX >> 4] = gui->curtile;
                        }
                    }
                }
                else if( msg->Class == IDCMP_REFRESHWINDOW )
                {
                    struct Rectangle rect;

                    BeginRefresh(gui->w);

                    GetRPAttrs(gui->w->RPort, RPTAG_DrawBounds, &rect, TAG_DONE);

                    bltBoardRastPort(gui->ii.brbitmap, rect.MinX, rect.MinY, gui->w->RPort, rect.MinX, rect.MinY, rect.MaxX - rect.MinX + 1, rect.MaxY - rect.MinY + 1);
                    EndRefresh(gui->w, TRUE);
                }
                ReplyMsg( ( struct Message * ) msg );
            }
        }
    }
}

int main( void )
{
    static struct GUI gui = { 0 };
    static ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        0
    }, stops[] =
    {
        ID_ILBM, ID_BODY,
        0
    };

    if( gui.ii.ParseInfo.iff = AllocIFF() )
    {
        gui.ii.ParseInfo.propchks = props;
        gui.ii.ParseInfo.stopchks = stops;
        gui.ii.ParseInfo.collectchks = NULL;
        if( loadbrush( &gui.ii, "Data/Graphics.iff" ) == 0 )
        {
            if( initGUI( &gui ) )
            {
                handleGUI( &gui );
                closeGUI( &gui );
            }
            unloadbrush( &gui.ii );
        }
        FreeIFF( gui.ii.ParseInfo.iff );
    }
    return( RETURN_OK );
}
