
#include <stdlib.h>
#include <stdio.h>

#include <exec/memory.h>
#include <exec/interrupts.h>
#include <intuition/screens.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

struct Library *IntuitionBase, *IFFParseBase, *DiskfontBase;

struct copperInfo
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screenInfo
{
    struct BitMap *bm[2];
    struct TextFont *tf;
    struct Screen *s;
    struct DBufInfo *dbi;
    struct MsgPort *sp;
    BOOL safe;
    UWORD frame;
    struct Interrupt is;
    struct copperInfo ci;
};

extern __far struct Custom custom;
extern void myCopper(void);

VOID closeLibs()
{
    CloseLibrary(DiskfontBase);
    CloseLibrary(IFFParseBase);
    CloseLibrary(IntuitionBase);
}

struct Library *openLib(STRPTR name, LONG v)
{
    struct Library *l;
    if (l = OpenLibrary(name, v))
    {
        return(l);
    }
    else
    {
        printf("Couldn't open %s V%ld!\n", name, v);
    }
    return(NULL);
}

BOOL openLibs()
{
    if (IntuitionBase = openLib("intuition.library", 39L))
    {
        if (IFFParseBase = openLib("iffparse.library", 39L))
        {
            if (DiskfontBase = openLib("diskfont.library", 39L))
            {
                atexit(closeLibs);
                return(TRUE);
            }
            CloseLibrary(IFFParseBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

BOOL prepScreen(struct screenInfo *si)
{
    static struct TextAttr ta = { "centurion.font", 9, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED };

    if (!(si->bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL)))
    {
        printf("Out of graphics mem!\n");
    }
    else
    {
        if (!(si->tf = OpenDiskFont(&ta)))
        {
            printf("Couldn't open %s size %d!\n", ta.ta_Name, ta.ta_YSize);
        }
        else
        {
            if (!(si->s = OpenScreenTags(NULL,
                SA_Width,   320,
                SA_Height,  256,
                SA_Depth,   5,
                SA_BitMap,  si->bm[0],
                SA_DisplayID,   LORES_KEY,
                SA_Font,    &ta,
                TAG_DONE)))
            {
                printf("Couldn't open screen!\n"):
            }
            else
            {
                struct ViewPort *vp =  &si->s->ViewPort;

                if (!(si->dbi = AllocDBufInfo(vp)))
                {
                    printf("Out of memory!\n")
                }
                else
                {
                    if (!(si->sp = CreateMsgPort()))
                    {
                        printf("Couldn't create msgport!\n");
                    }
                    else
                    {
                        struct UCopList *ucl;

                        si->dbi->dbi_SafeMessage.mn_ReplyPort = si->sp;
                        si->safe = TRUE;
                        si->frame = 1;

                        if (!(ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR)))
                        {
                            printf("Out of memory!\n");
                        }
                        else
                        {
                            CINIT(ucl, 3);
                            CWAIT(ucl, 0, 0);
                            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                            CEND(ucl);

                            Forbid();
                            vp->UCopIns = ucl;
                            Permit();

                            if ((si->ci.signal = AllocSignal(-1)) == -1)
                            {
                                printf("Couldn't alloc signal!\n");
                            }
                            else
                            {
                                struct Interrupt *is = &si->is;

                                si->ci.task = FindTask(NULL);
                                si->ci.vp = vp;

                                is->is_Code = myCopper;
                                is->is_Data = (APTR)&si->ci;
                                is->is_Node.ln_Pri = 0;

                                AddIntServer(INTB_COPER, is);
                                return(TRUE);
                            }
                        }
                        DeleteMsgPort(si->sp);
                    }
                    FreeDBufInfo(si->dbi);
                }
                CloseScreen(si->s);
            }
            CloseFont(si->tf);
        }
        FreeBitMap(si->bm[0]);
    }
    return(FALSE);
}
