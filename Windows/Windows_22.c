
#include "Window.h"

UWORD rbxy[] =
{
    0, 0,
    79, 0,
    79, 15,
    0, 15,
    0, 1
};

BOOL initMenu(struct windowData *wd)
{
    struct IntuiText *it = wd->it + GID_MENU1;
    struct Gadget *gad = wd->gads + GID_MENU1;

    wd->selected = -1;

    wd->rb.LeftEdge = 0;
    wd->rb.TopEdge  = 0;
    wd->rb.FrontPen = 1;
    wd->rb.BackPen  = 0;
    wd->rb.DrawMode = JAM1;
    wd->rb.Count    = 5;
    wd->rb.XY       = rbxy;
    wd->rb.NextBorder = NULL;

    wd->sb.LeftEdge = 0;
    wd->sb.TopEdge  = 0;
    wd->sb.FrontPen = 2;
    wd->sb.BackPen  = 0;
    wd->sb.DrawMode = JAM1;
    wd->sb.Count    = 5;
    wd->sb.XY       = rbxy;
    wd->sb.NextBorder = NULL;

    it->FrontPen    = 1;
    it->BackPen     = 0;
    it->DrawMode    = JAM1;
    it->LeftEdge    = 2;
    it->TopEdge     = 6;
    it->ITextFont   = NULL;
    it->IText       = "Menu1";
    it->NextText    = NULL;

    it++;

    it->FrontPen    = 1;
    it->BackPen     = 0;
    it->DrawMode    = JAM1;
    it->LeftEdge    = 2;
    it->TopEdge     = 6;
    it->ITextFont   = NULL;
    it->IText       = "Menu2";
    it->NextText    = NULL;

    it++;

    it->FrontPen    = 3;
    it->BackPen     = 0;
    it->DrawMode    = JAM1;
    it->LeftEdge    = 2;
    it->TopEdge     = 6;
    it->ITextFont   = NULL;
    it->IText       = "Menu3";
    it->NextText    = NULL;

    gad->NextGadget = gad + 1;
    gad->LeftEdge   = 0;
    gad->TopEdge    = 0;
    gad->Width      = 80;
    gad->Height     = 16;
    gad->Flags      = GFLG_GADGHIMAGE;
    gad->Activation = GACT_IMMEDIATE;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->GadgetRender = &wd->rb;
    gad->SelectRender = &wd->sb;
    gad->GadgetText = wd->it + GID_MENU1;
    gad->MutualExclude = 0;
    gad->SpecialInfo = NULL;
    gad->GadgetID = GID_MENU1;
    gad->UserData = NULL;

    gad++;

    gad->NextGadget = gad + 1;
    gad->LeftEdge   = 81;
    gad->TopEdge    = 0;
    gad->Width      = 80;
    gad->Height     = 16;
    gad->Flags      = GFLG_GADGHIMAGE;
    gad->Activation = GACT_IMMEDIATE;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->GadgetRender = &wd->rb;
    gad->SelectRender = &wd->sb;
    gad->GadgetText = wd->it + GID_MENU2;
    gad->MutualExclude = 0;
    gad->SpecialInfo = NULL;
    gad->GadgetID = GID_MENU2;
    gad->UserData = NULL;

    gad++;

    gad->NextGadget = NULL;
    gad->LeftEdge   = 162;
    gad->TopEdge    = 0;
    gad->Width      = 80;
    gad->Height     = 16;
    gad->Flags      = GFLG_GADGHIMAGE;
    gad->Activation = GACT_IMMEDIATE;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->GadgetRender = &wd->rb;
    gad->SelectRender = &wd->sb;
    gad->GadgetText = wd->it + GID_MENU3;
    gad->MutualExclude = 0;
    gad->SpecialInfo = NULL;
    gad->GadgetID = GID_MENU3;
    gad->UserData = NULL;

    return(TRUE);
}
