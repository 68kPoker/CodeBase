
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/exec_protos.h>

#include "CGUI.h"

extern void myCopper( void );
extern __far struct Custom custom;

struct TextAttr ta =
{
    "ld.font", 8,
    FS_NORMAL,
    FPF_DISKFONT | FPF_DESIGNED
};

BOOL openFont( CScreen *s )
{
    if( s->font = OpenDiskFont( &ta ) )
        return( TRUE );

    return( FALSE );
}

void closeFont( CScreen *s )
{
    CloseFont( s->font );
}

BOOL openScreen( CInputs inputs, CScreen *s, struct TagItem *more, CInputHandler safeHandler, CInputHandler copHandler )
{
    struct Rectangle dclip = { 0, 0, 319, 255 };
    UBYTE depth = 5;
    ULONG modeID = LORES_KEY;
    UWORD pens[] = { ~0 };
    BYTE coppri = 0;
    WORD coplen = 3; /* Copper-list length */
    static UBYTE title[] = "Magazyn";

    if( s->s = OpenScreenTags( NULL,
        SA_DClip,       &dclip,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Draggable,   FALSE,
        SA_Title,       title,
        SA_Pens,        pens,
        SA_Font,        &ta,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_MORE,       more ) ) {

        if( s->sb[ 0 ] = AllocScreenBuffer( s->s, NULL, SB_SCREEN_BITMAP ) ) {
            if( s->sb[ 1 ] = AllocScreenBuffer( s->s, NULL, 0 ) ) {
                s->frame = 1;

                if( s->safe.mp = CreateMsgPort() ) {
                    s->safe.safe = TRUE;
                    s->sb[ 0 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = s->safe.mp;
                    s->sb[ 1 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = s->safe.mp;

                    s->cop.vp = &s->s->ViewPort;
                    s->cop.task = FindTask( NULL );

                    if( ( s->cop.signal = AllocSignal( -1 ) ) != -1 ) {
                        struct Interrupt *is = &s->cop.is;
                        struct UCopList *ucl;

                        is->is_Code = myCopper;
                        is->is_Data = ( APTR ) &s->cop;
                        is->is_Node.ln_Pri = coppri;
                        is->is_Node.ln_Name = title;

                        if( ucl = AllocMem( sizeof( *ucl ), MEMF_PUBLIC | MEMF_CLEAR ) ) {
                            CINIT( ucl, coplen );
                            CWAIT( ucl, 0, 0 );
                            CMOVE( ucl, custom.intreq, INTF_SETCLR | INTF_COPER );
                            CEND( ucl );

                            Forbid();
                            s->s->ViewPort.UCopIns = ucl;
                            Permit();

                            RethinkDisplay();

                            AddIntServer( INTB_COPER, is );

                            initCopper( inputs + IN_COPPER, &s->cop, copHandler );
                            initSafe( inputs + IN_SAFE, &s->safe, safeHandler );

                            s->cop.s = s;
                            s->safe.s = s;

                            return( TRUE );
                        }
                        FreeSignal( s->cop.signal );
                    }
                    DeleteMsgPort( s->safe.mp );
                }
                FreeScreenBuffer( s->s, s->sb[ 1 ] );
            }
            FreeScreenBuffer( s->s, s->sb[ 0 ] );
        }
        CloseScreen( s->s );
    }
    return( FALSE );
}

void closeScreen( CScreen *s )
{
    RemIntServer( INTB_COPER, &s->cop.is );
    FreeSignal( s->cop.signal );

    if( !s->safe.safe )
        while( !GetMsg( s->safe.mp ) )
            WaitPort( s->safe.mp );

    DeleteMsgPort( s->safe.mp );
    FreeScreenBuffer( s->s, s->sb[ 1 ] );
    FreeScreenBuffer( s->s, s->sb[ 0 ] );
    CloseScreen( s->s );
}

BOOL openBackWindow( CInputs inputs, CScreen *s, CInputHandler handler )
{
    if( s->backw = OpenWindowTags( NULL,
        WA_CustomScreen,    s->s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->s->Width,
        WA_Height,          s->s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE,
        WA_ReportMouse,     TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        TAG_DONE ) ) {

        initIDCMP( inputs + IN_IDCMP, &s->idcmp, s->backw->UserPort, handler );

        return( TRUE );
    }
    return( FALSE );
}

void closeBackWindow( CScreen *s )
{
    CloseWindow( s->backw );
}
