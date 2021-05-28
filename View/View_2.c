
#include <graphics/videocontrol.h>
#include <graphics/gfxbase.h>
#include <clib/graphics_protos.h>

#include "View.h"

extern struct GfxBase *GfxBase;

/** Prepare view to display **/

BOOL initView(VPACK *p, ULONG modeID, WORD width, WORD height, UBYTE depth)
{
    VIEW *view = &p->view;
    VPORT *vp = &p->vp;
    RASINFO *ri = &p->ri;
    CMAP cm;
    BITMAP bm;
    VEXTRA ve;
    VPEXTRA vpe;
    struct TagItem vctags[] =
    {
        { VTAG_ATTACH_CM_SET, 0 },
        { VTAG_VIEWPORTEXTRA_SET, 0 },
        { VTAG_NORMAL_DISP_SET, 0 },
        { VTAG_END_CM, 0 }
    };
    struct DimensionInfo dim;

    InitView(view);
    view->ViewPort = vp;
    if (p->ve = ve = GfxNew(VIEW_EXTRA_TYPE))
    {
        GfxAssociate(view, ve);
        view->Modes |= EXTEND_VSTRUCT;

        if (p->monspec = OpenMonitor(NULL, modeID))
        {
            ve->Monitor = p->monspec;
            if (ri->BitMap = bm = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
            {
                ri->RxOffset = ri->RyOffset = 0;
                ri->Next = NULL;
                InitVPort(vp);
                vp->RasInfo = ri;
                vp->DxOffset = vp->DyOffset = 0;
                vp->DWidth = 320;
                vp->DHeight = 256;
                if (p->vpe = vpe = GfxNew(VIEWPORT_EXTRA_TYPE))
                {
                    vctags[1].ti_Data = (ULONG)vpe;
                    if (GetDisplayInfoData(NULL, (UBYTE *)&dim, sizeof(dim), DTAG_DIMS, modeID) > 0)
                    {
                        vpe->DisplayClip = dim.Nominal;
                        if (vctags[2].ti_Data = (ULONG)FindDisplayInfo(modeID))
                        {
                            if (vp->ColorMap = cm = GetColorMap(1 << depth))
                            {
                                vctags[0].ti_Data = (ULONG)vp;
                                if (!VideoControl(cm, vctags))
                                {
                                    MakeVPort(view, vp);
                                    MrgCop(view);
                                    return(TRUE);
                                }
                                FreeColorMap(cm);
                            }
                        }
                    }
                    GfxFree(vpe);
                }
                FreeBitMap(bm);
            }
            CloseMonitor(p->monspec);
        }
        GfxFree(ve);
    }
    return(FALSE);
}

/** Get previous (active) View **/

VIEW *saveView(VOID)
{
    return(GfxBase->ActiView);
}

/** Free view once it's done **/

VOID freeView(VPACK *p)
{
    VIEW *view = &p->view;
    VPORT *vp = &p->vp;

    FreeCprList(view->LOFCprList);
    FreeVPortCopLists(vp);
    FreeColorMap(vp->ColorMap);
    GfxFree(p->vpe);
    FreeBitMap(p->ri.BitMap);
    CloseMonitor(p->monspec);
    GfxFree(p->ve);
}
