
/* Initial public screen window - allows to choose game options */

/* Video options: */
/* 1. Resolution + Color depth */
/*    Allows to choose game resolution and colors with ASL requester. */
/*    You may choose HAM8 for 16M colors on AGA */
/*    Minimum colors are 16 */

/* 2. Rendering options */
/*    Allows to enable or disable Blitter, choose Software or Buster  */
/*    Chunky. */

/* 3. Rendering quality */
/*    Shows the bits-per-color (12 or 24). */
/*    Allows to choose high or low animation detail. */

#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <graphics/displayinfo.h>
#include <libraries/asl.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/asl_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#define WIN_WIDTH  500
#define WIN_HEIGHT 180
#define MARGIN 16
#define OUTER 8
#define INTERVAL 4
#define GAD_WIDTH  230
#define SMALL_WIDTH 32

enum
{
    GID_RESOLUTION = 1,
    GID_LAUNCH
};

enum
{
    TID_RESOLUTION,
    TID_GADGETS
};

__saveds BOOL filterFunc(register __a0 struct Hook *hook, register __a1 ULONG modeID, register __a2 struct ScreenModeRequester *smr)
{
    struct DisplayInfo di;

    if (GetDisplayInfoData(NULL, (UBYTE *)&di, sizeof(di), DTAG_DISP, modeID) > 0)
    {
        if ((di.PropertyFlags & (DIPF_IS_HAM|DIPF_IS_WB|DIPF_IS_EXTRAHALFBRITE)) &&
            (di.Resolution.x <= 44 && di.Resolution.y <= 44))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

struct Hook hook =
{
    { NULL, NULL },
    (ULONG(*)()) filterFunc,
    NULL,
    NULL
};

void putModeName(STRPTR modeName, ULONG modeID, WORD width, WORD height, WORD depth, UBYTE *bits)
{
    struct NameInfo ni;
    struct DisplayInfo di;

    GetDisplayInfoData(NULL, (UBYTE *)&ni, sizeof(ni), DTAG_NAME, modeID);
    GetDisplayInfoData(NULL, (UBYTE *)&di, sizeof(di), DTAG_DISP, modeID);

    *bits = di.RedBits + di.GreenBits + di.BlueBits;

    if (di.PropertyFlags & DIPF_IS_HAM)
    {
        sprintf(modeName, "%dx%d (HAM-%d)", width, height, depth);
    }
    else
    {
        sprintf(modeName, "%dx%d (%d-Bits)", width, height, depth);
    }
}

BOOL askResolution(STRPTR modeName, struct Window *w, struct Gadget *textgad, UBYTE *bits, ULONG *modeID, UBYTE *writeDepth)
{
    struct ScreenModeRequester *smr;
    static ULONG activeModeID = LORES_KEY;

    static WORD depth = 5;
    static UWORD width = 320, height = 256;

    if (smr = AllocAslRequestTags(ASL_ScreenModeRequest, TAG_DONE))
    {
        if (AslRequestTags(smr,
            w ? ASLSM_Window : TAG_IGNORE, w,
            w ? ASLSM_SleepWindow : TAG_IGNORE, TRUE,
            ASLSM_DoDepth, TRUE,
            ASLSM_InitialDisplayDepth, depth,
            ASLSM_InitialDisplayID, activeModeID,
            ASLSM_InitialDisplayWidth, width,
            ASLSM_InitialDisplayHeight, height,
            ASLSM_MinDepth, 5,
            ASLSM_TitleText, "Select Game Resolution",
            ASLSM_PropertyMask, 0,
            ASLSM_PropertyFlags, 0,
            /* ASLSM_FilterFunc, &hook, */
            TAG_DONE))
        {
            putModeName(modeName, smr->sm_DisplayID, smr->sm_DisplayWidth, smr->sm_DisplayHeight, smr->sm_DisplayDepth, bits);

            if (textgad)
            {
                GT_SetGadgetAttrs(textgad, w, NULL,
                    GTTX_Text, modeName,
                    TAG_DONE);
            }

            activeModeID = *modeID = smr->sm_DisplayID;
            depth = *writeDepth = smr->sm_DisplayDepth;
            width = smr->sm_DisplayWidth;
            height = smr->sm_DisplayHeight;

            FreeAslRequest(smr);
            return(TRUE);
        }
        FreeAslRequest(smr);
    }
    return(FALSE);
}

BOOL GameOptions(STRPTR pubname)
{
    struct Screen *pubs;
    STRPTR chunky[] =
    {
        "Off",
        "OCS/ECS/AGA",
        "AmigaCD³²/RTG",
        NULL
    }, quality[] =
    {
        "Low Detail",
        "High Detail",
        NULL
    };
    ULONG modeID;

    if (pubs = LockPubScreen(pubname))
    {
        struct VisualInfo *vi;
        struct TextAttr ta;
        static UBYTE modeName[DISPLAYNAMELEN+20];
        UBYTE bits, depth = 5;

        AskFont(&pubs->RastPort, &ta);

        putModeName(modeName, modeID = BestModeID(
            BIDTAG_ViewPort, &pubs->ViewPort,
            BIDTAG_NominalWidth, 320,
            BIDTAG_NominalHeight, 256,
            BIDTAG_Depth, 5,
            TAG_DONE),
            320, 256, 5, &bits);

        if (vi = GetVisualInfoA(pubs, NULL))
        {
            struct Gadget *glist, *prev, *gadgets[TID_GADGETS];
            struct NewGadget ng;
            WORD gad_height = pubs->RastPort.Font->tf_YSize + 8;

            prev = CreateContext(&glist);

            ng.ng_VisualInfo = vi;
            ng.ng_TextAttr = &ta;
            ng.ng_LeftEdge = pubs->WBorLeft + MARGIN;
            ng.ng_TopEdge = pubs->WBorTop + pubs->RastPort.Font->tf_YSize + 1 + MARGIN;
            ng.ng_Width = GAD_WIDTH;
            ng.ng_Height = gad_height;

            ng.ng_GadgetText = "Choose resolution...";
            ng.ng_Flags = PLACETEXT_IN;
            ng.ng_GadgetID = GID_RESOLUTION;
            ng.ng_UserData = NULL;

            prev = CreateGadget(BUTTON_KIND, prev, &ng, TAG_DONE);

            WORD top = ng.ng_TopEdge - 12;

            ng.ng_GadgetID = 0;
            ng.ng_GadgetText = "";
            ng.ng_LeftEdge += ng.ng_Width + 1;

            gadgets[TID_RESOLUTION] = prev = CreateGadget(TEXT_KIND, prev, &ng,
                GTTX_Text,   modeName,
                GTTX_Border, TRUE,
                TAG_DONE);

            ng.ng_GadgetText = "Blitter";
            ng.ng_LeftEdge -= ng.ng_Width + 1;
            ng.ng_TopEdge += ng.ng_Height + INTERVAL;
            ng.ng_Flags = PLACETEXT_RIGHT;

            prev = CreateGadget(CHECKBOX_KIND, prev, &ng,
                GTCB_Checked, TRUE,
                TAG_DONE);

            ng.ng_GadgetText = "Chunky";
            ng.ng_LeftEdge += ng.ng_Width + 1;
            ng.ng_Flags = PLACETEXT_LEFT;

            prev = CreateGadget(CYCLE_KIND, prev, &ng,
                GTCY_Labels, chunky,
                TAG_DONE);

            ng.ng_LeftEdge -= ng.ng_Width + 1;
            ng.ng_TopEdge += ng.ng_Height + INTERVAL;

            ng.ng_GadgetText = "Double-buffering";
            ng.ng_Flags = PLACETEXT_RIGHT;

            prev = CreateGadget(CHECKBOX_KIND, prev, &ng,
                GTCB_Checked, TRUE,
                TAG_DONE);

            ng.ng_TopEdge += ng.ng_Height + INTERVAL;

            ng.ng_GadgetText = "Color Bits";
            ng.ng_Width = SMALL_WIDTH;

            prev = CreateGadget(NUMBER_KIND, prev, &ng,
                GTNM_Number, bits,
                GTNM_Border, TRUE,
                TAG_DONE);

            ng.ng_LeftEdge += GAD_WIDTH + 1;
            ng.ng_Width = GAD_WIDTH;
            ng.ng_GadgetText = "Animation";
            ng.ng_Flags = PLACETEXT_LEFT;

            prev = CreateGadget(CYCLE_KIND, prev, &ng,
                GTCY_Labels, quality,
                TAG_DONE);

            WORD bottom = ng.ng_TopEdge + ng.ng_Height + INTERVAL - (pubs->WBorTop + pubs->RastPort.Font->tf_YSize + 1);

            ng.ng_TopEdge += (ng.ng_Height + INTERVAL) * 2;
            ng.ng_LeftEdge = pubs->WBorLeft + OUTER;
            ng.ng_GadgetText = "Optimal settings";
            ng.ng_Flags = PLACETEXT_IN;

            prev = CreateGadget(BUTTON_KIND, prev, &ng,
                TAG_DONE);

            ng.ng_LeftEdge += ng.ng_Width + OUTER;
            ng.ng_GadgetText = "Launch...";
            ng.ng_GadgetID = GID_LAUNCH;

            prev = CreateGadget(BUTTON_KIND, prev, &ng,
                TAG_DONE);

            ng.ng_LeftEdge -= ng.ng_Width + OUTER;
            ng.ng_TopEdge += ng.ng_Height + INTERVAL;
            ng.ng_GadgetText = "Help...";

            prev = CreateGadget(BUTTON_KIND, prev, &ng,
                TAG_DONE);

            struct Window *w;
            WORD winHeight = ng.ng_TopEdge + ng.ng_Height + MARGIN;

            if (w = OpenWindowTags(NULL,
                WA_Left,    (pubs->Width - WIN_WIDTH) / 2,
                WA_Top,     (pubs->Height - winHeight) / 2,
                WA_Width,   WIN_WIDTH,
                WA_Height,  winHeight,
                WA_Title,   "Warehouse - options",
                WA_Activate,    TRUE,
                WA_CloseGadget, TRUE,
                WA_DepthGadget, TRUE,
                WA_DragBar,     TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_IDCMP,   IDCMP_CLOSEWINDOW|BUTTONIDCMP|CYCLEIDCMP|IDCMP_REFRESHWINDOW,
                WA_Gadgets, glist,
                TAG_DONE))
            {
                GT_RefreshWindow(w, NULL);
                BOOL done = FALSE;

                DrawBevelBox(w->RPort, OUTER, top, WIN_WIDTH - (OUTER * 2), bottom, GT_VisualInfo, vi, GTBB_Recessed, TRUE, TAG_DONE);

                while (!done)
                {
                    struct IntuiMessage *msg;
                    WaitPort(w->UserPort);
                    while (msg = GT_GetIMsg(w->UserPort))
                    {
                        if (msg->Class == IDCMP_CLOSEWINDOW)
                        {
                            done = TRUE;
                        }
                        else if (msg->Class == IDCMP_GADGETUP)
                        {
                            struct Gadget *gad = (struct Gadget *)msg->IAddress;
                            if (gad->GadgetID == GID_RESOLUTION)
                            {
                                askResolution(modeName, w, gadgets[TID_RESOLUTION], &bits, &modeID, &depth);
                            }
                            else if (gad->GadgetID == GID_LAUNCH)
                            {
                                struct Screen *ns;

                                if (ns = OpenScreenTags(NULL,
                                    SA_DisplayID,   modeID,
                                    SA_Width,       STDSCREENWIDTH,
                                    SA_Height,      STDSCREENHEIGHT,
                                    SA_Depth,       depth,
                                    SA_Title,       "Warehouse",
                                    TAG_DONE))
                                {
                                    Delay(300);
                                    CloseScreen(ns);
                                }
                            }
                        }
                        else if (msg->Class == IDCMP_REFRESHWINDOW)
                        {
                            GT_BeginRefresh(w);
                            DrawBevelBox(w->RPort, OUTER, top, WIN_WIDTH - (OUTER * 2), bottom, GT_VisualInfo, vi, GTBB_Recessed, TRUE, TAG_DONE);
                            GT_EndRefresh(w, TRUE);
                        }
                        GT_ReplyIMsg(msg);
                    }
                }
                CloseWindow(w);
            }
            FreeGadgets(glist);
            FreeVisualInfo(vi);
        }
        UnlockPubScreen(NULL, pubs);
    }
    return(TRUE);
}

main()
{
    GameOptions(NULL);
    return(0);
}
