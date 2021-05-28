
/* Window.c: Auxilliary window functions */

#include <stdio.h>
#include <assert.h>

#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>

#include "Window.h"
#include "Tile.h"

STRPTR itemStrings[] =
{
    "Opcje gry",
    "Nowa gra",
    "Wczytaj grë",
    "Lista poziomów",
    "Punktacja",
    "Autor",
    "Wyjôcie",
    "Rozpocznij grë",
    "Edytuj poziom",
    "Poprzedni",
    "Nastëpny",
    "Wyjdú do menu"
};

struct Window *openBDWindow(struct Screen *s)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            64,
        WA_Top,             64,
        WA_Width,           192,
        WA_Height,          128,
        WA_Backdrop,        FALSE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_GADGETUP|IDCMP_REFRESHWINDOW|IDCMP_RAWKEY,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

struct Window *openReqWindow(struct Window *p)
{
    struct Screen *s = p->WScreen;
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Width,
        WA_Height,          s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           0,
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

BOOL initImages(struct Image img[], struct BitMap *gfx)
{
    if (cutImage(img + IMG_BUTTON, gfx, 0, 64, 80, 16))
    {
        if (cutImage(img + IMG_PRESSED, gfx, 0, 80, 80, 16))
        {
            if (cutImage(img + IMG_CLOSE, gfx, 0, 16, 16, 16))
            {
                if (cutImage(img + IMG_CLOSE_PRESSED, gfx, 16, 16, 16, 16))
                {
                    if (cutImage(img + IMG_NEXT, gfx, 64, 16, 16, 16))
                    {
                        if (cutImage(img + IMG_NEXT_PRESSED, gfx, 80, 16, 16, 16))
                        {
                            if (cutImage(img + IMG_PREV, gfx, 96, 16, 16, 16))
                            {
                                if (cutImage(img + IMG_PREV_PRESSED, gfx, 112, 16, 16, 16))
                                {
                                    return(TRUE);
                                }
                                freeImage(img + IMG_PREV);
                            }
                            freeImage(img + IMG_NEXT_PRESSED);
                        }
                        freeImage(img + IMG_NEXT);
                    }
                    freeImage(img + IMG_CLOSE_PRESSED);
                }
                freeImage(img + IMG_CLOSE);
            }
            freeImage(img + IMG_PRESSED);
        }
        freeImage(img + IMG_BUTTON);
    }
    return(FALSE);
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
    text->FrontPen = 10;
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

    gad[GID_MENU5].Activation |= GACT_TOGGLESELECT;

    initButton(gad + GID_CLOSE, NULL, GID_CLOSE, 0, 0, img + IMG_CLOSE, img + IMG_CLOSE_PRESSED);

    for (i = 0; i < GID_COUNT - 1; i++)
    {
        gad[i].NextGadget = gad + i + 1;
    }
}

void initReqButtons(struct Gadget gad[], struct Image img[], struct IntuiText text[])
{
    WORD i;

    initButton(gad + RID_CLOSE, NULL, RID_CLOSE, 0, 0, img + IMG_CLOSE, img + IMG_CLOSE_PRESSED);
    initButton(gad + RID_OPT1, text + RID_OPT1, RID_OPT1, 48, 112, img + IMG_BUTTON, img + IMG_PRESSED);
    initButton(gad + RID_OPT2, text + RID_OPT2, RID_OPT2, 128, 112, img + IMG_BUTTON, img + IMG_PRESSED);

    for (i = 0; i < RID_COUNT - 1; i++)
    {
        gad[i].NextGadget = gad + i + 1;
    }
}

void initTexts(struct IntuiText text[])
{
    initText(text + GID_MENU1, "Nowa gra");
    initText(text + GID_MENU2, "Wczytaj grë");
    initText(text + GID_MENU3, "Restartuj");
    initText(text + GID_MENU4, "Poziom");
    initText(text + GID_MENU5, "Opcje edycji");
}

void initEditTexts(struct IntuiText text[])
{
    initText(text + GID_MENU1, "Kafelek");
    initText(text + GID_MENU2, "Odtwórz");
    initText(text + GID_MENU3, "Zapisz");
    initText(text + GID_MENU4, "Poziom");
    initText(text + GID_MENU5, "Opcje edycji");
}

void initMenuText(struct IntuiText *intuiText, STRPTR string)
{
    initText(intuiText, string);
}

void initMenuButton(struct Gadget *gad, struct IntuiText *text, WORD id, WORD x, WORD y, struct Image *img)
{
    initButton(gad, text, id, x, y, img + IMG_BUTTON, img + IMG_PRESSED);
}

void initReqTexts(struct IntuiText text[], STRPTR opt1, STRPTR opt2)
{
    initText(text + RID_OPT1, opt1);
    initText(text + RID_OPT2, opt2);
}

BOOL initWindow(struct windowInfo *wi, struct BitMap *gfx)
{
    if (initImages(wi->img, gfx))
    {
        initTexts(wi->text);
        initButtons(wi->gads, wi->img, wi->text);
        return(TRUE);
    }
    return(FALSE);
}

void freeWindow(struct windowInfo *wi)
{
    freeImages(wi->img);
}

void initReq(struct reqInfo *ri, struct windowInfo *wi, STRPTR opt1, STRPTR opt2)
{
    ri->img = wi->img;
    initReqTexts(ri->text, opt1, opt2);
    initReqButtons(ri->gads, ri->img, ri->text);
}
