
#include <graphics/view.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "ILBM.h"

void showILBM(ILBM *ilbm)
{
    struct View view;
    struct ViewPort vp;
    struct RasInfo ri;

    InitView(&view);
    view.ViewPort = &vp;
    InitVPort(&vp);
    vp.DxOffset = vp.DyOffset = 0;
    vp.DWidth = 320;
    vp.DHeight = 256;
    vp.RasInfo = &ri;
    vp.ColorMap = ilbm->cm;
    vp.Next = NULL;
    ri.RxOffset = ri.RyOffset = 0;
    ri.BitMap = ilbm->bm;
    ri.Next = NULL;
    MakeVPort(&view, &vp);
    MrgCop(&view);
    LoadView(&view);
    Delay(300);

    FreeCprList(view.LOFCprList);
    FreeVPortCopLists(&vp);
}

int main(void)
{
    ILBM ilbm = { 0 };

    if (queryILBM(&ilbm, "Data/Graphics.iff"))
    {
        if (loadCMAP(&ilbm) && loadILBM(&ilbm))
        {
            showILBM(&ilbm);
        }
        freeILBM(&ilbm);
        closeIFF(&ilbm.iff);
        freeIFF(&ilbm.iff);
    }
    return(0);
}
