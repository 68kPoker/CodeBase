
/* $Id$ */

#include "System.h"

#include <clib/exec_protos.h>

struct Library
    *IntuitionBase,
    *IFFParseBase,
    *DiskfontBase;

VOID libsClose( VOID )
{
    CloseLibrary( DiskfontBase );
    CloseLibrary( IFFParseBase );
    CloseLibrary( IntuitionBase );
}

BOOL libsOpen( VOID )
{
    if( IntuitionBase = OpenLibrary( "intuition.library", 39L ) )
    {
        if( IFFParseBase = OpenLibrary( "iffparse.library", 39L ) )
        {
            if( DiskfontBase = OpenLibrary( "diskfont.library", 39L ) )
            {
                return( TRUE );
            }
            CloseLibrary( IFFParseBase );
        }
        CloseLibrary( IntuitionBase );
    }
    return( FALSE );
}
