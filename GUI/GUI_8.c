
    /***************************************************************\
    *                                                               *
    *   GUI.c                                                       *
    *                                                               *
    \***************************************************************/

#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#define SCREEN_WIDE 320
#define SCREEN_TALL 256
#define SCREEN_DEEP 5
#define IDCMP_FLAGS IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE

struct NewMenu nm[] =
{
    { NM_TITLE, "Cell", 0, 0, 0, 0 },
    { NM_ITEM,  "Type", 0, 0, 0, 0 },
    { NM_SUB,   "Floor    ", "1", CHECKIT, ~1, 0 },
    { NM_SUB,   "Wall     ", "2", CHECKIT|CHECKED, ~2, 0 },
    { NM_SUB,   "Box      ", "3", CHECKIT, ~4, 0 },
    { NM_SUB,   "Flagstone", "4", CHECKIT, ~8, 0 },
    { NM_SUB,   NM_BARLABEL, 0, 0, 0, 0 },
    { NM_SUB,   "Clear    ", "0", CHECKIT, ~32, 0 },
    { NM_SUB,   "Delete   ", "D", CHECKIT, ~64, 0 },
    { NM_END }
};

struct Window *openGUI(ULONG modeID)
{
    struct Screen *s;
    WORD pens[] = { ~0 };

    if (s = OpenScreenTags(NULL,
        SA_Width,   SCREEN_WIDE,
        SA_Height,  SCREEN_TALL,
        SA_Depth,   SCREEN_DEEP,
        SA_DisplayID,   modeID,
        SA_ShowTitle,   FALSE,
        SA_Quiet,       TRUE,
        SA_Pens,        pens,
        TAG_DONE))
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
            WA_IDCMP,           IDCMP_FLAGS,
            WA_ReportMouse,     TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_NewLookMenus,    TRUE,
            TAG_DONE))
        {
            return(w);
        }
        CloseScreen(s);
    }
    return(NULL);
}

void closeGUI(struct Window *w)
{
    struct Screen *s = w->WScreen;

    CloseWindow(w);
    CloseScreen(s);
}

struct Menu *addMenu(struct Window *w, struct VisualInfo **vi)
{
    if (*vi = GetVisualInfoA(w->WScreen, NULL))
    {
        struct MenuStrip *menu;

        if (menu = CreateMenus(nm, TAG_DONE))
        {
            if (LayoutMenus(menu, *vi, GTMN_NewLookMenus, TRUE, TAG_DONE))
            {
                SetMenuStrip(w, menu);
                return(menu);
            }
            FreeMenus(menu);
        }
        FreeVisualInfo(*vi);
    }
    return(NULL);
}

void deleteMenu(struct Window *w, struct VisualInfo *vi, struct Menu *menu)
{
    ClearMenuStrip(w);
    FreeMenus(menu);
    FreeVisualInfo(vi);
}

int main()
{
    struct Window *w;

    if (w = openGUI(LORES_KEY))
    {
        struct Menu *menu;
        struct VisualInfo *vi;

        if (menu = addMenu(w, &vi))
        {
            Delay(300);
            deleteMenu(w, vi, menu);
        }
        closeGUI(w);
    }
    return(0);
}
