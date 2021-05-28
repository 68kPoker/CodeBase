
/* Magazyn dziaîa na dwóch ekranach */

#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <graphics/videocontrol.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

/* Otwieranie podstawowego ekranu LORES 32-kolory */

#define IDCMP     IDCMP_MENUPICK|IDCMP_GADGETDOWN|IDCMP_RAWKEY
#define IDCMP_AUX IDCMP_MENUPICK

UWORD pens[] = { ~0 };

struct NewMenu nm[] =
{
    { NM_TITLE, "Projekt", 0, 0, 0, 0 },
    { NM_ITEM,  "Nowy...  ", "N", 0, 0, 0 },
    { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
    { NM_ITEM,  "Koniec...", "Q", 0, 0, 0 },

    { NM_TITLE, "Kafelek", 0, 0, 0, 0 },
    { NM_ITEM,  "Wybierz...", "T", 0, 0, 0 },

    { NM_TITLE, "Obiekt", 0, 0, 0, 0 },
    { NM_ITEM,  "Wybierz...", "B", 0, 0, 0 },

    { NM_END }
};

WORD myXY[] =
{
    0, 14,
    0, 0,
    63, 0
};

WORD myXY2[] =
{
    63, 1,
    63, 14,
    1, 14
};

struct Border myShade =
{
    0, 0,
    2, 0,
    JAM1,
    3,
    myXY2,
    NULL
};

struct Border myBorder =
{
    0, 0,
    2, 0,
    JAM1,
    3,
    myXY,
    &myShade
};

struct IntuiText myText =
{
    2, 0,
    JAM1,
    2, 4,
    NULL,
    "Plansza",
    NULL
};

struct Gadget showBoardGad =
{
    NULL,
    0, 0,
    64, 15,
    GFLG_GADGHCOMP|GFLG_SELECTED,
    GACT_IMMEDIATE|GACT_TOGGLESELECT,
    GTYP_BOOLGADGET,
    &myBorder,
    NULL,
    &myText,
    0,
    NULL,
    1,
    NULL
};

struct ColorSpec colspec[] =
{
    { 0, 0, 0, 0 },
    { 3, 8, 8, 13 },
    { (~1) & 0x1f, 15, 15, 15 },
    { (~2) & 0x1f, 0, 0, 0 },
    { (~0) & 0x1f, 5, 5, 15 },
    { (~3) & 0x1f, 0, 0, 5 },
    { -1 }
};

struct Screen *openScreen()
{
    struct Screen *s;

    s = OpenScreenTags(NULL,
        SA_Width,       320,
        SA_Height,      256,
        SA_Depth,       5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Draggable,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Magazyn: Ekran planszy",
        SA_Colors,      colspec,
        TAG_DONE);

    if (s)
    {
        return s;
    }
    return NULL;
}

/* Otwieranie ekranu pomocniczego HIRES 4-kolory */

struct Screen *openAuxScreen(struct Screen *p, WORD top)
{
    struct Screen *s;
    struct TagItem vctags[] =
    {
        VC_NoColorPaletteLoad, TRUE,
        TAG_DONE
    };

    s = OpenScreenTags(NULL,
        SA_Parent,      p,
        SA_Top,         top,
        SA_Width,       640,
        SA_Height,      240,
        SA_Depth,       2,
        SA_DisplayID,   HIRES_KEY,
        SA_Title,       "Magazyn: Ekran menu",
        SA_Pens,        pens,
        SA_Behind,      TRUE,
        SA_VideoControl,    vctags,
        SA_MinimizeISG, TRUE,
        SA_Draggable,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE);

    if (s)
    {
        return s;
    }
    return NULL;
}

/* Zamykanie ekranu */

void closeScreen(struct Screen *s)
{
    CloseScreen(s);
}

/* Otwieranie okna w tle */

struct Window *openBackWindow(struct Screen *s, ULONG idcmp, struct Gadget *gads, BOOL rmbtrap)
{
    struct Window *w;

    w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Width,
        WA_Height,          s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         rmbtrap,
        WA_IDCMP,           idcmp,
        WA_BackFill,        LAYERS_BACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_NewLookMenus,    TRUE,
        WA_Gadgets,         gads,
        TAG_DONE);

    if (w)
    {
        return w;
    }
    return NULL;
}

/* Otwieranie gîównego okna */

struct Window *openMainWindow(struct Screen *s)
{
    struct Window *w;

    w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             s->BarHeight + 1,
        WA_Width,           s->Width,
        WA_Height,          s->Height - (s->BarHeight + 1),
        WA_SimpleRefresh,   TRUE,
        WA_DragBar,         TRUE,
        WA_CloseGadget,     TRUE,
        WA_DepthGadget,     TRUE,
        WA_IDCMP,           IDCMP_CLOSEWINDOW|IDCMP_MENUPICK,
        WA_Title,           "Magazyn: Menu",
        WA_ScreenTitle,     "Magazyn",
        WA_NewLookMenus,    TRUE,
        TAG_DONE);

    if (w)
    {
        return w;
    }
    return NULL;
}

/* Zamykanie okna */

void closeWindow(struct Window *w)
{
    CloseWindow(w);
}

/* Utwórz i dodaj menu */

struct Menu *addMenu(struct Window *w, struct VisualInfo *vi, struct NewMenu *nm)
{
    struct Menu *menu;

    if (menu = CreateMenus(nm, TAG_DONE))
    {
        if (LayoutMenus(menu, vi, GTMN_NewLookMenus, TRUE, TAG_DONE))
        {
            SetMenuStrip(w, menu);
            return menu;
        }
        FreeMenus(menu);
    }
    return NULL;
}

/* Usuï i zwolnij menu */

void remMenu(struct Window *w, struct Menu *menu)
{
    ClearMenuStrip(w);
    FreeMenus(menu);
}

int main()
{
    struct Screen *s[2];
    struct Window *w[3];
    struct VisualInfo *vi;
    struct Menu *menu;

    if (s[0] = openScreen())
    {
        if (s[1] = openAuxScreen(s[0], 16))
        {
            if (w[0] = openBackWindow(s[0], IDCMP, &showBoardGad, TRUE))
            {
                if (w[1] = openBackWindow(s[1], IDCMP_AUX, NULL, FALSE))
                {
                    if (vi = GetVisualInfoA(s[1], NULL))
                    {
                        if (w[2] = openMainWindow(s[1]))
                        {
                            if (menu = addMenu(w[2], vi, nm))
                            {
                                BOOL done = FALSE;
                                WORD toggle = 0;

                                while (!done)
                                {
                                    struct IntuiMessage *msg;
                                    WaitPort(w[0]->UserPort);
                                    while (msg = GT_GetIMsg(w[0]->UserPort))
                                    {
                                        if (msg->Class == IDCMP_GADGETDOWN)
                                        {
                                            toggle ^= 1;
                                            if (toggle)
                                                ScreenDepth(s[1], SDEPTH_TOFRONT|SDEPTH_INFAMILY, 0);
                                            else
                                                ScreenDepth(s[1], SDEPTH_TOBACK|SDEPTH_INFAMILY, 0);

                                            /* ScreenPosition(s[1], SPOS_ABSOLUTE|SPOS_FORCEDRAG, 0, pos[toggle], 319, 255); */
                                        }
                                        else if (msg->Class == IDCMP_RAWKEY)
                                        {
                                            done = TRUE;
                                        }
                                        GT_ReplyIMsg(msg);
                                    }
                                }
                                remMenu(w[2], menu);
                            }
                            closeWindow(w[2]);
                        }
                        FreeVisualInfo(vi);
                    }
                    closeWindow(w[1]);
                }
                closeWindow(w[0]);
            }
            closeScreen(s[1]);
        }
        closeScreen(s[0]);
    }
    return 0;
}
