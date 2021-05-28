
#include <stdio.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

#define IDCMP_FLAGS (IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETDOWN)

enum
{
    GID_MENU,
    GID_PALETTE,
    GID_BOARD,
    GADGETS
};

struct windowData
{
    struct Gadget gads[GADGETS];
};

struct Screen *openScreen(ULONG mode, UBYTE depth)
{
    struct Screen *s;

    s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       STDSCREENWIDTH,
        SA_Height,      STDSCREENHEIGHT,
        SA_Depth,       depth,
        SA_DisplayID,   mode,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   FALSE,
        SA_Draggable,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Interleaved, TRUE,
        TAG_DONE);

    /* Print secondary error */
    if (!s) printf("Couldn't open %d-deep screen (modeID = $%X).\n", depth, mode);
    return(s);
}

void closeScreen(struct Screen *s)
{
    CloseScreen(s);
}

/* Open backdrop window */

struct Window *openWindow(struct Screen *s, WORD left, WORD top, WORD width, WORD height)
{
    extern void createGadget(struct Gadget *gad, WORD id, APTR user, WORD left, WORD top, WORD width, WORD height);
    struct Window *w;

    w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            left,
        WA_Top,             top,
        WA_Width,           width,
        WA_Height,          height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_FLAGS,
        WA_ReportMouse,     TRUE,
        TAG_DONE);

    if (!w) printf("Couldn't open window.\n");

    else
    {
        struct windowData *data;
        data = AllocMem(sizeof(*data), MEMF_PUBLIC);

        if (!data) printf("Out of memory.\n");

        else
        {
            createGadget(data->gads + GID_MENU,    GID_MENU,    NULL,  0,  0, 320,  32);
            createGadget(data->gads + GID_PALETTE, GID_PALETTE, NULL,  0, 32,  32, 224);
            createGadget(data->gads + GID_BOARD,   GID_BOARD,   NULL, 32, 32, 288, 224);

            data->gads[GID_MENU   ].NextGadget = data->gads + GID_PALETTE;
            data->gads[GID_PALETTE].NextGadget = data->gads + GID_BOARD;

            w->UserData = (APTR)data;
            return(w);
        }

        CloseWindow(w);
    }

    return(NULL);
}

void closeWindow(struct Window *w)
{
    CloseWindow(w);
}

/* Create gadget */

void createGadget(struct Gadget *gad, WORD id, APTR user, WORD left, WORD top, WORD width, WORD height)
{
    gad->LeftEdge       = left;
    gad->TopEdge        = top;
    gad->Width          = width;
    gad->Height         = height;
    gad->Flags          = GFLG_GADGHNONE;
    gad->Activation     = GACT_IMMEDIATE;
    gad->GadgetType     = GTYP_BOOLGADGET;
    gad->GadgetRender   = NULL;
    gad->SelectRender   = NULL;
    gad->GadgetText     = NULL;
    gad->MutualExclude  = 0;
    gad->SpecialInfo    = NULL;
    gad->GadgetID       = id;
    gad->UserData       = user;
    gad->NextGadget     = NULL;
}
