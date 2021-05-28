
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "MyClasses.h"
#include "IFF.h"
#include "GUI.h"

ULONG *getPalette(struct ColorMap *cm)
{
    ULONG *pal;
    WORD count = cm->Count;
    LONG size = ((count * 3) + 2) * sizeof(ULONG);

    if (pal = AllocVec(size, MEMF_PUBLIC))
    {
        pal[0] = count << 16;
        GetRGB32(cm, 0, count, pal + 1);
        pal[(count * 3) + 1] = 0L;
    }
    return(pal);
}

struct Window *openScreen(struct screenInfo *si, ULONG *pal, void (*draw)(struct BitMap *bm), struct boardInfo *bi)
{
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };
    WORD width = dclip.MaxX - dclip.MinX + 1;
    WORD height = dclip.MaxY - dclip.MinY + 1;
    ULONG modeID = LORES_KEY;
    UBYTE depth = 5;

    if (si->bm[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
    {
        draw(si->bm[0]);
        if (si->bm[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
        {
            struct Screen *s;
            if (s = OpenScreenTags(NULL,
                SA_DClip,       &dclip,
                SA_Depth,       depth,
                SA_DisplayID,   modeID,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_ShowTitle,   FALSE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                SA_Title,       "Magazyn",
                SA_Colors32,    pal,
                SA_BitMap,      si->bm[0],
                TAG_DONE))
            {
                s->UserData = (APTR)si;
                if (si->sb[0] = AllocScreenBuffer(s, si->bm[0], 0))
                {
                    if (si->sb[1] = AllocScreenBuffer(s, si->bm[1], 0))
                    {
                        if (si->mp = CreateMsgPort())
                        {
                            si->safe = TRUE;
                            si->frame = 1;
                            if (si->w = OpenWindowTags(NULL,
                                WA_CustomScreen,    s,
                                WA_Left,            0,
                                WA_Top,             0,
                                WA_Width,           width,
                                WA_Height,          height,
                                WA_Backdrop,        TRUE,
                                WA_Borderless,      TRUE,
                                WA_RMBTrap,         TRUE,
                                WA_Activate,        TRUE,
                                WA_IDCMP,           IDCMP_MOUSEBUTTONS|IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEMOVE|IDCMP_RAWKEY,
                                WA_ReportMouse,     TRUE,
                                WA_SimpleRefresh,   TRUE,
                                WA_BackFill,        LAYERS_NOBACKFILL,
                                TAG_DONE))
                            {
                                si->w->UserData = (APTR)bi;
                                return(si->w);
                            }
                            DeleteMsgPort(si->mp);
                        }
                        FreeScreenBuffer(s, si->sb[1]);
                    }
                    FreeScreenBuffer(s, si->sb[0]);
                }
                CloseScreen(s);
            }
            FreeBitMap(si->bm[1]);
        }
        FreeBitMap(si->bm[0]);
    }
    return(NULL);
}

void safeToDraw(struct Window *w)
{
    struct Screen *s = w->WScreen;
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    if (!si->safe)
    {
        while (!GetMsg(si->mp))
        {
            WaitPort(si->mp);
        }
        si->safe = TRUE;
    }
}

void changeBuffer(struct Window *w)
{
    struct Screen *s = w->WScreen;
    struct screenInfo *si = (struct screenInfo *)s->UserData;
    WORD frame = si->frame;

    while (!ChangeScreenBuffer(s, si->sb[frame]))
    {
        WaitTOF();
    }
    si->frame = frame ^= 1;
    w->RPort->BitMap = si->bm[frame];

    si->safe = FALSE;
}

void closeScreen(struct Window *w)
{
    struct Screen *s = w->WScreen;
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    CloseWindow(w);
    if (!si->safe)
    {
        while (!GetMsg(si->mp))
        {
            WaitPort(si->mp);
        }
    }
    DeleteMsgPort(si->mp);
    FreeScreenBuffer(s, si->sb[1]);
    FreeScreenBuffer(s, si->sb[0]);
    CloseScreen(s);
    FreeBitMap(si->bm[1]);
    FreeBitMap(si->bm[0]);
}
