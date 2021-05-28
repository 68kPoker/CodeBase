
#include "GUI.h"
#include "Tiles.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>

#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>

struct TextAttr ta = {
    "topaz.font", 8,
    FS_NORMAL, FPF_ROMFONT|FPF_DESIGNED
};

int initgui( struct GUI *gui, struct Rectangle *dclip, UBYTE depth, ULONG modeID, ULONG idcmp )
{
    if( gui->font = OpenDiskFont( &ta ) ) {
        if( gui->s = OpenScreenTags( NULL,
            SA_DClip,       dclip,
            SA_Depth,       depth,
            SA_DisplayID,   modeID,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Quiet,       TRUE,
            SA_ShowTitle,   FALSE,
            SA_Exclusive,   TRUE,
            SA_Font,        &ta,
            SA_Colors32,    gui->ii.colorrecord,
            TAG_DONE ) ) {
            if( gui->sb[ 0 ] = AllocScreenBuffer( gui->s, NULL, SB_SCREEN_BITMAP ) ) {
                if( gui->sb[ 1 ] = AllocScreenBuffer( gui->s, NULL, 0 ) ) {
                    if( gui->gadgets[ GID_BOARD ] = NewObject( NULL, "buttongclass",
                        GA_Left,    0,
                        GA_Top,     16,
                        GA_Width,   320,
                        GA_Height,  240,
                        GA_ID,      GID_BOARD,
                        GA_Immediate,   TRUE,
                        GA_RelVerify,   TRUE,
                        TAG_DONE ) ) {
                        if( gui->gadgets[ GID_TILE ] = NewObject( NULL, "buttongclass",
                            GA_Previous,    gui->gadgets[ GID_BOARD ],
                            GA_Left,    0,
                            GA_Top,     0,
                            GA_Width,   320,
                            GA_Height,  16,
                            GA_ID,      GID_TILE,
                            GA_Immediate,   TRUE,
                            TAG_DONE ) ) {
                            if( gui->w = OpenWindowTags( NULL,
                                WA_CustomScreen,    gui->s,
                                WA_Gadgets,         gui->gadgets[ GID_BOARD ],
                                WA_Left,            0,
                                WA_Top,             0,
                                WA_Width,           gui->s->Width,
                                WA_Height,          gui->s->Height,
                                WA_Backdrop,        TRUE,
                                WA_Borderless,      TRUE,
                                WA_Activate,        TRUE,
                                WA_RMBTrap,         TRUE,
                                WA_BackFill,        LAYERS_NOBACKFILL,
                                WA_SimpleRefresh,   TRUE,
                                WA_IDCMP,           idcmp,
                                WA_ReportMouse,     TRUE,
                                TAG_DONE ) ) {
                                return( TRUE );
                            }
                            DisposeObject( gui->gadgets[ GID_TILE ] );
                        }
                        DisposeObject( gui->gadgets[ GID_BOARD ] );
                    }
                    FreeScreenBuffer( gui->s, gui->sb[ 1 ] );
                }
                FreeScreenBuffer( gui->s, gui->sb[ 0 ] );
            }
            CloseScreen( gui->s );
        }
        CloseFont( gui->font );
    }
    return( FALSE );
}

int loadgfx( struct GUI *gui, STRPTR name )
{
    struct ILBMInfo *ii = &gui->ii;
    LONG props[] = {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        0
    }, stops[] = {
        ID_ILBM, ID_BODY,
        0
    };

    if (ii->ParseInfo.iff = AllocIFF()) {
        ii->ParseInfo.propchks = props;
        ii->ParseInfo.collectchks = NULL;
        ii->ParseInfo.stopchks = stops;
        if (loadbrush(ii, name) == 0) {
            return( TRUE );
        }
        FreeIFF(ii->ParseInfo.iff);
    }
    return(FALSE);
}

void freegfx( struct GUI *gui )
{
    unloadbrush( &gui->ii );
    FreeIFF( gui->ii.ParseInfo.iff );
}

void freegui( struct GUI *gui )
{
    CloseWindow( gui->w );
    DisposeObject( gui->gadgets[ GID_TILE ] );
    DisposeObject( gui->gadgets[ GID_BOARD ] );
    FreeScreenBuffer( gui->s, gui->sb[ 1 ] );
    FreeScreenBuffer( gui->s, gui->sb[ 0 ] );
    CloseScreen( gui->s );
}

int handlegui( struct GUI *gui )
{
    struct MsgPort *mp = gui->w->UserPort;
    struct RastPort *rp = gui->w->RPort;
    BOOL done = FALSE;
    UWORD prevx = 0, prevy = 0;
    gui->paintMode = FALSE;
    gui->tile = 0;
    gui->maxTiles = 10;

    SetAPen( rp, 1 );
    RectFill( rp, gui->tile << 4, 0, (gui->tile << 4) + 15, 15 );

    while( !done ) {
        struct IntuiMessage *msg;

        WaitPort( mp );
        while( msg = ( struct IntuiMessage * ) GetMsg( mp ) ) {
            ULONG class = msg->Class;
            WORD code = msg->Code;
            UWORD mx = msg->MouseX;
            UWORD my = msg->MouseY;
            APTR iaddr = msg->IAddress;

            ReplyMsg( ( struct Message * ) msg );

            if( class == IDCMP_GADGETDOWN ) {
                struct Gadget *gad = ( struct Gadget * ) iaddr;

                if( gad->GadgetID == GID_BOARD ) {
                    gui->paintMode = TRUE;

                    BltBitMapRastPort(gui->ii.brbitmap, gui->tile << 4, 16, rp, mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0);
                    prevx = mx >> 4;
                    prevy = my >> 4;
                }
                else if( gad->GadgetID == GID_TILE ) {
                    if( ( mx >> 4 ) < gui->maxTiles ) {
                        BltBitMapRastPort(gui->ii.brbitmap, gui->tile << 4, 16, rp, gui->tile << 4, 0, 16, 16, 0xc0);
                        gui->tile = mx >> 4;
                        SetAPen(rp, 2);
                        Move(rp, gui->tile << 4, 0);
                        Draw(rp, (gui->tile << 4) + 15, 0);
                        Draw(rp, (gui->tile << 4) + 15, 15);
                        Draw(rp, gui->tile << 4, 15);
                        Draw(rp, gui->tile << 4, 1);
                    }
                }
            }
            else if( class == IDCMP_GADGETUP ) {
                struct Gadget *gad = ( struct Gadget * ) iaddr;

                if( gad->GadgetID == GID_BOARD ) {
                    gui->paintMode = FALSE;
                }
            }
            else if( class == IDCMP_MOUSEBUTTONS ) {
                if( code == (IECODE_LBUTTON|IECODE_UP_PREFIX) ) {
                    gui->paintMode = FALSE;
                }
            }
            else if( class == IDCMP_MOUSEMOVE )  {
                mx >>= 4;
                my >>= 4;
                if( mx < B_WIDTH && my >= 1 && my < B_HEIGHT ) {
                    if( gui->paintMode && ( mx != prevx || my != prevy ) ) {
                        SetAPen( rp, 1 );
                        BltBitMapRastPort(gui->ii.brbitmap, gui->tile << 4, 16, rp, mx << 4, my << 4, 16, 16, 0xc0);
                        prevx = mx;
                        prevy = my;
                    }
                }
            }
            else if( class == IDCMP_RAWKEY ) {
                if( code == ESC_KEY )
                    done = TRUE;
            }
        }
    }
}

int main()
{
    struct GUI gui = { 0 };
    struct Rectangle dclip = { 0, 0, 319, 239 };

    if (loadgfx(&gui, "Grafika/Szablon.iff")) {
        if( initgui( &gui, &dclip, 5, VGA_MONITOR_ID|VGAPRODUCT_KEY, IDCMP ) ) {
            BltBitMapRastPort(gui.ii.brbitmap, 0, 16, gui.w->RPort, 0, 0, 320, 16, 0xc0);
            handlegui( &gui );
            freegui( &gui );
        }
        freegfx(&gui);
    }
    return( 0 );
}
