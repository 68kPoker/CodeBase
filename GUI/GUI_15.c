
/*
 * Magazyn
 * Graficzny interfejs uûytkownika.
 * Skîada sië z ekranu i okienek.
 * Jedno okienko to podglâd planszy - po zmaksymalizowaniu
 * wyôwietla sië plansza na caîym ekranie.
 */

#include <intuition/intuition.h>
#include <libraries/gadtools.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

struct GUI
    {
    struct Screen     *ekran;
    struct VisualInfo *infoWizualne; /* Dla gadtools */
    struct Window     *podglad, *menu;
    struct Plansza    *plansza;
    };

UWORD olowki[] = { ~0 };
struct ColorSpec colspec[] =
    {
    { 0, 0, 0, 0 },
    { -1 }
    };

/*
 * Ekran z planszâ
 */

struct Plansza
    {
    struct Screen     *ekran;
    struct Window     *okno;
    struct GUI        *gui;
    };

BOOL przygotujGUI(struct GUI *gui, UBYTE gleb, ULONG trybID)
{
    if (gui->ekran = OpenScreenTags(NULL,
        SA_Left,    0,
        SA_Top,     0,
        SA_Width,   STDSCREENWIDTH,
        SA_Height,  128,
        SA_Depth,   gleb,
        SA_DisplayID,   trybID,
        SA_Title,   "Magazyn (c)2018-2020 Robert Szacki",
        SA_Pens,    olowki,
        SA_Interleaved, TRUE,
        TAG_DONE))
        {
        if (gui->infoWizualne = GetVisualInfo(gui->ekran, TAG_DONE))
            {
            if (gui->podglad = OpenWindowTags(NULL,
                WA_CustomScreen,    gui->ekran,
                WA_Left,            0,
                WA_Top,             128,
                WA_Width,           320,
                WA_Height,          128,
                WA_IDCMP,           IDCMP_ACTIVEWINDOW|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW,
                WA_SimpleRefresh,   TRUE,
                WA_Borderless,      TRUE,
                WA_Backdrop,        TRUE,
                TAG_DONE))
                {
                if (gui->menu = OpenWindowTags(NULL,
                    WA_CustomScreen,    gui->ekran,
                    WA_Width,           320,
                    WA_Height,          128,
                    WA_IDCMP,           IDCMP_CLOSEWINDOW,
                    WA_CloseGadget,     TRUE,
                    WA_DepthGadget,     TRUE,
                    WA_DragBar,         TRUE,
                    WA_SimpleRefresh,   TRUE,
                    WA_Title,           "Magazyn - Opcje",
                    TAG_DONE))
                    {
                    struct RastPort *rp = gui->podglad->RPort;
                    SetAPen(rp, 1);
                    RectFill(rp, 0, 0, 319, 127);

                    return(TRUE);
                    }
                CloseWindow(gui->podglad);
                }
            FreeVisualInfo(gui->infoWizualne);
            }
        CloseScreen(gui->ekran);
        }
    return(FALSE);
}

VOID zamknijGUI(struct GUI *gui)
{
    CloseWindow(gui->menu);
    CloseWindow(gui->podglad);
    FreeVisualInfo(gui->infoWizualne);
    CloseScreen(gui->ekran);
}

BOOL przygotujPlansze(struct Plansza *pl, struct GUI *gui, UBYTE gleb, ULONG trybID)
{
    if (pl->ekran = OpenScreenTags(NULL,
        SA_Left,    0,
        SA_Top,     0,
        SA_Width,   STDSCREENWIDTH,
        SA_Height,  STDSCREENHEIGHT,
        SA_Depth,   gleb,
        SA_DisplayID,   trybID,
        SA_Title,   "Magazyn (c)2018-2020 Robert Szacki (Gra)",
        SA_ShowTitle,   FALSE,
        SA_Quiet,       TRUE,
        SA_Draggable,   FALSE,
        SA_Pens,    olowki,
        SA_Parent,  gui->ekran,
        SA_Behind,  TRUE,
        SA_Colors,  colspec,
        TAG_DONE))
        {
        if (pl->okno = OpenWindowTags(NULL,
            WA_CustomScreen,    pl->ekran,
            WA_Left,            0,
            WA_Top,             0,
            WA_Width,           pl->ekran->Width,
            WA_Height,          pl->ekran->Height,
            WA_Backdrop,        TRUE,
            WA_Borderless,      TRUE,
            WA_RMBTrap,         TRUE,
            WA_IDCMP,           IDCMP_MOUSEBUTTONS,
            TAG_DONE))
            {
            struct RastPort *rp = pl->okno->RPort;
            SetAPen(rp, 3);
            Move(rp, 0, 0);
            Draw(rp, pl->ekran->Width - 1, 0);
            Draw(rp, pl->ekran->Width - 1, pl->ekran->Height - 1);
            Draw(rp, 0, pl->ekran->Height - 1);
            Draw(rp, 0, 1);
            return(TRUE);
            }
        CloseScreen(pl->ekran);
        }
    return(FALSE);
}

VOID zamknijPlansze(struct Plansza *pl)
{
    CloseWindow(pl->okno);
    CloseScreen(pl->ekran);
}

VOID wyswietlPlansze(struct Plansza *pl)
{
    struct IntuiMessage *msg;
    ScreenDepth(pl->ekran, SDEPTH_TOFRONT|SDEPTH_INFAMILY, 0);
    BOOL koniec = FALSE;

    while (!koniec)
        {
        WaitPort(pl->okno->UserPort);
        while (msg = GT_GetIMsg(pl->okno->UserPort))
            {
            ULONG class = msg->Class;
            WORD code = msg->Code;
            WORD mx = msg->MouseX, my = msg->MouseY;
            GT_ReplyIMsg(msg);
            if (class == IDCMP_MOUSEBUTTONS)
                {
                if (code == IECODE_LBUTTON)
                    {
                    SetAPen(pl->okno->RPort, 2);
                    RectFill(pl->okno->RPort, mx & 0xfff0, my & 0xfff0, (mx & 0xfff0) + 15, (my & 0xfff0) + 15);
                    SetAPen(pl->gui->podglad->RPort, 2);
                    RectFill(pl->gui->podglad->RPort, (mx & 0xfff0), (my & 0xfff0) >> 1, (mx & 0xfff0) + 15, ((my & 0xfff0) >> 1) + 7);
                    }
                else if (code == IECODE_RBUTTON)
                    {
                    koniec = TRUE;
                    }
                }
            }
        }

    ScreenDepth(pl->ekran, SDEPTH_TOBACK|SDEPTH_INFAMILY, 0);
}

LONG obsluzGUI(struct GUI *gui)
{
    BOOL koniec = FALSE;

    while (!koniec)
        {
        struct IntuiMessage *msg;
        ULONG wynik;
        wynik = Wait((1L << gui->podglad->UserPort->mp_SigBit) |
             (1L << gui->menu->UserPort->mp_SigBit));

        if (wynik & (1L << gui->podglad->UserPort->mp_SigBit))
            {
            while (msg = GT_GetIMsg(gui->podglad->UserPort))
                {
                ULONG class = msg->Class;
                GT_ReplyIMsg(msg);
                if (class == IDCMP_ACTIVEWINDOW)
                    {
                    wyswietlPlansze(gui->plansza);
                    }
                else if (class == IDCMP_REFRESHWINDOW)
                    {
                    BeginRefresh(gui->podglad);
                    RectFill(gui->podglad->RPort, 0, 0, 319, 127);
                    EndRefresh(gui->podglad, TRUE);
                    }
                }
            }

        if (wynik & (1L << gui->menu->UserPort->mp_SigBit))
            {
            while (msg = GT_GetIMsg(gui->menu->UserPort))
                {
                ULONG class = msg->Class;
                GT_ReplyIMsg(msg);
                if (class == IDCMP_CLOSEWINDOW)
                    {
                    koniec = TRUE;
                    }
                }
            }
        }
    return(0);
}

int main()
{
    struct GUI gui;
    struct Plansza pl;

    gui.plansza = &pl;
    pl.gui = &gui;

    if (przygotujGUI(&gui, 2, HIRES_KEY))
        {
        if (przygotujPlansze(gui.plansza, &gui, 5, LORES_KEY))
            {
            obsluzGUI(&gui);
            zamknijPlansze(&pl);
            }
        zamknijGUI(&gui);
        }
    return(0);
}
