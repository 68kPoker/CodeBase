
/* Gearwork Libraries */

#include "GWlibs.h"

#include <stdlib.h>
#include <stdio.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>

struct Library
    *IntuitionBase = NULL,
    *GadToolsBase  = NULL,
    *GfxBase       = NULL,
    *IFFParseBase  = NULL;

VOID GWcleanup(VOID)
{
    if (IFFParseBase)   CloseLibrary(IFFParseBase);
    if (GfxBase)        CloseLibrary(GfxBase);
    if (GadToolsBase)   CloseLibrary(GadToolsBase);
    if (IntuitionBase)  CloseLibrary(IntuitionBase);
}

VOID GWerror(STRPTR errorDesc)
{
    printf("ERROR: %s\n", errorDesc);
}

VOID GWbailout(STRPTR errorDesc)
{
    GWerror(errorDesc);
    exit(RETURN_ERROR);
}

BOOL GWopenLibs(ULONG minVersion) /* Open all required libraries */
{
    GWprint("Gear work game engine starting...\n");
    GWprint("Required Amiga OS version is: %ld.\n", minVersion);

    atexit(GWcleanup);

    if (!(IntuitionBase = OpenLibrary("intuition.library", minVersion)))
        GWbailout("Unable to open intuition.library!\n");

    if (!(GadToolsBase = OpenLibrary("gadtools.library", minVersion)))
        GWbailout("Unable to open gadtools.library!\n");

    if (!(GfxBase = OpenLibrary("graphics.library", minVersion)))
        GWbailout("Unable to open graphics.library!\n");

    if (!(IFFParseBase = OpenLibrary("iffparse.library", minVersion)))
        GWbailout("Unable to open iffparse.library!\n");

    return(TRUE);
}
