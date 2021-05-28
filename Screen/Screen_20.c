
/*
 * Screen.c - Screen bitmaps, Color Obtaining, Sprites.
 */

#include <intuition/screens.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Data.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

void testObtainPen(struct Screen *s)
{
    WORD i, depth = GetBitMapAttr(s->RastPort.BitMap, BMA_DEPTH);
    WORD pens[ 256 ];
    WORD colors = 1 << depth;
    struct ColorMap *cm = s->ViewPort.ColorMap;

    for (i = 0; i < colors; i++)
    {
        if ((pens[i] = ObtainPen(cm, i, 0, 0, 0, PEN_NO_SETCOLOR)) != -1)
        {
            printf("Pen %d obtained.\n", i);
        }
        else
        {
            printf("Pen %d NOT obtained.\n", i);
        }
    }

    for (i = colors; i-- > 0;)
    {
        if (pens[i] != -1)
        {
            ReleasePen(cm, pens[i]);
        }
    }
}

void testObtainBestPenA(struct Screen *s)
{
    struct BitMap *bm;

    if (bm = loadBitMap("Data/Warehouse.pic", s))
    {
        BltBitMap(bm, 0, 0, s->RastPort.BitMap, 0, 0, 160, 128, 0xc0, 0xff, NULL);
        Delay(300);
        FreeBitMap(bm);
    }
}

/*
 * First, we open a screen of given ModeID and depth, with optional
 * Custom bitmap.
 */

struct Screen *openScreen(ULONG modeID, UBYTE depth, struct BitMap *bitmap)
{
    struct Screen *s;
    struct ColorSpec colors[] =
    {
        { 0, 0, 0, 0 }, /* Black background and block pen */
        { 1, 15, 15, 15 }, /* White detail pen */
        { 17, 0, 0, 15 }, /* Blue mouse pointer */
        { 18, 0, 0, 5 },
        { 19, 10, 10, 15 },
        { -1 }
    };

    if (s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       STDSCREENWIDTH,
        SA_Height,      STDSCREENHEIGHT,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        bitmap ? SA_BitMap : TAG_IGNORE, bitmap,
        SA_Exclusive,   TRUE,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Draggable,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Colors,      colors,
        SA_SharePens,   TRUE,
        TAG_DONE))
    {
        /* Test ObtainBestPenA */
        testObtainBestPenA(s);
        return(s);
    }
    return(NULL);
}

void closeScreen(struct Screen *s)
{
    CloseScreen(s);
}

main()
{
    struct Screen *s;

    if (s = openScreen(LORES_KEY, 5, NULL))
    {
        closeScreen(s);
    }
    return(0);
}
