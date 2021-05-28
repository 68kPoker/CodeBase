
/* $Log$ */

#include <intuition/intuition.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Gadgets.h"

VOID initText(struct IntuiText *text, STRPTR name)
{
    text->LeftEdge = 4;
    text->TopEdge = 4;
    text->NextText = NULL;
    text->FrontPen = TEXT_COLOR;
    text->BackPen = 0;
    text->DrawMode = JAM1;
    text->ITextFont = NULL;
    text->IText = name;
}

VOID initButton(struct Gadget *gad, struct IntuiText *text, WORD gid, WORD x, WORD y, struct Image *render, struct Image *select)
{
    gad->NextGadget = NULL;
    gad->LeftEdge = x;
    gad->TopEdge = y;
    gad->Width = render->Width;
    gad->Height = render->Height;
    gad->Flags = GFLG_GADGIMAGE|GFLG_GADGHIMAGE;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->Activation = GACT_IMMEDIATE|GACT_RELVERIFY;
    gad->GadgetRender = render;
    gad->SelectRender = select;
    gad->GadgetText = text;
    gad->MutualExclude = 0;
    gad->GadgetID = gid;
    gad->UserData = NULL;
    gad->SpecialInfo = NULL;
}

/* Cut Image from BitMap */
BOOL cutImage(struct Image *img, struct BitMap *bm, WORD x, WORD y, WORD width, WORD height)
{
    UBYTE depth = GetBitMapAttr(bm, BMA_DEPTH);
    LONG planeSize = WordWidth(width) * height;
    LONG fullSize = planeSize * depth;
    struct BitMap aux;
    WORD bpr = WordWidth(width) << 1;
    WORD i;

    img->LeftEdge = img->TopEdge = 0;
    img->Width = width;
    img->Height = height;
    img->Depth = depth;
    img->PlanePick = 0xff;
    img->PlaneOnOff = 0x00;
    img->NextImage = NULL;

    if (img->ImageData = AllocVec(fullSize * sizeof(UWORD), MEMF_CHIP))
    {
        InitBitMap(&aux, depth, width, height);
        aux.Planes[0] = (PLANEPTR)img->ImageData;
        for (i = 1; i < depth; i++)
        {
            aux.Planes[i] = aux.Planes[i - 1] + (planeSize << 1);
        }
        BltBitMap(bm, x, y, &aux, 0, 0, bpr << 3, height, 0xc0, 0xff, NULL);
        return(TRUE);
    }
    return(FALSE);
}

VOID freeImage(struct Image *img)
{
    FreeVec(img->ImageData);
}
