
/*
** Project: Magazyn
** File:    Screen.c
** Desc:    Screen management functions
*/

#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>

#include "Screen.h"

__far extern struct Custom custom;
extern void myCopper(void);

struct Screen *openScreen(struct screenParam *p, struct screen *s)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (s->s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_BitMap,      s->bm[0],
        p->colors ? SA_Colors32 : TAG_IGNORE,    p->colors,
        SA_Font,        p->ta,
        SA_DisplayID,   LORES_KEY,
        SA_Title,       p->name,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        TAG_DONE))
    {
        struct windowParam wp =
        {
            s->s,
            0, 0, s->s->Width, s->s->Height,
            TRUE,
            IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE
        };

        s->s->UserData = (APTR)s;

        if (openWindow(&wp, &s->bdw))
        {
            struct ViewPort *vp = &s->s->ViewPort;

            if (s->dbi = AllocDBufInfo(vp))
            {
                if (s->safemp = CreateMsgPort())
                {
                    struct Interrupt *is = &s->is;
                    struct copper *cop =  &s->cop;

                    s->dbi->dbi_SafeMessage.mn_ReplyPort = s->safemp;
                    s->safe = TRUE;
                    s->frame = 1;

                    is->is_Code = myCopper;
                    is->is_Data = (APTR)cop;
                    is->is_Node.ln_Pri = 0;

                    if ((cop->signal = AllocSignal(-1)) != -1)
                    {
                        struct UCopList *ucl;

                        cop->vp = vp;
                        cop->task = FindTask(NULL);

                        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                        {
                            CINIT(ucl, 3);
                            CWAIT(ucl, 0, 0);
                            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                            CEND(ucl);

                            Forbid();
                            vp->UCopIns = ucl;
                            Permit();

                            AddIntServer(INTB_COPER, is);
                            return(s->s);
                        }
                        FreeSignal(cop->signal);
                    }
                    DeleteMsgPort(s->safemp);
                }
                FreeDBufInfo(s->dbi);
            }
            closeWindow(&s->bdw);
        }
        CloseScreen(s->s);
    }
    return(NULL);
}

void closeScreen(struct screen *s)
{
    RemIntServer(INTB_COPER, &s->is);
    FreeSignal(s->cop.signal);

    if (!s->safe)
    {
        while (!GetMsg(s->safemp))
        {
            WaitPort(s->safemp);
        }
    }

    DeleteMsgPort(s->safemp);
    FreeDBufInfo(s->dbi);
    closeWindow(&s->bdw);
    CloseScreen(s->s);
}

struct Window *openWindow(struct windowParam *p, struct window *w)
{
    if (w->w = OpenWindowTags(NULL,
        WA_CustomScreen,    p->s,
        WA_Left,            p->left,
        WA_Top,             p->top,
        WA_Width,           p->width,
        WA_Height,          p->height,
        WA_Backdrop,        p->backdrop,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           p->idcmp,
        TAG_DONE))
    {
        w->w->UserData = (APTR)w;
        return(w->w);
    }
    return(NULL);
}

void closeWindow(struct window *w)
{
    CloseWindow(w->w);
}

struct Screen *prepScreen(struct screen *s)
{
    struct screenParam sp;
    struct TextAttr ta =
    {
        "centurion.font", 9,
        FS_NORMAL,
        FPF_DISKFONT|FPF_DESIGNED
    };

    if (s->tf = OpenDiskFont(&ta))
    {
        sp.ta = &ta;
        sp.colors = NULL;
        sp.name = "Magazyn";

        if (s->bm[0] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
        {
            if (s->bm[1] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
            {
                if (openScreen(&sp, s))
                {
                    return(s->s);
                }
                FreeBitMap(s->bm[1]);
            }
            FreeBitMap(s->bm[0]);
        }
        CloseFont(s->tf);
    }
    return(NULL);
}

void freeScreen(struct screen *s)
{
    closeScreen(s);
    FreeBitMap(s->bm[1]);
    FreeBitMap(s->bm[0]);
    CloseFont(s->tf);
}
