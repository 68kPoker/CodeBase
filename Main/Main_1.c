
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/dos_protos.h>

#include "Main.h"
#include "ILBM.h"
#include "Blit.h"

__far extern struct Custom custom;
extern void myCopper();

__chip UWORD mouseData[] =
{
    0, 0,
    0x1008, 0x6006,
    0x6816, 0xb00d,
    0x6816, 0xf00f,
    0x9009, 0x6006,
    0x6186, 0x0000,
    0x0240, 0x0180,
    0x05a0, 0x03c0,
    0x0bd0, 0x07e0,
    0x0bd0, 0x07e0,
    0x05a0, 0x03c0,
    0x0240, 0x0180,
    0x6186, 0x0000,
    0x9009, 0x6006,
    0x6816, 0xf00f,
    0x6816, 0xb00d,
    0x1008, 0x6006,
    0, 0
};


BOOL initMain(struct mainData *md)
{
    struct TextAttr ta =
    {
        "centurion.font",
        9,
        FS_NORMAL,
        FPF_DISKFONT|FPF_DESIGNED
    };

    struct Rectangle clip =
    {
        0, 0, 319, 255
    };

    if (md->bitmaps[0] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        if (md->bitmaps[1] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
        {
            if (md->font = OpenDiskFont(&ta))
            {
                if (md->colors = AllocVec(((COLORS * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC|MEMF_CLEAR))
                {
                    md->colors[0] = COLORS << 16;

                    if (md->screen = OpenScreenTags(NULL,
                        SA_DClip,       &clip,
                        SA_BitMap,      md->bitmaps[0],
                        SA_Font,        &ta,
                        SA_DisplayID,   LORES_KEY,
                        SA_Quiet,       TRUE,
                        SA_ShowTitle,   FALSE,
                        SA_Exclusive,   TRUE,
                        SA_Colors32,    md->colors,
                        SA_Title,       TITLE,
                        SA_BackFill,    LAYERS_NOBACKFILL,
                        TAG_DONE))
                    {
                        struct UCopList *ucl;

                        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                        {
                            struct ViewPort *vp = &md->screen->ViewPort;
                            struct copperData *cd = &md->copData;

                            CINIT(ucl, 3);
                            CWAIT(ucl, 0, 0);
                            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                            CEND(ucl);

                            Forbid();
                            vp->UCopIns = ucl;
                            Permit();

                            RethinkDisplay();

                            if ((cd->signal = AllocSignal(-1)) != -1)
                            {
                                struct Interrupt *is = &md->copInt;

                                cd->vport = vp;
                                cd->task = FindTask(NULL);

                                is->is_Code = myCopper;
                                is->is_Data = (APTR)cd;
                                is->is_Node.ln_Pri = 0;
                                is->is_Node.ln_Name = TITLE;

                                if (md->dbi = AllocDBufInfo(vp))
                                {
                                    if (md->safePort = CreateMsgPort())
                                    {
                                        md->dbi->dbi_SafeMessage.mn_ReplyPort = md->safePort;
                                        md->safe = TRUE;
                                        md->frame = 1;

                                        if (md->window = OpenWindowTags(NULL,
                                            WA_CustomScreen,    md->screen,
                                            WA_Left,            0,
                                            WA_Top,             0,
                                            WA_Width,           320,
                                            WA_Height,          256,
                                            WA_Backdrop,        TRUE,
                                            WA_Borderless,      TRUE,
                                            WA_Activate,        TRUE,
                                            WA_RMBTrap,         TRUE,
                                            WA_SimpleRefresh,   TRUE,
                                            WA_BackFill,        LAYERS_NOBACKFILL,
                                            WA_IDCMP,           IDCMP_MOUSEBUTTONS,
                                            WA_ReportMouse,     TRUE,
                                            TAG_DONE))
                                        {
                                            SetPointer(md->window, mouseData, 16, 16, -8, -8);
                                            AddIntServer(INTB_COPER, is);
                                            return(TRUE);
                                        }
                                        DeleteMsgPort(md->safePort);
                                    }
                                    FreeDBufInfo(md->dbi);
                                }
                                FreeSignal(cd->signal);
                            }
                        }
                        CloseScreen(md->screen);
                    }
                    FreeVec(md->colors);
                }
                CloseFont(md->font);
            }
            FreeBitMap(md->bitmaps[1]);
        }
        FreeBitMap(md->bitmaps[0]);
    }
    return(FALSE);
}

void freeMain(struct mainData *md)
{
    RemIntServer(INTB_COPER, &md->copInt);
    CloseWindow(md->window);

    if (!md->safe)
    {
        while (!GetMsg(md->safePort))
        {
            WaitPort(md->safePort);
        }
    }
    DeleteMsgPort(md->safePort);
    FreeDBufInfo(md->dbi);
    FreeSignal(md->copData.signal);
    CloseScreen(md->screen);
    FreeVec(md->colors);
    CloseFont(md->font);
    FreeBitMap(md->bitmaps[1]);
    FreeBitMap(md->bitmaps[0]);
}

int main()
{
    struct mainData md;
    struct ColorMap *cm;

    if (initMain(&md))
    {
        cm = md.screen->ViewPort.ColorMap;

        if (loadILBM("Dane/Magazyn.iff", &md.gfx, &cm))
        {
            MakeScreen(md.screen);
            RethinkDisplay();

            SetSignal(0L, 1L << md.copData.signal);
            Wait(1L << md.copData.signal);

            BltBitMap(md.gfx, 0, 0, md.bitmaps[0], 0, 0, 320, 32, 0xc0, 0xff, NULL);
            bltBoardRastPort(md.gfx, 0, 32, md.window->RPort, 0, 32, 320, 224);
            Delay(300);
            freeILBM(md.gfx, NULL);
        }
        freeMain(&md);
    }
    return(0);
}
