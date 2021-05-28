
#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

#include "Windows.h"

struct TagItem wintags[] =
{
    WA_CustomScreen,    0,
    WA_Left,            0,
    WA_Top,             0,
    WA_Width,           320,
    WA_Height,          256,
    WA_AutoAdjust,      FALSE,
    WA_IDCMP,           0,
    WA_BackFill,        (ULONG)LAYERS_NOBACKFILL,
    WA_SimpleRefresh,   TRUE,
    WA_ReportMouse,     TRUE,
    WA_Backdrop,        TRUE,
    WA_Borderless,      TRUE,
    WA_Activate,        TRUE,
    WA_RMBTrap,         TRUE,
    TAG_DONE
};

void initGadget(struct Gadget *gad, struct Gadget *prev, WORD left, WORD top, WORD width, WORD height, WORD gid)
{
    if (prev)
        prev->NextGadget = gad;

    gad->NextGadget = NULL;
    gad->LeftEdge   = left;
    gad->TopEdge    = top;
    gad->Width      = width;
    gad->Height     = height;
    gad->Flags      = GFLG_GADGHNONE;
    gad->Activation = GACT_RELVERIFY;
    gad->GadgetType = GTYP_BOOLGADGET;
    gad->GadgetRender = NULL;
    gad->SelectRender = NULL;
    gad->GadgetText = NULL;
    gad->GadgetID   = gid;
    gad->SpecialInfo  = NULL;
    gad->MutualExclude = 0;
    gad->UserData = NULL;
}

void initMenu(struct windowInfo *wi, struct Gadget *gads, WORD max, WORD menu, BOOL horiz)
{
    WORD i;
    struct Gadget *prev = NULL;
    const WORD bits = 8, left, top;
    static UWORD tops[]  = { 0, 16, 64,  16, 240 };
    static UWORD lefts[] = { 0,  0, 64, 304,   0 };

    left = lefts[menu];
    top  = tops [menu];

    menu <<= bits;

    for (i = 0; i < max; i++)
    {
        RectFill(wi->w->RPort, left, top, left + 15, top + 15);
        initGadget(gads + i, prev, left, top, 16, 16, i | menu);
        if (horiz)
            left += 16;
        else
            top += 16;
        prev = gads + i;
    }
}

void initGadgets(struct windowInfo *wi)
{
    initMenu(wi, wi->top_gads,    MAX_TOP_GADS,    MID_TOP,     TRUE);
    initMenu(wi, wi->left_gads,   MAX_LEFT_GADS,   MID_LEFT,    FALSE);
    initMenu(wi, wi->center_gads, MAX_CENTER_GADS, MID_CENTER,  TRUE);
    initMenu(wi, wi->right_gads,  MAX_RIGHT_GADS,  MID_RIGHT,   FALSE);
    initMenu(wi, wi->bottom_gads, MAX_BOTTOM_GADS, MID_BOTTOM,  TRUE);
}

struct Window *openWindow(struct TagItem *taglist, ULONG tag1, ...)
{
    struct Window *w;

    ApplyTagChanges(taglist, (struct TagItem *)&tag1);

    if (w = OpenWindowTagList(NULL, taglist))
    {
        return(w);
    }
    return(NULL);
}

void addWindowInfo(struct Window *w, struct windowInfo *wi)
{
    wi->w = w;
    w->UserData = (APTR)wi;
}

void addGadgets(struct windowInfo *wi)
{
    AddGList(wi->w, wi->top_gads, -1, -1, NULL);
    AddGList(wi->w, wi->left_gads, -1, -1, NULL);
    AddGList(wi->w, wi->center_gads, -1, -1, NULL);
    AddGList(wi->w, wi->right_gads, -1, -1, NULL);
    AddGList(wi->w, wi->bottom_gads, -1, -1, NULL);
}
