
/*=================================================================*/
/* $Log:	Screen.c,v $
 * Revision 1.1  12/.0/.2  .1:.0:.4  RS
 * Initial revision
 *  */
/*=================================================================*/

#include <intuition/screens.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>

#include "Screen.h"
#include "iffp/ilbmapp.h"

__far extern struct Custom custom;
void myCopper(void);

BOOL getGraphics(struct graphics *gfx, STRPTR name)
{
    struct IFFHandle *iff;
    struct ILBMInfo ii = { 0 };
    LONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        ID_ILBM, ID_CAMG,
        0
    }, stops[] =
    {
        ID_ILBM, ID_BODY,
        0
    };

    if (ii.ParseInfo.iff = iff = AllocIFF())
    {
        ii.ParseInfo.propchks = props;
        ii.ParseInfo.collectchks = NULL;
        ii.ParseInfo.stopchks = stops;

        if (!loadbrush(&ii, name))
        {
            gfx->modeID = ii.camg;

            gfx->dclip.MinX = gfx->dclip.MinY = 0;
            gfx->dclip.MaxX = 319;
            gfx->dclip.MaxY = 255;

            if (gfx->bm = AllocBitMap(ii.Bmhd.w, ii.Bmhd.h, ii.Bmhd.nPlanes, BMF_INTERLEAVED, NULL))
            {
                BltBitMap(ii.brbitmap, 0, 0, gfx->bm, 0, 0, ii.Bmhd.w, ii.Bmhd.h, 0xc0, 0xff, NULL);

                if (gfx->pal = AllocVec(ii.crecsize, MEMF_PUBLIC))
                {
                    CopyMem(ii.colorrecord, gfx->pal, ii.crecsize);
                    WaitBlit();
                    unloadbrush(&ii);
                    FreeIFF(iff);
                    return(TRUE);
                }
                FreeBitMap(gfx->bm);
            }
            unloadbrush(&ii);
        }
        FreeIFF(iff);
    }
    return(FALSE);
}

void freeGraphics(struct graphics *gfx)
{
    FreeVec(gfx->pal);
    FreeBitMap(gfx->bm);
}

/*=================================================================*/
/* Open game screen */

/* Load color palette and resolution from IFF ILBM */
/* s:       Screen instance, */
/* w, h, d: Width, height, depth dimensions */
/* gfx:     Graphics instance */
/* gfxName: Graphics name */

/*=================================================================*/

struct Screen *openScreen(struct screen *s, UWORD w, UWORD h, UBYTE d, struct graphics *gfx, STRPTR gfxName)
{
    if (getGraphics(gfx, gfxName))
    {
        if (s->bm[0] = AllocBitMap(w, h, d, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
        {
            if (s->bm[1] = AllocBitMap(w, h, d, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
            {
                /* TODO: Fill s->bm[0] with gfx->bm here. */

                BltBitMap(gfx->bm, 0, 0, s->bm[0], 0, 0, 320, 256, 0xc0, 0xff, NULL);
                WaitBlit();

                if (s->s = OpenScreenTags(NULL,
                    SA_DClip,       &gfx->dclip,
                    SA_BitMap,      s->bm[0],
                    SA_DisplayID,   gfx->modeID,
                    SA_Colors32,    gfx->pal,
                    SA_Title,       "Gear works screen",
                    SA_ShowTitle,   FALSE,
                    SA_Quiet,       TRUE,
                    SA_Exclusive,   TRUE,
                    SA_BackFill,    LAYERS_NOBACKFILL,
                    TAG_DONE))
                {
                    struct ViewPort *vp = &s->s->ViewPort;

                    if (s->dbi = AllocDBufInfo(vp))
                    {
                        if (s->dbi->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort())
                        {
                            struct copper *cop = &s->cop;

                            s->safe = TRUE;
                            s->frame = 1;
                            s->gfx = gfx; /* Game graphics */

                            cop->vp = vp;
                            if ((cop->signal = AllocSignal(-1)) != -1)
                            {
                                struct Interrupt *is = &s->is;
                                struct UCopList *ucl;

                                cop->task = FindTask(NULL);

                                is->is_Code = myCopper;
                                is->is_Data = (APTR)cop;
                                is->is_Node.ln_Pri = COP_PRI;

                                if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                                {
                                    CINIT(ucl, 3);
                                    CWAIT(ucl, COP_LINE, 0);
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
                            DeleteMsgPort(s->dbi->dbi_SafeMessage.mn_ReplyPort);
                        }
                        FreeDBufInfo(s->dbi);
                    }
                    CloseScreen(s->s);
                }
                FreeBitMap(s->bm[1]);
            }
            FreeBitMap(s->bm[0]);
        }
        freeGraphics(gfx);
    }
    return(NULL);
}

/*=================================================================*/
/* Close game screen */

/*=================================================================*/

void closeScreen(struct screen *s)
{
    struct MsgPort *mp = s->dbi->dbi_SafeMessage.mn_ReplyPort;

    RemIntServer(INTB_COPER, &s->is);
    FreeSignal(s->cop.signal);

    if (!s->safe)
    {
        while (!GetMsg(mp))
        {
            WaitPort(mp);
        }
    }

    DeleteMsgPort(mp);
    FreeDBufInfo(s->dbi);
    CloseScreen(s->s);
    FreeBitMap(s->bm[1]);
    FreeBitMap(s->bm[0]);
    freeGraphics(s->gfx);
}
