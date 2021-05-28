
/* Game launcher */

#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>

#include <clib/diskfont_protos.h>
#include <clib/asl_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>

#define FONT_INTERVAL   6
#define LEFT_MARGIN     16
#define TOP_MARGIN      8
#define GAD_WIDTH       160
#define WINDOW_WIDTH    320
#define VERT_INTERVAL   4
#define HORIZ_INTERVAL  8

enum
{
    RESULT_ERROR,
    RESULT_CLOSE,
    RESULT_JUMP,
    RESULT_NEW,
    RESULT_RUN
};

enum
{
    GID_NEXT_SCREEN,
    GID_OPEN_SCREEN,
    GID_LAUNCH
};

/* It's a window that open itself on any public screen. */
/* It may "jump" between screens and also create new public screen */
/* It's used to open game on desired resolution and colors */

struct Gadget *createGadgets(struct Screen *pubs, struct VisualInfo *vi, struct DrawInfo *di, WORD *height, BOOL opened)
{
    struct Gadget *prev, *glist;
    struct NewGadget ng;
    struct TextFont *font = pubs->RastPort.Font;
    static struct TextAttr ta;
    WORD gad_height = font->tf_YSize + FONT_INTERVAL;

    AskFont(&pubs->RastPort, &ta);

    ng.ng_VisualInfo    = vi;
    ng.ng_TextAttr      = &ta;
    ng.ng_LeftEdge      = pubs->WBorLeft + LEFT_MARGIN;
    ng.ng_TopEdge       = pubs->WBorTop + font->tf_YSize + 1 + TOP_MARGIN;
    ng.ng_Width         = GAD_WIDTH;
    ng.ng_Height        = gad_height;

    ng.ng_GadgetText    = "Next screen";
    ng.ng_GadgetID      = GID_NEXT_SCREEN;
    ng.ng_UserData      = NULL;
    ng.ng_Flags         = PLACETEXT_IN;

    prev = CreateContext(&glist);

    prev = CreateGadget(BUTTON_KIND, prev, &ng, TAG_DONE);

    ng.ng_TopEdge       += ng.ng_Height + VERT_INTERVAL;
    ng.ng_GadgetText    = "Launch here";
    ng.ng_GadgetID      = GID_LAUNCH;

    prev = CreateGadget(BUTTON_KIND, prev, &ng, TAG_DONE);

    ng.ng_TopEdge       += ng.ng_Height + VERT_INTERVAL;
    ng.ng_GadgetText    = "Open new screen...";
    ng.ng_GadgetID      = GID_OPEN_SCREEN;

    prev = CreateGadget(BUTTON_KIND, prev, &ng,
        GA_Disabled, opened,
        TAG_DONE);

    *height = ng.ng_TopEdge + ng.ng_Height + TOP_MARGIN + pubs->WBorBottom;

    return(glist);
}

BOOL handleLauncher(struct Window *w)
{
    struct IntuiMessage *msg;

    WaitPort(w->UserPort);

    while (msg = GT_GetIMsg(w->UserPort))
    {
        ULONG class = msg->Class;
        UWORD code  = msg->Code;
        WORD  mx    = msg->MouseX;
        WORD  my    = msg->MouseY;
        APTR  iaddr = msg->IAddress;

        GT_ReplyIMsg(msg);

        if (class == IDCMP_CLOSEWINDOW)
        {
            return(RESULT_CLOSE);
        }
        else if (class == IDCMP_GADGETUP)
        {
            struct Gadget *gad = (struct Gadget *)iaddr;
            if (gad->GadgetID == GID_NEXT_SCREEN)
            {
                return(RESULT_JUMP);
            }
            else if (gad->GadgetID == GID_OPEN_SCREEN)
            {
                return(RESULT_NEW);
            }
            else if (gad->GadgetID == GID_LAUNCH)
            {
                return(RESULT_RUN);
            }
        }
    }
    return(RESULT_CLOSE);
}

/* Open launcher window on a given public screen */
LONG openLauncher(STRPTR name, WORD *left, WORD *top, BOOL opened)
{
    struct Screen *pubs;
    LONG result = RESULT_ERROR;

    if (!(pubs = LockPubScreen(name)))
        printf("Couldn't lock %s!\n", name);
    else
    {
        struct VisualInfo *vi;

        if (!(vi = GetVisualInfoA(pubs, NULL)))
            printf("Couldn't get visual info!\n");
        else
        {
            struct DrawInfo *di;

            /* Read screen DrawInfo */
            if (!(di = GetScreenDrawInfo(pubs)))
                printf("Couldn't get draw info!\n");
            else
            {
                struct Gadget *glist;
                struct Window *w;
                WORD height;

                /* Create gadgets */
                glist = createGadgets(pubs, vi, di, &height, opened);

                if (!(w = OpenWindowTags(NULL,
                    WA_PubScreen,   pubs,
                    WA_Left,        0,
                    WA_Top,         pubs->BarHeight + 1,
                    WA_Width,       WINDOW_WIDTH,
                    WA_Height,      height,
                    WA_DragBar,     TRUE,
                    WA_CloseGadget, TRUE,
                    WA_DepthGadget, TRUE,
                    WA_Activate,    TRUE,
                    WA_Gadgets,     glist,
                    WA_Title,       "Warehouse Launcher",
                    WA_ScreenTitle, "Warehouse Launcher",
                    WA_IDCMP,       IDCMP_CLOSEWINDOW|BUTTONIDCMP,
                    TAG_DONE)))
                    printf("Couldn't open window!\n");
                else
                {
                    result = handleLauncher(w);
                    *left = w->LeftEdge;
                    *top  = w->TopEdge;
                    CloseWindow(w);
                }
                FreeGadgets(glist);
                FreeScreenDrawInfo(pubs, di);
            }
            FreeVisualInfo(vi);
        }
        UnlockPubScreen(NULL, pubs);
    }
    return(result);
}

int main()
{
    BYTE buffer[MAXPUBSCREENNAME + 1];
    STRPTR name = NULL;
    LONG result;
    WORD left = 100, top = 50;
    struct Screen *cust = NULL;
    WORD depth = 5;
    UWORD pens[] = { ~0 };
    ULONG oldModes;
    struct TextFont *font = NULL;
    struct TextAttr ta;
    struct FontRequester *fr;

    oldModes = SetPubScreenModes(POPPUBSCREEN);

    while ((result = openLauncher(name, &left, &top, cust != NULL)) == RESULT_JUMP || result == RESULT_NEW)
    {
        struct Screen *s;

        if (result == RESULT_JUMP)
        {
            if (s = LockPubScreen(name))
            {
                name = NextPubScreen(s, buffer);
                UnlockPubScreen(NULL, s);
            }
        }
        else if (result == RESULT_NEW)
        {
            if (fr = AllocAslRequestTags(ASL_FontRequest, TAG_DONE))
            {
                if (AslRequestTags(fr, TAG_DONE))
                {
                    struct ScreenModeRequester *smr;

                    ta = fr->fo_Attr;

                    font = OpenDiskFont(&ta);

                    cust = OpenScreenTags(NULL,
                        SA_Font,            &ta,
                        SA_Left,            0,
                        SA_Top,             0,
                        SA_Width,           320,
                        SA_Height,          256,
                        SA_Depth,           depth,
                        SA_DisplayID,       LORES_KEY,
                        SA_Pens,            pens,
                        SA_SharePens,       TRUE,
                        SA_Title,           "Warehouse Public Screen",
                        SA_PubName,         "WAREHOUSE.1",
                        TAG_DONE);

                    if (cust)
                    {
                        name = "WAREHOUSE.1";
                        PubScreenStatus(cust, ~PSNF_PRIVATE);
                    }
                }
            }
        }
    }
    if (cust)
    {
        CloseScreen(cust);
    }
    if (font)
    {
        CloseFont(font);
    }
    if (fr)
    {
        FreeAslRequest(fr);
    }
    if (result == RESULT_ERROR)
    {
        printf("Error encountered.\n");
    }
    return(0);
}
