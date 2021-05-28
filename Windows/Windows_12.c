
#include "windows.h"
#include "screen.h"

#include <stdio.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

BOOL initGadget(struct windowInfo *wi, struct gadgetInfo *gi, WORD kind)
{
    struct Gadget *gad = &gi->gad;

    gad->NextGadget = NULL;
    gad->LeftEdge = (kind == GID_SDEPTH ? 19 << 4 : kind << 4);
    gad->TopEdge = 0;
    gad->Width = (kind == GID_SDRAG ? 304 - (kind << 4) : 16);
    gad->Height = 16;
    gad->Flags = GFLG_GADGHNONE;
    gad->Activation = GACT_IMMEDIATE;

    gad->GadgetType = GTYP_BOOLGADGET;

    if (kind == GID_SDEPTH)
        gad->GadgetType |= GTYP_SDEPTH;
    else if (kind == GID_SDRAG)
        gad->GadgetType |= GTYP_SDRAGGING;

    gad->GadgetID = kind;
    gad->UserData = (APTR)gi;

    return(TRUE);
}

/* Add gadgets to window */
VOID linkGadgets(struct windowInfo *wi)
{
    struct gadgetInfo *gi;
    WORD gid;

    for (gid = 0; gid < wi->count - 1; gid++)
        {
        gi = wi->gads + gid;
        gi->gad.NextGadget = gi + 1;
        }

    AddGList(wi->w, &wi->gads->gad, -1, -1, NULL);
}

BOOL openWindow(struct windowInfo *wi, struct Screen *s, WORD kind)
{
    if (wi->w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Width,
        WA_Height,          s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETDOWN,
        WA_ReportMouse,     TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        TAG_DONE))
        {
        SetFont(wi->w->RPort, ((struct screenInfo *)s->UserData)->font);
        if (wi->gads = AllocMem(GID_COUNT * sizeof(struct gadgetInfo), MEMF_PUBLIC|MEMF_CLEAR))
            {
            WORD gid;

            wi->count = GID_COUNT;
            wi->w->UserData = (APTR)wi;

            for (gid = 0; gid < GID_COUNT; gid++)
                {
                initGadget(wi, wi->gads + gid, gid);
                }

            linkGadgets(wi);
            return(TRUE);
            }
        CloseWindow(wi->w);
        }
    return(FALSE);
}

VOID closeWindow(struct windowInfo *wi)
{
    RemoveGList(wi->w, wi->gads, -1);
    FreeMem(wi->gads, wi->count * sizeof(struct gadgetInfo));
    CloseWindow(wi->w);
}

VOID mouseButtons(struct windowInfo *wi, WORD code, WORD mx, WORD my)
{
    struct RastPort *rp = wi->w->RPort;

    if (code == IECODE_LBUTTON)
        {
        if (my >= 16)
            {
            /* Place tile */
            BltBitMapRastPort(wi->gfx, wi->tile << 4, 16, rp, mx & 0xfff0, my & 0xfff0, 16, 16, 0xc0);

            if (wi->trigger)
                {
                displayNumber(rp, (mx & 0xfff0) + 8, (my & 0xfff0) + 4 + rp->Font->tf_Baseline, wi->trigger);
                }

            wi->paint = TRUE;

            }
        }
    else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
        {
        wi->paint = FALSE;
        }
}

VOID mouseMove(struct windowInfo *wi, WORD code, WORD mx, WORD my)
{
    static WORD prevx = 0, prevy = 0;

    if ((mx >> 4) == prevx && ((my >> 4) == prevy))
        {
        return;
        }

    prevx = mx >> 4;
    prevy = my >> 4;

    if (wi->paint)
        {
        mouseButtons(wi, IECODE_LBUTTON, mx, my);
        }
}

LONG handleWindow(struct windowInfo *wi)
{
    struct RastPort *rp = wi->w->RPort;
    struct MsgPort *mp = wi->w->UserPort;
    BOOL done = FALSE;

    wi->tile = 3;

    while (!done)
        {
        struct IntuiMessage *msg;
        WaitPort(mp);
        while (msg = (struct IntuiMessage *)GetMsg(mp))
            {
            if (msg->Class == IDCMP_GADGETDOWN)
                {
                struct Gadget *gad = (struct Gadget *)msg->IAddress;
                if (gad->GadgetID == GID_CLOSE)
                    {
                    done = TRUE;
                    }
                else if (gad->GadgetID == GID_TILE)
                    {
                    if (++wi->tile == TILE_COUNT)
                        {
                        wi->tile = 0;
                        }
                    BltBitMapRastPort(wi->gfx, wi->tile << 4, 16, rp, 16, 0, 16, 16, 0xc0);
                    }
                else if (gad->GadgetID == GID_TRIGGER)
                    {
                    if (++wi->trigger == 4)
                        {
                        wi->trigger = 0;
                        }
                    displayNumber(rp, 40, 4 + rp->Font->tf_Baseline, wi->trigger);
                    }
                }
            else if (msg->Class == IDCMP_MOUSEBUTTONS)
                {
                mouseButtons(wi, msg->Code, msg->MouseX, msg->MouseY);
                }
            else if (msg->Class == IDCMP_MOUSEMOVE)
                {
                mouseMove(wi, msg->Code, msg->MouseX, msg->MouseY);
                }
            ReplyMsg((struct Message *)msg);
            }
        }
    return(0);
}
