
#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "GUI.h"

BOOL allocBitMaps(struct BitMap *bitmaps[], ULONG modeID, UBYTE depth)
{
    WORD width = 320, height = 256;

    if (bitmaps[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE, NULL))
    {
        if (bitmaps[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE, NULL))
        {
            return(TRUE);
        }
        FreeBitMap(bitmaps[0]);
    }
    return(FALSE);
}

void freeBitMaps(struct BitMap *bitmaps[])
{
    FreeBitMap(bitmaps[1]);
    FreeBitMap(bitmaps[0]);
}

BOOL getColors(struct screenInfo *si, struct ColorMap *cm)
{
    WORD count = cm->Count;

    if (si->pal = AllocVec(((count * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC))
    {
        ULONG *pal = si->pal;

        pal[0] = count << 16;
        GetRGB32(cm, 0, count, pal + 1);
        pal[(count * 3) + 1] = 0L;

        return(TRUE);
    }
    return(FALSE);
}

void freeColors(struct screenInfo *si)
{
    FreeVec(si->pal);
}

/* Otwórz ekran podwójnie buforowany z podanâ bitmapâ */
struct Window *openWindow(struct screenInfo *si, struct BitMap *bitmaps[])
{
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    ULONG IDCMPFlags = IDCMP_RAWKEY|IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE;
    struct Rectangle rect = { 0, 0, 319, 255 }, *dclip = &rect;
    ULONG modeID = LORES_KEY;
    UBYTE depth = 5;

    if (s = OpenScreenTags(NULL,
        SA_BitMap,      bitmaps[0],
        SA_Colors32,    si->pal,
        SA_Left,        0,
        SA_Top,         0,
        SA_DClip,       dclip,
        SA_DisplayID,   modeID,
        SA_Depth,       depth,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Magazyn",
        TAG_DONE))
    {
        if (sb[0] = AllocScreenBuffer(s, bitmaps[0], 0))
        {
            if (sb[1] = AllocScreenBuffer(s, bitmaps[1], 0))
            {
                struct Window *w;

                if (w = OpenWindowTags(NULL,
                    WA_CustomScreen,    s,
                    WA_Left,            0,
                    WA_Top,             0,
                    WA_Width,           s->Width,
                    WA_Height,          s->Height,
                    WA_Backdrop,        TRUE,
                    WA_Borderless,      TRUE,
                    WA_Activate,        TRUE,
                    WA_RMBTrap,         TRUE,
                    WA_SimpleRefresh,   TRUE,
                    WA_BackFill,        LAYERS_NOBACKFILL,
                    WA_IDCMP,           IDCMPFlags,
                    WA_ReportMouse,     TRUE,
                    TAG_DONE))
                {
                    si->sb[0] = sb[0];
                    si->sb[1] = sb[1];
                    si->w     = w;
                    return(w);
                }
                FreeScreenBuffer(s, sb[1]);
            }
            FreeScreenBuffer(s, sb[0]);
        }
        CloseScreen(s);
    }
    return(NULL);
}

void closeWindow(struct Window *w, struct screenInfo *si)
{
    struct Screen *s = w->WScreen;

    CloseWindow(w);
    FreeScreenBuffer(s, si->sb[1]);
    FreeScreenBuffer(s, si->sb[0]);
    CloseScreen(s);
}
