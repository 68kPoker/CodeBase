
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id: Screen.c,v 1.1 12/.0/.0 .2:.1:.0 Robert Exp Locker: Robert $
*/

#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"

struct Screen* openScreen()
{
    struct BitMap* bm[2];

    if (bm[0] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE, NULL))
    {
        if (bm[1] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE, NULL))
        {
            struct Screen* s;
            ULONG* colors;

            if (colors = allocColors())
            {
                if (s = OpenScreenTags(NULL,
                    SA_BitMap,      bm[0],
                    SA_Left,        0,
                    SA_Top,         0,
                    SA_Width,       320,
                    SA_Height,      256,
                    SA_Depth,       DEPTH,
                    SA_DisplayID,   PAL_MONITOR_ID|LORES_KEY,
                    SA_Quiet,       TRUE,
                    SA_Exclusive,   TRUE,
                    SA_ShowTitle,   FALSE,
                    SA_Draggable,   FALSE,
                    SA_BackFill,    LAYERS_NOBACKFILL,
                    SA_Colors32,    colors,
                    TAG_DONE))
                {
                    struct Window* w;

                    if (w = OpenWindowTags(NULL,
                        WA_CustomScreen,    s,
                        WA_Left,            0,
                        WA_Top,             0,
                        WA_Width,           320,
                        WA_Height,          256,
                        WA_Backdrop,        TRUE,
                        WA_Borderless,      TRUE,
                        WA_Activate,        TRUE,
                        WA_RMBTrap,         TRUE,
                        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
                        WA_SimpleRefresh,   TRUE,
                        WA_BackFill,        LAYERS_NOBACKFILL,
                        TAG_DONE))
                    {
                        struct DBufInfo* dbi;

                        if (dbi = AllocDBufInfo(&s->ViewPort))
                        {
                            struct MsgPort* mp[2];

                            if (mp[0] = CreateMsgPort())
                            {
                                if (mp[1] = CreateMsgPort())
                                {
                                    struct screenInfo* si;

                                    dbi->dbi_SafeMessage.mn_ReplyPort = mp[0];
                                    dbi->dbi_DispMessage.mn_ReplyPort = mp[1];

                                    if (si = AllocMem(sizeof(*si), MEMF_PUBLIC))
                                    {
                                        si->s      = s;
                                        si->bm[0]  = bm[0];
                                        si->bm[1]  = bm[1];
                                        si->dbi    = dbi;
                                        si->mp[0]  = mp[0];
                                        si->mp[1]  = mp[1];
                                        si->w      = w;
                                        si->colors = colors;

                                        s->UserData = (APTR)si;

                                        si->safe[0] = si->safe[1] = TRUE;
                                        return(s);
                                    }
                                    DeleteMsgPort(mp[1]);
                                }
                                DeleteMsgPort(mp[0]);
                            }
                            FreeDBufInfo(dbi);
                        }
                        CloseWindow(w);
                    }
                    CloseScreen(s);
                }
                FreeMem(colors, ((COLORS * 3) + 2) * sizeof(ULONG));
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    return(NULL);
}

ULONG* allocColors()
{
    ULONG* colors;

    if (colors = AllocMem(((COLORS * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC|MEMF_CLEAR))
    {
        colors[0] = COLORS << 16;
        return(colors);
    }
    return(NULL);
}

void closeScreen(struct Screen* s)
{
    struct screenInfo* si = (struct screenInfo* )s->UserData;

    if (!si->safe[1])
        while (!GetMsg(si->mp[1]))
            WaitPort(si->mp[1]);

    if (!si->safe[0])
        while (!GetMsg(si->mp[0]))
            WaitPort(si->mp[0]);

    DeleteMsgPort(si->mp[1]);
    DeleteMsgPort(si->mp[0]);

    FreeDBufInfo(si->dbi);
    CloseWindow(si->w);
    CloseScreen(si->s);
    FreeBitMap(si->bm[1]);
    FreeBitMap(si->bm[0]);
    FreeMem(si->colors, ((COLORS * 3) + 2) * sizeof(ULONG));
    FreeMem(si, sizeof(*si));
}

/* EOF */
