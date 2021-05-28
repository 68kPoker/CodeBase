
#include "Screen.h"

#include <intuition/intuition.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

/* Alloc bitmaps before showing them */
LONG allocScreenBitMaps(struct ScreenInfo *si, UWORD width, UWORD height, UBYTE depth)
{
    if (si->bitmaps[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
        {
        if (si->bitmaps[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
            {
            return(0);
            }
        FreeBitMap(si->bitmaps[0]);
        }
    return(-1);
}

/* Free bitmaps */
VOID freeScreenBitMaps(struct ScreenInfo *si)
{
    FreeBitMap(si->bitmaps[1]);
    FreeBitMap(si->bitmaps[0]);
}

LONG openScreen(struct ScreenInfo *si, struct PictureInfo *pi)
{
    if (si->screen = OpenScreenTags(NULL,
        SA_Width,   320,
        SA_Height,  256,
        SA_Depth,   5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,   TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_BitMap,  si->bitmaps[0],
        SA_Colors32,    pi->colors,
        TAG_DONE))
        {
        return(0);
        }
    return(-1);
}

VOID closeScreen(struct ScreenInfo *si)
{
    CloseScreen(si->screen);
}
