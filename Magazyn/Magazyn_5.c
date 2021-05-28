
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>
#include <exec/interrupts.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

struct Interrupt is, bltis;
struct Task *task;
ULONG signal, bltsignal;

__far extern struct Custom custom;

extern void copperInt(), blitterInt();

main()
{
    struct BitMap *bm[2];
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };
    BYTE sig, bltsig;
    struct TagItem vctags[] =
    {
        VC_IntermediateCLUpdate, FALSE,
        TAG_DONE
    };
    BOOL prev;

    task = FindTask(NULL);

    if ((sig = AllocSignal(-1)) == -1)
    {
        return(0);
    }

    if ((bltsig = AllocSignal(-1)) == -1)
    {
        FreeSignal(sig);
        return(0);
    }

    signal = 1L << sig;
    bltsignal = 1L << bltsig;

    /* SetTaskPri(task, 2); */

    is.is_Node.ln_Pri = 100;
    is.is_Node.ln_Name = "Magazyn";
    is.is_Code = copperInt;
    is.is_Data = NULL;

    AddIntServer(INTB_COPER, &is);

    bltis.is_Node.ln_Type = NT_INTERRUPT;
    bltis.is_Code = blitterInt;
    bltis.is_Data = NULL;

    if (bm[0] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (bm[1] = AllocBitMap(320, 256, 5, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            s = OpenScreenTags(NULL,
                SA_DClip, &dclip,
                SA_DisplayID, LORES_KEY,
                SA_Depth, 5,
                SA_Quiet, TRUE,
                SA_Exclusive, TRUE,
                SA_ShowTitle, FALSE,
                SA_BackFill, LAYERS_NOBACKFILL,
                SA_BitMap, bm[0],
                SA_VideoControl, vctags,
                TAG_DONE);

            if (s)
            {
                struct MsgPort *mp[2];

                if (mp[0] = CreateMsgPort())
                {
                    if (mp[1] = CreateMsgPort())
                    {
                        struct ViewPort *vp = &s->ViewPort;
                        struct DBufInfo *dbi;

                        if (dbi = AllocDBufInfo(vp))
                        {
                            struct UCopList *ucl;

                            dbi->dbi_SafeMessage.mn_ReplyPort = mp[0];
                            /* dbi->dbi_DispMessage.mn_ReplyPort = mp[1]; */

                            if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                            {
                                BOOL safe[2] = { FALSE };
                                WORD frame = 1;
                                ULONG signals[] = { 1L << mp[0]->mp_SigBit, signal, bltsignal };
                                ULONG total = signals[0] | signals[1] | signals[2];
                                BOOL done = FALSE;
                                WORD counter = 0;
                                BOOL cop = FALSE;
                                struct Interrupt *previs;

                                CINIT(ucl, 6);
                                CWAIT(ucl, 0, 0);
                                CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                                CEND(ucl);

                                Forbid();
                                vp->UCopIns = ucl;
                                Permit();
                                RethinkDisplay();

                                ChangeVPBitMap(vp, bm[frame], dbi);
                                frame ^= 1;

                                prev = custom.intenar & INTF_BLIT ? TRUE : FALSE;
                                custom.intena = INTF_BLIT;
                                previs = SetIntVector(INTB_BLIT, &bltis);
                                custom.intena = INTF_SETCLR | INTF_BLIT;
                                WORD blit = 0;

                                while (!done)
                                {
                                    ULONG result = Wait(SIGBREAKF_CTRL_C | total);

                                    if (result & SIGBREAKF_CTRL_C || counter == 100)
                                        break;

                                    if (result & signals[0])
                                    {
                                        cop = TRUE;
                                        if (!safe[0])
                                        {
                                            while (!GetMsg(mp[0]))
                                                WaitPort(mp[0]);
                                            safe[0] = TRUE;
                                        }

                                        struct RastPort *rp = &s->RastPort;
                                        rp->BitMap = bm[frame];
                                        static WORD x = 0, prevx[2] = { 0 };



                                        prevx[frame] = x;
                                        x++;
                                    }
                                    if (result & signals[1])
                                    {
                                        WaitBlit();

                                        ChangeVPBitMap(vp, bm[frame], dbi);
                                        frame ^= 1;
                                        safe[0] = safe[1] = FALSE;
                                        counter++;
                                    }

                                    if (result & signals[2])
                                    {
                                        blit++;
                                    }
                                }
                                if (!safe[0])
                                    while (!GetMsg(mp[0]))
                                        WaitPort(mp[0]);

                                custom.intena = INTF_BLIT;
                                SetIntVector(INTB_BLIT, previs);
                                if (prev)
                                {
                                    custom.intena = INTF_SETCLR | INTF_BLIT;
                                }
                                printf("Blits: %d\n", blit);
                            }
                            FreeDBufInfo(dbi);
                        }
                        DeleteMsgPort(mp[1]);
                    }
                    DeleteMsgPort(mp[0]);
                }
                CloseScreen(s);
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    RemIntServer(INTB_COPER, &is);
    /* SetTaskPri(task, 0); */
    FreeSignal(bltsig);
    FreeSignal(sig);
    return(0);
}
