
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>

#include "Screen.h"

#define COPPERLIST_LEN 3

__far extern struct Custom custom;

extern void myCopper(void);

struct TextAttr ta =
{
    "ld.font", 8, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED
};

BOOL openScreenFont(struct mainData *md, struct screenData *sd)
{
    if (sd->tf = OpenDiskFont(&ta))
    {
        if (openScreen(md, sd))
        {
            return(TRUE);
        }
        CloseFont(sd->tf);
    }
    return(FALSE);
}

void closeScreenFont(struct screenData *sd)
{
    closeScreen(sd);
    CloseFont(sd->tf);
}

BOOL openScreen(struct mainData *md, struct screenData *sd)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (sd->bm[0] = AllocBitMap(WIDTH, HEIGHT, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        if (sd->s = OpenScreenTags(NULL,
            SA_BitMap,      sd->bm[0],
            SA_DClip,       &dclip,
            SA_Font,        &ta,
            SA_DisplayID,   MODEID,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE))
        {
            if (sd->mp = CreateMsgPort())
            {
                if (sd->sb[0] = AllocScreenBuffer(sd->s, sd->bm[0], 0))
                {
                    sd->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->mp;
                    sd->safe = TRUE;
                    sd->frame = 0;

                    sd->copis.is_Code = myCopper;
                    sd->copis.is_Data = (APTR)&sd->copData;
                    sd->copis.is_Node.ln_Pri = 0;

                    if ((sd->copData.signal = AllocSignal(-1)) != -1)
                    {
                        struct UCopList *ucl;

                        sd->copData.vp = &sd->s->ViewPort;
                        sd->copData.task = FindTask(NULL);

                        if (openBDWindow(sd))
                        {
                            if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                            {
                                CINIT(ucl, COPPERLIST_LEN);
                                CWAIT(ucl, 200, 0);
                                CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                                CEND(ucl);

                                Forbid();
                                sd->s->ViewPort.UCopIns = ucl;
                                Permit();

                                RethinkDisplay();

                                AddIntServer(INTB_COPER, &sd->copis);
                                return(TRUE);
                            }
                            closeWindow(sd, sd->bdw);
                        }
                        FreeSignal(sd->copData.signal);
                    }
                    FreeScreenBuffer(sd->s, sd->sb[0]);
                }
                DeleteMsgPort(sd->mp);
            }
            CloseScreen(sd->s);
        }
        FreeBitMap(sd->bm[0]);
    }
    return(FALSE);
}

void closeScreen(struct screenData *sd)
{
    RemIntServer(INTB_COPER, &sd->copis);
    closeWindow(sd, sd->bdw);
    FreeSignal(sd->copData.signal);

    if (!sd->safe)
    {
        while (!GetMsg(sd->mp))
        {
            WaitPort(sd->mp);
        }
    }

    FreeScreenBuffer(sd->s, sd->sb[0]);
    DeleteMsgPort(sd->mp);
    CloseScreen(sd->s);
    FreeBitMap(sd->bm[0]);
}

void closeWindow(struct screenData *sd, struct Window *w)
{
    CloseWindow(w);
}

BOOL openBDWindow(struct screenData *sd)
{
    initMenu(&sd->wd[WID_BOARD]);

    if (sd->bdw = OpenWindowTags(NULL,
        WA_Left,        0,
        WA_Top,         0,
        WA_Width,       sd->s->Width,
        WA_Height,      sd->s->Height,
        WA_CustomScreen,    sd->s,
        WA_Gadgets,     sd->wd[WID_BOARD].gads,
        WA_Backdrop,    TRUE,
        WA_Borderless,  TRUE,
        WA_RMBTrap,     TRUE,
        WA_Activate,    TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,    LAYERS_NOBACKFILL,
        WA_IDCMP,       IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETDOWN,
        WA_ReportMouse, TRUE,
        TAG_DONE))
    {
        return(TRUE);
    }
    return(FALSE);
}

BOOL openMenuWindow(struct screenData *sd, WORD wid)
{
    /* initMenu1(&sd->wd[WID_MENU1]); */

    if (sd->menuw = OpenWindowTags(NULL,
        WA_Left,        wid * 81,
        WA_Top,         17,
        WA_Width,       80,
        WA_Height,      160,
        WA_CustomScreen,    sd->s,
        WA_Gadgets,     NULL, /* sd->wd[WID_BOARD].gads, */
        WA_Backdrop,    TRUE,
        WA_Borderless,  TRUE,
        WA_RMBTrap,     TRUE,
        WA_Activate,    TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,    LAYERS_NOBACKFILL,
        WA_IDCMP,       IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETDOWN,
        WA_ReportMouse, TRUE,
        TAG_DONE))
    {
        struct RastPort *rp = sd->menuw->RPort;

        sd->update = TRUE;

/*
        SetOutlinePen(rp, 1);
        SetAPen(rp, 3);

        RectFill(rp, 0, 0, sd->menuw->Width - 1, sd->menuw->Height - 1);
*/
/*
        SetAPen(rp, 1);
        Move(rp, 0, 0);
        Draw(rp, sd->menuw->Width - 1, 0);
        Draw(rp, sd->menuw->Width - 1, sd->menuw->Height - 1);
        Draw(rp, 0, sd->menuw->Height - 1);
        Draw(rp, 0, 1);
*/
        return(TRUE);
    }
    return(FALSE);
}
