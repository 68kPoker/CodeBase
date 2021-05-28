
#include <stdio.h>
#include "debug.h"

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>

#include "WinGad.h"
#include "WinGad_protos.h"

extern ULONG HookEntry();

/* An Image with BitMap source and Selected and Disabled states */

struct myImage
{
    struct BitMap *bm;
    Point point[2];
};

__saveds ULONG dispatchMyImage(Class *cl, Object *o, Msg msg)
{
    struct myImage *data;
    APTR retval = NULL;

    switch (msg->MethodID)
    {
        case OM_NEW:
            if (retval = (APTR)DoSuperMethodA(cl, o, msg))
            {
                struct TagItem *tag, *taglist = ((struct opSet *)msg)->ops_AttrList;
                data = (struct myImage *)INST_DATA(cl, retval);

                while (tag = NextTagItem(&taglist))
                {
                    switch (tag->ti_Tag)
                    {
                        case IA_BitMap:
                            data->bm = (struct BitMap *)tag->ti_Data;
                            break;
                        case IA_Points:
                            Point *p = (Point *)tag->ti_Data;
                            data->point[IDS_NORMAL] = *p++;
                            data->point[IDS_SELECTED] = *p;
                            break;
                    }
                }
            }
            break;
        case IM_DRAW:
            struct impDraw *id = (struct impDraw *)msg;
            data = (struct myImage *)INST_DATA(cl, o);
            WORD state = id->imp_State;

            switch (state)
            {
                case IDS_NORMAL:
                case IDS_INACTIVENORMAL:
                    BltBitMapRastPort(data->bm, data->point[0].x, data->point[0].y, id->imp_RPort, id->imp_Offset.X + ((struct Image *)o)->LeftEdge, id->imp_Offset.Y + ((struct Image *)o)->TopEdge, ((struct Image *)o)->Width, ((struct Image *)o)->Height, 0xc0);
                    break;
                case IDS_SELECTED:
                case IDS_INACTIVESELECTED:
                    BltBitMapRastPort(data->bm, data->point[1].x, data->point[1].y, id->imp_RPort, id->imp_Offset.X + ((struct Image *)o)->LeftEdge, id->imp_Offset.Y + ((struct Image *)o)->TopEdge, ((struct Image *)o)->Width, ((struct Image *)o)->Height, 0xc0);
                    break;
            }
            break;
        default:
            retval = (APTR)DoSuperMethodA(cl, o, msg);
            break;
    }
    return((ULONG)retval);
}

Class *makeImage()
{
    Class *class;

    if (class = MakeClass(NULL, "imageclass", NULL, sizeof(struct myImage), 0))
    {
        /* Class made */

        /* Put dispatcher */
        class->cl_Dispatcher.h_Entry = HookEntry;
        class->cl_Dispatcher.h_SubEntry = (HOOKFUNC)dispatchMyImage;

        return(class);
    }
    return(NULL);
}

/* initImages: Init BOOPSI images */
BOOL initImages(Object *img[], Class *cl, struct BitMap *gfx)
{
    WORD i;
    Point points[][2] = {
        { 0, 0, 0, 16 },
        { 16, 0, 16, 16 },
        { 32, 0, 32, 16 },
        { 112, 0, 112, 16 },
        { 192, 0, 192, 16 },
        { 0, 176, 0, 176 },
        { 0, 48, 0, 48 },
        { 80, 48, 80, 48 },
        { 80, 64, 80, 64 },
        { 80, 80, 80, 80 }
    };
    WORD widths[] = { 16, 16, 80, 80, 80, 64, 80, 80, 80, 80 };

    D(bug("initImages:\n"));

    for (i = 0; i < IID_COUNT; i++)
    {
        img[i] = NewObject(cl, NULL,
            IA_BitMap,  gfx,
            IA_Points,  points[i],
            IA_Width,   widths[i],
            IA_Height,  i == IID_TILES ? 64 : 16,
            TAG_DONE);

        if (!img[i])
        {
            freeImages(img, i);
            return(FALSE);
        }
    }
    return(TRUE);
}

void freeImages(Object *img[], WORD count)
{
    WORD i;

    D(bug("freeImages:\n"));
    for (i = count - 1; i >= 0; i--)
    {
        DisposeObject(img[i]);
    }
}

/* Open main menu window */
WIN *openWindow(SCR *scr, Object *img[])
{
    WIN *win;
    WININFO *wi;
    SCRINFO *si = (SCRINFO *)scr->UserData;

    D(bug("openWindow:\n"));

    if (wi = AllocMem(sizeof(*wi), MEMF_PUBLIC))
    {
        /* Alloc basic gadgets */

        if (wi->buttons[GID_CLOSE] = NewObject(NULL, "buttongclass",
            GA_Left,        0,
            GA_Top,         0,
            GA_Image,       img[IID_CLOSE],
            GA_RelVerify,   TRUE,
            GA_Width,       16,
            GA_Height,      16,
            GA_ID,          GID_CLOSE,
            TAG_DONE))
        {
            if (wi->buttons[GID_DEPTH] = NewObject(NULL, "buttongclass",
                GA_Left,        304,
                GA_Top,         0,
                GA_Image,       img[IID_DEPTH],
                GA_RelVerify,   TRUE,
                GA_Width,       16,
                GA_Height,      16,
                GA_ID,          GID_DEPTH,
                GA_Previous,    wi->buttons[GID_CLOSE],
                TAG_DONE))
            {
                if (wi->buttons[GID_PANEL] = NewObject(NULL, "buttongclass",
                    GA_Left,        16,
                    GA_Top,         0,
                    GA_Image,       img[IID_PANEL],
                    GA_RelVerify,   TRUE,
                    GA_Width,       80,
                    GA_Height,      16,
                    GA_ID,          GID_PANEL,
                    GA_Previous,    wi->buttons[GID_DEPTH],
                    TAG_DONE))
                {
                    if (wi->buttons[GID_EDITOR] = NewObject(NULL, "buttongclass",
                        GA_Left,        96,
                        GA_Top,         0,
                        GA_Image,       img[IID_EDITOR],
                        GA_RelVerify,   TRUE,
                        GA_Width,       80,
                        GA_Height,      16,
                        GA_ID,          GID_EDITOR,
                        GA_Previous,    wi->buttons[GID_PANEL],
                        TAG_DONE))
                    {
                        if (wi->buttons[GID_OPTIONS] = NewObject(NULL, "buttongclass",
                            GA_Left,        176,
                            GA_Top,         0,
                            GA_Image,       img[IID_OPTIONS],
                            GA_RelVerify,   TRUE,
                            GA_Width,       80,
                            GA_Height,      16,
                            GA_ID,          GID_OPTIONS,
                            GA_Previous,    wi->buttons[GID_EDITOR],
                            TAG_DONE))
                        {
                            if (win = OpenWindowTags(NULL,
                                WA_CustomScreen,    scr,
                                WA_Left,            0,
                                WA_Top,             0,
                                WA_Width,           scr->Width,
                                WA_Height,          16,
                                WA_Backdrop,        TRUE,
                                WA_Borderless,      TRUE,
                                WA_Activate,        TRUE,
                                WA_RMBTrap,         TRUE,
                                WA_BackFill,        LAYERS_NOBACKFILL,
                                WA_SimpleRefresh,   TRUE,
                                WA_IDCMP,           IDCMP_GADGETUP|IDCMP_REFRESHWINDOW,
                                TAG_DONE))
                            {
                                struct DrawInfo *dri;

                                if (wi->dri = dri = GetScreenDrawInfo(scr))
                                {
                                    struct GadgetInfo gi =
                                    {
                                        scr,
                                        win,
                                        NULL,
                                        win->RPort,
                                        win->WLayer,
                                        { 0 },
                                        { 1, 0 },
                                        dri,
                                    };
                                    AddGList(win, (struct Gadget *)wi->buttons[GID_CLOSE], -1, -1, NULL);
                                    DoGadgetMethod((struct Gadget *)wi->buttons[GID_CLOSE], win, NULL, GM_RENDER, &gi, win->RPort, GREDRAW_REDRAW);
                                    DoGadgetMethod((struct Gadget *)wi->buttons[GID_DEPTH], win, NULL, GM_RENDER, &gi, win->RPort, GREDRAW_REDRAW);
                                    DoGadgetMethod((struct Gadget *)wi->buttons[GID_PANEL], win, NULL, GM_RENDER, &gi, win->RPort, GREDRAW_REDRAW);
                                    DoGadgetMethod((struct Gadget *)wi->buttons[GID_EDITOR], win, NULL, GM_RENDER, &gi, win->RPort, GREDRAW_REDRAW);
                                    DoGadgetMethod((struct Gadget *)wi->buttons[GID_OPTIONS], win, NULL, GM_RENDER, &gi, win->RPort, GREDRAW_REDRAW);

                                    wi->window = win;
                                    win->UserData = (APTR)wi;
                                    return(win);
                                }
                                CloseWindow(win);
                            }
                            DisposeObject(wi->buttons[GID_OPTIONS]);
                        }
                        DisposeObject(wi->buttons[GID_EDITOR]);
                    }
                    DisposeObject(wi->buttons[GID_PANEL]);
                }
                DisposeObject(wi->buttons[GID_DEPTH]);
            }
            DisposeObject(wi->buttons[GID_CLOSE]);
        }
        FreeMem(wi, sizeof(*wi));
    }
    return(NULL);
}

WIN *openBoardWindow(SCR *scr)
{
    WIN *win;

    if (win = OpenWindowTags(NULL,
        WA_CustomScreen,    scr,
        WA_Left,            0,
        WA_Top,             16,
        WA_Width,           320,
        WA_Height,          240,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(win);
    }
    return(NULL);
}

/* Open auxilliary menu window */
WIN *openMenu(SCR *scr, Object *img[], GFX *gfx, WORD type, WORD x, WORD y)
{
    WIN *w;
    MENUINFO *mi;

    D(bug("openMenu:\n"));

    if (mi = AllocMem(sizeof(*mi), MEMF_PUBLIC|MEMF_CLEAR))
    {
        if (mi->buttons[MID_CLOSE] = NewObject(NULL, "buttongclass",
            GA_Left,        0,
            GA_Top,         0,
            GA_Image,       img[IID_CLOSE],
            GA_RelVerify,   TRUE,
            GA_Width,       16,
            GA_Height,      16,
            GA_ID,          MID_CLOSE,
            TAG_DONE))
        {
            BltBitMapRastPort(gfx->bitmap, 198, 48, &scr->RastPort, x, y, 99, 128, 0xc0);
            if (w = OpenWindowTags(NULL,
                WA_CustomScreen,    scr,
                WA_Width,           99,
                WA_Height,          128,
                WA_Left,            x,
                WA_Top,             y,
                WA_Borderless,      TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_BackFill,        LAYERS_NOBACKFILL,
                WA_IDCMP,           IDCMP_GADGETUP,
                WA_Gadgets,         mi->buttons[MID_CLOSE],
                TAG_DONE))
            {
                w->UserData = (APTR)mi;
                mi->window = w;

                return(w);
            }
            DisposeObject(mi->buttons[MID_CLOSE]);
        }
        FreeMem(mi, sizeof(*mi));
    }
    return(NULL);
}

void closeWindow(WIN *win)
{
    WININFO *wi = (WININFO *)win->UserData;

    FreeScreenDrawInfo(win->WScreen, wi->dri);
    CloseWindow(win);
    DisposeObject(wi->buttons[GID_CLOSE]);
    DisposeObject(wi->buttons[GID_DEPTH]);
    DisposeObject(wi->buttons[GID_PANEL]);
    DisposeObject(wi->buttons[GID_EDITOR]);
    DisposeObject(wi->buttons[GID_OPTIONS]);
    FreeMem(wi, sizeof(*wi));
}

void closeMenu(WIN *win)
{
    MENUINFO *mi = (MENUINFO *)win->UserData;

    CloseWindow(win);
    DisposeObject(mi->buttons[MID_CLOSE]);
    FreeMem(mi, sizeof(*mi));
}
