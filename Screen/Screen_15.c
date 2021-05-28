
/* Gearwork Screen manipulation */

#include "GWScreen.h"
#include "GWLibs.h"

#include <intuition/screens.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

static struct GWscreen screen = { 0 };
static UWORD pens[] = { ~0 };

struct GWscreen *GWgetScreen(BOOL view) /* Get Screen or View */
{
    struct GWscreen *s = &screen;
    struct GWintui *intui = &s->disp.intui;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (view)
    {
        GWprint("Warning: View not implemented.\n");
        return(NULL);
    }

    if (s->bitmaps[0] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (s->bitmaps[1] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            if (intui->screen = OpenScreenTags(NULL,
                SA_DClip,       &dclip,
                SA_BitMap,      s->bitmaps[0],
                SA_DisplayID,   LORES_KEY,
                SA_ShowTitle,   FALSE,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                SA_Pens,        pens,
                TAG_DONE))
            {
                if (intui->buffers[0] = AllocScreenBuffer(intui->screen, s->bitmaps[0], 0))
                {
                    if (intui->buffers[1] = AllocScreenBuffer(intui->screen, s->bitmaps[1], 0))
                    {
                        if (intui->ports[0] = CreateMsgPort())
                        {
                            if (intui->ports[1] = CreateMsgPort())
                            {
                                intui->safe[0] = intui->safe[1] = TRUE;
                                s->frame = 1;
                                intui->buffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = intui->ports[0];
                                intui->buffers[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = intui->ports[1];
                                intui->buffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = intui->ports[0];
                                intui->buffers[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = intui->ports[1];
                                atexit(GWfreeScreen);
                                return(s);
                            }
                            else
                                GWerror("Couldn't create message port!\n");
                            DeleteMsgPort(intui->ports[1]);
                        }
                        else
                            GWerror("Couldn't create message port!\n");
                        FreeScreenBuffer(intui->screen, intui->buffers[1]);
                    }
                    else
                        GWerror("Couldn't alloc screen buffer!\n");
                    FreeScreenBuffer(intui->screen, intui->buffers[0]);
                }
                else
                    GWerror("Couldn't alloc screen buffer!\n");
                CloseScreen(intui->screen);
            }
            else
                GWerror("Couldn't open screen!\n");
            FreeBitMap(s->bitmaps[1]);
        }
        else
            GWerror("Out of graphics memory!\n");
        FreeBitMap(s->bitmaps[0]);
    }
    else
        GWerror("Out of graphics memory!\n");
    return(NULL);
}

VOID GWfreeScreen(VOID)
{
    struct GWscreen *s = &screen;
    struct GWintui *intui = &s->disp.intui;

    if (!intui->safe[1])
    {
        while (!GetMsg(intui->ports[1]))
        {
            WaitPort(intui->ports[1]);
        }
    }
    if (!intui->safe[0])
    {
        while (!GetMsg(intui->ports[0]))
        {
            WaitPort(intui->ports[0]);
        }
    }
    DeleteMsgPort(intui->ports[1]);
    DeleteMsgPort(intui->ports[0]);
    FreeScreenBuffer(intui->screen, intui->buffers[1]);
    FreeScreenBuffer(intui->screen, intui->buffers[0]);
    CloseScreen(intui->screen);
    FreeBitMap(s->bitmaps[1]);
    FreeBitMap(s->bitmaps[0]);
}
