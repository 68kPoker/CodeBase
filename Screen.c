
/* $Id$ */

/* Screen functions. */

#include "Screen.h"

#include <intuition/screens.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <graphics/gfxmacros.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

__far extern struct Custom custom;
extern void myCopper(void);

/*
 * getScreenTags: Obtain standard TagItem list for the openScreen.
 */

struct TagItem *getScreenTags(void)
{
    static struct TagItem tags[] =
    {
        { SA_DClip,     0           },
        { SA_DisplayID, LORES_KEY   },
        { SA_Depth,     DEPTH       },
        { SA_Quiet,     TRUE        },
        { SA_Exclusive, TRUE        },
        { SA_ShowTitle, FALSE       },
        { SA_BackFill,  (ULONG) LAYERS_NOBACKFILL },
        { SA_Title,     0           },
        { TAG_DONE }
    };
    static struct Rectangle rect =
        { 0, 0, 319, 255 };

    tags[0].ti_Data = (ULONG) &rect;
    tags[7].ti_Data = (ULONG) "Magazyn";

    return(tags);
}

/*
 * openScreen: Open double-buffered screen.
 */

struct screenInfo *openScreen(struct TagItem *base, ULONG tag1, ...)
{
    struct screenInfo *si;

    if (si = AllocMem(sizeof(*si), MEMF_PUBLIC|MEMF_CLEAR))
    {
        ApplyTagChanges(base, (struct TagItem *) &tag1);

        if (si->screen = OpenScreenTagList(NULL, base))
        {
            if (si->buffers[0] = AllocScreenBuffer(si->screen, NULL, SB_SCREEN_BITMAP))
            {
                if (si->buffers[1] = AllocScreenBuffer(si->screen, NULL, SB_COPY_BITMAP))
                {
                    if (si->safePort = CreateMsgPort())
                    {
                        si->safeToDraw = TRUE;
                        si->frame = 1;
                        si->buffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safePort;
                        si->buffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safePort;

                        if ((si->copInfo.dispSignal = AllocSignal(-1)) != -1)
                        {
                            struct UCopList *ucl;

                            si->copInfo.sigTask = FindTask(NULL);
                            si->copInfo.viewPort = &si->screen->ViewPort;

                            si->dispInt.is_Code = myCopper;
                            si->dispInt.is_Data = (APTR)&si->copInfo;
                            si->dispInt.is_Node.ln_Pri = 0;

                            if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                            {
                                CINIT(ucl, 3); /* Copper-list length */
                                CWAIT(ucl, 0, 0);
                                CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                                CEND(ucl);

                                Forbid();
                                si->screen->ViewPort.UCopIns = ucl;
                                Permit();

                                RethinkDisplay();

                                AddIntServer(INTB_COPER, &si->dispInt);

                                si->screen->UserData = (APTR)si;

                                return(si);
                            }
                            FreeSignal(si->copInfo.dispSignal);
                        }
                        DeleteMsgPort(si->safePort);
                    }
                    FreeScreenBuffer(si->screen, si->buffers[1]);
                }
                FreeScreenBuffer(si->screen, si->buffers[0]);
            }
            CloseScreen(si->screen);
        }
        FreeMem(si, sizeof(*si));
    }
    return(NULL);
}

/*
 * closeScreen: Close double-buffered screen.
 */

void closeScreen(struct screenInfo *si)
{
    RemIntServer(INTB_COPER, &si->dispInt);
    FreeSignal(si->copInfo.dispSignal);

    if (!(si->safeToDraw))
    {
        while (!(GetMsg(si->safePort)))
        {
            WaitPort(si->safePort);
        }
    }
    DeleteMsgPort(si->safePort);
    FreeScreenBuffer(si->screen, si->buffers[1]);
    FreeScreenBuffer(si->screen, si->buffers[0]);
    CloseScreen(si->screen);
    FreeMem(si, sizeof(*si));
}

/** EOF **/
