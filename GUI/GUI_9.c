
#include "GUI.h"
#include "GUI_protos.h"

#include <intuition/intuition.h>
#include <libraries/gadtools.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/exec_protos.h>

struct NewMenu noweMenu[] =
{
    { NM_TITLE, "Plansza", 0, 0, 0, 0 },
    { NM_ITEM,  "Nowa...      ", "N", 0, 0, 0 },
    { NM_ITEM,  "Wczytaj...   ", "O", 0, 0, 0 },
    { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
    { NM_ITEM,  "Zapisz...    ", "S", 0, 0, 0 },
    { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
    { NM_ITEM,  "Informacje...", "?", 0, 0, 0 },
    { NM_ITEM,  "Wyjdú...     ", "Q", 0, 0, 0 },
    { NM_TITLE,  "Podîoga", 0, 0, 0, 0 },
    { NM_ITEM,   "Gumka  ", "0", CHECKIT, ~1, 0 },
    { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
    { NM_ITEM,   "Podîoga", "1", CHECKIT|CHECKED, ~4, 0 },
    { NM_ITEM,   "Miejsce", "2", CHECKIT, ~8, 0 },
    { NM_TITLE,  "Obiekt ", 0, 0, 0, 0 },
    { NM_ITEM,   "Aktywny ", "W", CHECKIT|MENUTOGGLE|CHECKED, 0, 0 },
    { NM_ITEM,  NM_BARLABEL, 0, 0, 0, 0 },
    { NM_ITEM,   "Wymaû   ", "-", CHECKIT, ~(4|1), 0 },
    { NM_ITEM,   "Ôciana  ", "!", CHECKIT|CHECKED, ~(8|1), 0 },
    { NM_ITEM,   "Skrzynia", "@", CHECKIT, ~(16|1), 0 },
    { NM_ITEM,   "Bohater ", "#", CHECKIT, ~(32|1), 0 },
    { NM_END }
};

BOOL otworzEkran(struct ekranInfo *ei)
{
    struct Screen *s;
    ULONG tryb = LORES_KEY;
    static UBYTE tytul[] = "Magazyn";
    static UWORD olowki[] = { ~0 };

    if (ei->s = s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       STDSCREENWIDTH,
        SA_Height,      STDSCREENHEIGHT,
        SA_Depth,       GLEB,
        SA_DisplayID,   tryb,
        SA_Title,       tytul,
        SA_Font,        &ei->ta,
        SA_Pens,        olowki,
        TAG_DONE))
    {
        if (ei->vi = GetVisualInfoA(s, NULL))
        {
            if (ei->bufory[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
            {
                if (ei->bufory[1] = AllocScreenBuffer(s, NULL, 0))
                {
                    if (ei->porty[0] = CreateMsgPort())
                    {
                        if (ei->porty[1] = CreateMsgPort())
                        {
                            ei->bufory[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = ei->porty[0];
                            ei->bufory[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = ei->porty[0];
                            ei->bufory[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = ei->porty[1];
                            ei->bufory[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = ei->porty[1];
                            ei->moznaRysowac = ei->moznaZmieniac = TRUE;

                            s->UserData = (APTR)ei;
                            return(TRUE);
                        }
                        DeleteMsgPort(ei->porty[0]);
                    }
                    FreeScreenBuffer(s, ei->bufory[1]);
                }
                FreeScreenBuffer(s, ei->bufory[0]);
            }
            FreeVisualInfo(ei->vi);
        }
        CloseScreen(s);
    }
    return(FALSE);
}

BOOL otworzOkno(struct oknoInfo *oi, WORD typ, struct ekranInfo *ei)
{
    switch (typ)
    {
        case OKNO_GLOWNE:
            if (oi->w = OpenWindowTags(NULL,
                WA_CustomScreen,    ei->s,
                WA_Left,            0,
                WA_Top,             0,
                WA_Width,           ei->s->Width,
                WA_Height,          ei->s->Height,
                WA_Backdrop,        TRUE,
                WA_Borderless,      TRUE,
                WA_Activate,        TRUE,
                WA_IDCMP,           IDCMP_MENUPICK|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW,
                WA_ReportMouse,     TRUE,
                WA_SimpleRefresh,   TRUE,
                WA_NewLookMenus,    TRUE,
                TAG_DONE))
            {
                if (oi->menu = CreateMenus(noweMenu, TAG_DONE))
                {
                    if (LayoutMenus(oi->menu, ei->vi, GTMN_NewLookMenus, TRUE, TAG_DONE))
                    {
                        SetMenuStrip(oi->w, oi->menu);
                        return(TRUE);
                    }
                    FreeMenus(oi->menu);
                }
                CloseWindow(oi->w);
            }
            break;
        case OKNO_OPCJI:
            break;
    }
    return(FALSE);
}

VOID zamknijEkran(struct ekranInfo *ei)
{
    if (!ei->moznaZmieniac)
    {
        while (!GetMsg(ei->porty[1]))
        {
            WaitPort(ei->porty[1]);
        }
    }

    if (!ei->moznaRysowac)
    {
        while (!GetMsg(ei->porty[0]))
        {
            WaitPort(ei->porty[0]);
        }
    }

    DeleteMsgPort(ei->porty[1]);
    DeleteMsgPort(ei->porty[0]);

    FreeScreenBuffer(ei->s, ei->bufory[1]);
    FreeScreenBuffer(ei->s, ei->bufory[0]);

    FreeVisualInfo(ei->vi);
    CloseScreen(ei->s);
}

VOID zamknijOkno(struct oknoInfo *oi)
{
    ClearMenuStrip(oi->w);
    FreeMenus(oi->menu);
    CloseWindow(oi->w);
}
