
/* Window.c: Auxilliary window functions */

#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>

#include "Window.h"
#include "Tile.h"

struct Window *openBDWindow(struct Screen *s, struct windowInfo *wi)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Width,
        WA_Height,          s->Height,
        WA_Gadgets,         wi->gads,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETUP|IDCMP_REFRESHWINDOW,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

struct Window *openMenuWindow(struct Window *p, WORD width, WORD height)
{
    struct Screen *s = p->WScreen;
    WORD left = (s->Width - width) / 2;
    WORD top = (s->Height - height) / 2;
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            left,
        WA_Top,             top,
        WA_Width,           width,
        WA_Height,          height,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_MOUSEBUTTONS,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

/* moveWindow: This proc moves intuition window by dx/dy */
void moveWindow(struct Window *w, WORD dx, WORD dy)
{
    struct Layer *oldl = w->WLayer; /* Get old layer */
    struct Layer *newl; /* New layer */
    struct Layer_Info *li = &w->WScreen->LayerInfo;
    struct BitMap *bm = w->RPort->BitMap;
    struct Screen *s = w->WScreen;

    struct Rectangle *rect = &oldl->bounds;
    ULONG lock;

    if ((rect->MinX + dx < 0) || (rect->MaxX + dx >= s->Width) || (rect->MinY + dy < 0) || (rect->MaxY + dy >= s->Height))
        return;

    lock = LockIBase(0);

    /* Create layer in new position */
    if (newl = CreateBehindHookLayer(li, bm, rect->MinX + dx, rect->MinY + dy, rect->MaxX + dx, rect->MaxY + dy, LAYERSIMPLE, LAYERS_NOBACKFILL, NULL))
    {
        /* Attach new layer */
        w->WLayer = newl;
        w->RPort = newl->rp;

        /* Attach window */
        newl->Window = w;

        /* Change position */
        w->LeftEdge += dx;
        w->TopEdge += dy;

        MoveLayerInFrontOf(newl, oldl->back);

        /* Delete old layer */
        DeleteLayer(0, oldl);
    }

    UnlockIBase(lock);
}

void initImages(struct Image img[], struct BitMap *gfx)
{
    cutImage(img + IMG_BUTTON, gfx, 0, 0, 64, 16);
    cutImage(img + IMG_PRESSED, gfx, 80, 0, 64, 16);
}

void freeImages(struct Image img[])
{
    WORD i;

    for (i = 0; i < IMG_COUNT; i++)
    {
        freeImage(img + i);
    }
}

void initText(struct IntuiText *text, STRPTR name)
{
    text->LeftEdge = 4;
    text->TopEdge = 4;
    text->NextText = NULL;
    text->FrontPen = 4;
    text->BackPen = 0;
    text->DrawMode = JAM1;
    text->ITextFont = NULL;
    text->IText = name;
}

void initButton(struct Gadget *gad, struct IntuiText *text, WORD gid, WORD x, WORD y, struct Image *render, struct Image *select)
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

void initButtons(struct Gadget gad[], struct Image img[], struct IntuiText text[])
{
    WORD i;

    initButton(gad + GID_MENU1, text + GID_MENU1, GID_MENU1, 0, 240, img + IMG_BUTTON, img + IMG_PRESSED);
    initButton(gad + GID_MENU2, text + GID_MENU2, GID_MENU2, 64, 240, img + IMG_BUTTON, img + IMG_PRESSED);
    initButton(gad + GID_MENU3, text + GID_MENU3, GID_MENU3, 128, 240, img + IMG_BUTTON, img + IMG_PRESSED);
    initButton(gad + GID_MENU4, text + GID_MENU4, GID_MENU4, 192, 240, img + IMG_BUTTON, img + IMG_PRESSED);
    initButton(gad + GID_MENU5, text + GID_MENU5, GID_MENU5, 256, 240, img + IMG_BUTTON, img + IMG_PRESSED);

    for (i = 0; i < GID_COUNT - 1; i++)
    {
        gad[i].NextGadget = gad + i + 1;
    }
}

void initTexts(struct IntuiText text[])
{
    initText(text + GID_MENU1, "Magazyn");
    initText(text + GID_MENU2, "Edytor");
    initText(text + GID_MENU3, "Kafelek");
    initText(text + GID_MENU4, "Opcje");
    initText(text + GID_MENU5, "Ustawienia");
}

void initWindow(struct windowInfo *wi, struct BitMap *gfx)
{
    initImages(wi->img, gfx);
    initTexts(wi->text);
    initButtons(wi->gads, wi->img, wi->text);
}

void freeWindow(struct windowInfo *wi)
{
    freeImages(wi->img);
}
