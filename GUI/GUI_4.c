
/* Graphics User Interface */

#include <stdio.h>
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/gadtools_protos.h>

#define IDCMP_FLAGS IDCMP_RAWKEY|IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE

extern __far struct Custom custom;

enum
{
    IID_CLOSE,
    IID_CLOSE_SEL,
    IID_DEPTH,
    IID_DEPTH_SEL,
    IID_SDRAG,
    IID_IMAGES
};

enum
{
    TID_FLOOR,
    TID_WALL,
    TID_BOX,
    TID_COINS,
    TID_HERO,
    TID_SKULL,
    TID_PLACE,
    TID_FRUITS,
    TID_TILES
};

enum
{
    GID_CLOSE,
    GID_DEPTH,
    GID_SDRAG,
    GID_GADGETS
};

struct imgSource
{
    WORD left, top, width, height;
} imgSources[IID_IMAGES] =
{
    {  0, 0, 16, 16 },
    { 16, 0, 16, 16 },
    { 32, 0, 16, 16 },
    { 48, 0, 16, 16 },
    { 0, 32, 288, 16 }
}, tileSources[TID_TILES] =
{
    { 0, 16, 16, 16 },
    { 16, 16, 16, 16 },
    { 32, 16, 16, 16 },
    { 48, 16, 16, 16 },
    { 64, 16, 16, 16 },
    { 80, 16, 16, 16 },
    { 96, 16, 16, 16 },
    { 112, 16, 16, 16 }
};

struct GUI
{
    struct TextFont *tf;
    struct Screen *s;
    struct Window *w;
    Object        *o;
    struct BitMap *gfx;
    struct Image  images[IID_IMAGES], tiles[TID_TILES];
    struct Gadget gadgets[GID_GADGETS];
    WORD curTile;
    BOOL paint;
};

/* Init empty image of given size and color */

void emptyImage(struct Image *img, WORD width, WORD height, UBYTE color)
{
    img->ImageData  = NULL;
    img->LeftEdge   = 0;
    img->TopEdge    = 0;
    img->Width      = width;
    img->Height     = height;
    img->Depth      = 0;
    img->PlanePick  = 0;
    img->PlaneOnOff = color;
    img->NextImage  = NULL;
}

void initBitMap(struct BitMap *aux, PLANEPTR gfx, WORD width, WORD height, WORD depth)
{
    WORD plane;
    LONG planesize = RASSIZE(width, height);
    InitBitMap(aux, depth, width, height);

    for (plane = 0; plane < depth; plane++)
    {
        aux->Planes[plane] = gfx;
        gfx += planesize;
    }
}

/* Init Image from part of a bitmap */

BOOL cutImage(struct Image *img, struct BitMap *bm, WORD left, WORD top, WORD width, WORD height)
{
    UBYTE depth = GetBitMapAttr(bm, BMA_DEPTH);

    if (depth > 8)
    {
        printf("Current version doesn't support more than 8-bit graphics!\n");
        return(FALSE);
    }

    LONG planesize = RASSIZE(width, height);
    LONG size = planesize * depth;

    img->LeftEdge   = 0;
    img->TopEdge    = 0;
    img->Width      = width;
    img->Height     = height;
    img->Depth      = depth;
    img->PlanePick  = ~(0xff << depth);
    img->PlaneOnOff = 0x00;
    img->NextImage  = NULL;

    if (img->ImageData = AllocMem(size, MEMF_CHIP | MEMF_CLEAR))
    {
        struct BitMap aux;
        PLANEPTR gfx = (PLANEPTR)img->ImageData;

        initBitMap(&aux, gfx, width, height, depth);
        BltBitMap(bm, left, top, &aux, 0, 0, width, height, 0xc0, 0xff, NULL);

        return(TRUE);
    }
    return(FALSE);
}

void freeImage(struct Image *img)
{
    LONG planesize = RASSIZE(img->Width, img->Height);
    LONG size = planesize * img->Depth;

    FreeMem(img->ImageData, size);
}

/* Open game screen */

struct Screen *openScreen(UBYTE depth, ULONG modeID, struct TextAttr *ta)
{
    struct Screen *s;

    printf("Screen information:\n");
    printf("ModeID = 0x%X\n", modeID);
    printf("Depth  = %d\n", depth);

    if (s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       STDSCREENWIDTH,
        SA_Height,      STDSCREENHEIGHT,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        SA_Font,        ta,
        SA_SharePens,   TRUE,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       "Magazyn",
        TAG_DONE))
    {
        return(s);
    }
    else
    {
        printf("Couldn't open screen!\n");
    }

    return(NULL);
}

/* Get screen depth */

UBYTE getScreenDepth(struct Screen *s)
{
    struct DrawInfo *di;

    if (di = GetScreenDrawInfo(s))
    {
        UBYTE depth = di->dri_Depth;
        FreeScreenDrawInfo(s, di);
        return(depth);
    }
    return(0);
}

/* Load picture color palette */

void loadColors(struct Screen *s, Object *o)
{
    struct ColorMap *cm = s->ViewPort.ColorMap;
    ULONG *colorRegs, colorCount;
    WORD color;

    GetDTAttrs(o,
        PDTA_CRegs,     &colorRegs,
        PDTA_NumColors, &colorCount,
        TAG_DONE);

    for (color = 0; color < colorCount; color++)
    {
        SetRGB32CM(cm, color, colorRegs[0], colorRegs[1], colorRegs[2]);
        colorRegs += 3;
    }
    MakeScreen(s);
    RethinkDisplay();
}

/* Load graphics */

Object *loadGraphics(struct Screen *s, STRPTR name)
{
    Object *o;
    UBYTE depth = getScreenDepth(s);
    BOOL remap;

    if (depth == 0)
    {
        printf("Couldn't obtain screen depth!\n");
        return(NULL);
    }
    printf("Screen depth = %d\n", depth);

    remap = depth > 8;

    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    s,
        PDTA_Remap,     remap,
        TAG_DONE))
    {
        if (!remap)
            loadColors(s, o);

        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        return(o);
    }
    else
    {
        printf("Couldn't load %s picture!\n", name);
    }
    return(NULL);
}

void freeGraphics(Object *o)
{
    DisposeDTObject(o);
}

/* Open full-screen window */

struct Window *openWindow(struct Screen *s, ULONG idcmp, struct Gadget *glist)
{
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
        WA_IDCMP,           idcmp,
        WA_ReportMouse,     TRUE,
        WA_Gadgets,         glist,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

void freeImages(struct Image *img, WORD count)
{
    WORD i;

    for (i = 0; i < count; i++)
    {
        freeImage(img + i);
    }
}

/* Init basic images */

BOOL initImages(struct Image *img, struct BitMap *bm, struct imgSource *imgSources, WORD count)
{
    WORD i;

    for (i = 0; i < count; i++)
    {
        struct imgSource *is = imgSources + i;
        if (!cutImage(img + i, bm, is->left, is->top, is->width, is->height))
        {
            freeImages(img, i);
            return(FALSE);
        }
    }
    return(TRUE);
}

struct Gadget *initGadgets(struct Gadget *glist, struct Image *img)
{
    struct Gadget *prev = NULL;
    struct Gadget *gad = glist + GID_CLOSE;

    gad->NextGadget = prev;
    gad->LeftEdge   = 0;
    gad->TopEdge    = 0;
    gad->Width      = 16;
    gad->Height     = 16;
    gad->Flags      = GFLG_GADGIMAGE | GFLG_GADGHIMAGE;
    gad->Activation = GACT_RELVERIFY;
    gad->GadgetType = GTYP_CLOSE;
    gad->GadgetText = NULL;
    gad->GadgetRender = img + IID_CLOSE;
    gad->SelectRender = img + IID_CLOSE_SEL;
    gad->GadgetID   = GID_CLOSE;
    gad->UserData   = NULL;
    gad->SpecialInfo = NULL;
    gad->MutualExclude = 0;

    prev = gad++;

    gad->NextGadget = prev;
    gad->LeftEdge   = 304;
    gad->TopEdge    = 0;
    gad->Width      = 16;
    gad->Height     = 16;
    gad->Flags      = GFLG_GADGIMAGE | GFLG_GADGHIMAGE;
    gad->Activation = GACT_RELVERIFY;
    gad->GadgetType = GTYP_SDEPTH;
    gad->GadgetText = NULL;
    gad->GadgetRender = img + IID_DEPTH;
    gad->SelectRender = img + IID_DEPTH_SEL;
    gad->GadgetID   = GID_DEPTH;
    gad->UserData   = NULL;
    gad->SpecialInfo = NULL;
    gad->MutualExclude = 0;

    prev = gad++;

    gad->NextGadget = prev;
    gad->LeftEdge   = 16;
    gad->TopEdge    = 0;
    gad->Width      = 288;
    gad->Height     = 16;
    gad->Flags      = GFLG_GADGIMAGE | GFLG_GADGHNONE;
    gad->Activation = GACT_RELVERIFY;
    gad->GadgetType = GTYP_SDRAGGING;
    gad->GadgetText = NULL;
    gad->GadgetRender = img + IID_SDRAG;
    gad->SelectRender = NULL;
    gad->GadgetID   = GID_SDRAG;
    gad->UserData   = NULL;
    gad->SpecialInfo = NULL;
    gad->MutualExclude = 0;

    return(gad);
}

/* Construct GUI */

BOOL constructGUI(struct GUI *gui, struct TextAttr *ta)
{
    if (gui->tf = OpenDiskFont(ta))
    {
        if (gui->s = openScreen(5, LORES_KEY, ta))
        {
            if (gui->o = loadGraphics(gui->s, "Dane/Magazyn.pic"))
            {
                GetDTAttrs(gui->o, PDTA_BitMap, &gui->gfx, TAG_DONE);
                if (initImages(gui->images, gui->gfx, imgSources, IID_IMAGES))
                {
                    if (initImages(gui->tiles, gui->gfx, tileSources, TID_TILES))
                    {
                        struct Gadget *glist;
                        glist = initGadgets(gui->gadgets, gui->images);
                        if (gui->w = openWindow(gui->s, IDCMP_FLAGS, glist))
                        {
                            return(TRUE);
                        }
                        freeImages(gui->tiles, TID_TILES);
                    }
                    freeImages(gui->images, IID_IMAGES);
                }
                freeGraphics(gui->o);
            }
            CloseScreen(gui->s);
        }
        CloseFont(gui->tf);
    }
    return(FALSE);
}

void freeGUI(struct GUI *gui)
{
    CloseWindow(gui->w);
    freeImages(gui->tiles, TID_TILES);
    freeImages(gui->images, IID_IMAGES);
    freeGraphics(gui->o);
    CloseScreen(gui->s);
    CloseFont(gui->tf);
}

BOOL userMenu(struct GUI *gui, WORD mx, WORD my)
{
    struct Window *w;
    struct Requester req;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    gui->s,
        WA_Left,            mx,
        WA_Top,             my,
        WA_Width,           64 + 2,
        WA_Height,          32 + 18,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_MOUSEBUTTONS,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_BACKFILL,
        TAG_DONE))
    {
        WORD i;
        BOOL done = FALSE;

        BltBitMapRastPort(gui->gfx, 128, 16, w->RPort, 1, 0, w->Width - 2, 16, 0xc0);

        for (i = 0; i < TID_TILES; i++)
        {
            DrawImage(w->RPort, gui->tiles + i, ((i % 4) << 4) + 1, ((i / 4) << 4) + 17);
        }

        InitRequester(&req);
        Request(&req, gui->w);
        SetWindowPointer(gui->w, WA_BusyPointer, TRUE, TAG_DONE);

        while (!done)
        {
            struct IntuiMessage *msg;

            WaitPort(w->UserPort);
            while (msg = GT_GetIMsg(w->UserPort))
            {
                if (msg->Class == IDCMP_MOUSEBUTTONS)
                    if (msg->Code == IECODE_LBUTTON)
                    {
                        WORD cx = msg->MouseX - 1;
                        WORD cy = msg->MouseY - 17;
                        if (cy >= 0)
                        {
                            gui->curTile = ((cy >> 4) << 2) + (cx >> 4);
                            if (gui->curTile >= TID_TILES)
                                gui->curTile = 0;
                        }
                        done = TRUE;
                    }

                GT_ReplyIMsg(msg);
            }
        }

        SetWindowPointer(gui->w, WA_BusyPointer, FALSE, TAG_DONE);

        EndRequest(&req, gui->w);
        CloseWindow(w);
        return(TRUE);
    }
    return(FALSE);
}

BOOL drawTile(struct GUI *gui, WORD mx, WORD my, ULONG class, UWORD code)
{
    struct Image *img = gui->tiles + gui->curTile;
    LONG planesize = RASSIZE(img->Width, img->Height);
    struct BitMap *bm = gui->w->RPort->BitMap;
    WORD i;
    WORD wordWidth = (img->Width + 15) >> 4;
    PLANEPTR gfx = (PLANEPTR)img->ImageData;

    if (my < 16)
        return(FALSE);

    mx &= 0xfff0;
    my &= 0xfff0;

    OwnBlitter();

    struct Custom *cust = &custom;

    for (i = 0; i < bm->Depth; i++)
    {
        WaitBlit();
        cust->bltcon0 = 0x09f0;
        cust->bltcon1 = 0;
        cust->bltapt = gfx;
        gfx += planesize;
        cust->bltdpt = bm->Planes[i] + (my * bm->BytesPerRow) + (mx >> 3);
        cust->bltamod = 0;
        cust->bltdmod = bm->BytesPerRow - (wordWidth << 1);
        cust->bltafwm = 0xffff;
        cust->bltalwm = 0xffff;
        cust->bltsize = (img->Height << 6) | wordWidth;

    }

    DisownBlitter();
}

void drawBoard(struct GUI *gui)
{
    WORD x, y;
    struct Image *img = gui->tiles + gui->curTile;

    for (y = 1; y < 16; y++)
    {
        for (x = 0; x < 20; x++)
        {
            drawTile(gui, x << 4, y << 4, IDCMP_MOUSEBUTTONS, IECODE_LBUTTON);
        }
        if (y == 7)
            WaitTOF();
    }
}

int main()
{
    struct GUI gui;
    struct TextAttr ta =
    {
        "tny.font",
        8,
        FS_NORMAL,
        FPF_DISKFONT | FPF_DESIGNED
    };

    if (constructGUI(&gui, &ta))
    {
        BOOL done = FALSE;

        gui.paint = FALSE;
        gui.curTile = TID_FLOOR;

        drawBoard(&gui);

        gui.curTile = TID_WALL;

        while (!done)
        {
            struct IntuiMessage *msg;
            WaitPort(gui.w->UserPort);

            while (msg = GT_GetIMsg(gui.w->UserPort))
            {
                ULONG class = msg->Class;
                UWORD code = msg->Code;
                WORD mx = msg->MouseX;
                WORD my = msg->MouseY;
                struct Gadget *gad = (struct Gadget *)msg->IAddress;
                GT_ReplyIMsg(msg);

                if (class == IDCMP_CLOSEWINDOW)
                    done = TRUE;
                else if (class == IDCMP_RAWKEY)
                {
                    if (code == 0x45)
                    {
                        done = TRUE;
                    }
                }
                else if (class == IDCMP_MOUSEBUTTONS)
                {
                    if (code == IECODE_RBUTTON)
                        userMenu(&gui, mx, my);
                    else if (code == IECODE_LBUTTON)
                    {
                        drawTile(&gui, mx, my, class, code);
                        gui.paint = TRUE;
                    }
                    else if (code == (IECODE_LBUTTON|IECODE_UP_PREFIX))
                    {
                        gui.paint = FALSE;
                    }
                }
                else if (class == IDCMP_MOUSEMOVE)
                {
                    if (gui.paint)
                        drawTile(&gui, mx, my, class, code);
                }
            }
        }
        freeGUI(&gui);
    }
    return(0);
}
