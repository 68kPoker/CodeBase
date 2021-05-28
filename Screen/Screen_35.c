
#include <intuition/screens.h>
#include <exec/memory.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"
#include "System.h"
#include "Screen_protos.h"

SCR *openScreen(SYSDATA *sysdata)
{
    struct BitMap *bitmaps[2];
    struct Rectangle dclip = { 0, 0, 319, 255 };
    WORD width = dclip.MaxX - dclip.MinX + 1;
    WORD height = dclip.MaxY - dclip.MinY + 1;
    UBYTE depth = 5;
    ULONG modeID = LORES_KEY;

    if (bitmaps[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        if (bitmaps[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
        {
            SCR *s;
            if (s = OpenScreenTags(NULL,
                SA_BitMap,      bitmaps[0],
                SA_DClip,       &dclip,
                SA_DisplayID,   modeID,
                SA_Colors32,    sysdata->gfx->colorsRGB32,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_ShowTitle,   FALSE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                TAG_DONE))
            {
                struct ScreenBuffer *sb[2];
                if (sb[0] = AllocScreenBuffer(s, bitmaps[0], 0))
                {
                    if (sb[1] = AllocScreenBuffer(s, bitmaps[1], 0))
                    {
                        struct MsgPort *mp;
                        if (mp = CreateMsgPort())
                        {
                            SCRINFO *si;
                            if (si = AllocMem(sizeof(*si), MEMF_PUBLIC))
                            {
                                si->screen = s;
                                si->bitmaps[0] = bitmaps[0];
                                si->bitmaps[1] = bitmaps[1];
                                si->buffers[0] = sb[0];
                                si->buffers[1] = sb[1];
                                si->safeport = mp;
                                si->safe = TRUE;
                                si->frame = 1;
                                s->UserData = (APTR)si;
                                return(s);
                            }
                            DeleteMsgPort(mp);
                        }
                        FreeScreenBuffer(s, sb[1]);
                    }
                    FreeScreenBuffer(s, sb[0]);
                }
                CloseScreen(s);
            }
            FreeBitMap(bitmaps[1]);
        }
        FreeBitMap(bitmaps[0]);
    }
    return(NULL);
}

void closeScreen(SCR *s)
{
    SCRINFO *si = (SCRINFO *)s->UserData;

    if (!si->safe)
    {
        while (!GetMsg(si->safeport))
        {
            WaitPort(si->safeport);
        }
    }
    DeleteMsgPort(si->safeport);
    FreeScreenBuffer(s, si->buffers[1]);
    FreeScreenBuffer(s, si->buffers[0]);
    CloseScreen(s);
    FreeBitMap(si->bitmaps[1]);
    FreeBitMap(si->bitmaps[0]);
    FreeMem(si, sizeof(*si));
}
