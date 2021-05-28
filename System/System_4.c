
#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/datatypes_protos.h>

#include "System.h"

struct Library *IntuitionBase, *GfxBase, *DataTypesBase;

VOID narysujBelke(struct Zasoby *zas)
{
    BltBitMapRastPort(zas->bitmapa, 0, 0, &zas->ekran->RastPort, 0, 0, 320, 16, 0xc0);
}

BOOL otworzBiblioteki(LONG ver)
{
    if (IntuitionBase = OpenLibrary("intuition.library", ver))
    {
        if (GfxBase = OpenLibrary("graphics.library", ver))
        {
            if (DataTypesBase = OpenLibrary("datatypes.library", ver))
            {
                return(TRUE);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

VOID zamknijBiblioteki()
{
    CloseLibrary(DataTypesBase);
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
}

BOOL przygotujZasoby(struct Zasoby *zas, STRPTR nazwa)
{
    if (otworzBiblioteki(WERSJA))
    {
        struct Screen *pubs;
        if (pubs = LockPubScreen(NULL))
        {
            if ((zas->trybEkranu = BestModeID(
                BIDTAG_ViewPort, &pubs->ViewPort,
                BIDTAG_NominalWidth,  SZER_EKRANU,
                BIDTAG_NominalHeight, WYS_EKRANU,
                BIDTAG_Depth, GLEB_EKRANU,
                TAG_DONE)) != INVALID_ID)
            {
                if (zas->ekran = OpenScreenTags(NULL,
                    SA_Left,            0,
                    SA_Top,             0,
                    SA_Width,           SZER_EKRANU,
                    SA_Height,          WYS_EKRANU,
                    SA_Depth,           GLEB_EKRANU,
                    SA_DisplayID,       zas->trybEkranu,
                    SA_Quiet,           TRUE,
                    SA_ShowTitle,       FALSE,
                    SA_Draggable,       FALSE,
                    SA_BackFill,        LAYERS_NOBACKFILL,
                    SA_Behind,          TRUE,
                    TAG_DONE))
                {
                    if (zas->bufory[0] = AllocScreenBuffer(zas->ekran, NULL, SB_SCREEN_BITMAP))
                    {
                        if (zas->bufory[1] = AllocScreenBuffer(zas->ekran, NULL, 0))
                        {
                            if (zas->porty[0] = CreateMsgPort())
                            {
                                if (zas->porty[1] = CreateMsgPort())
                                {
                                    zas->bufory[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = zas->porty[0];
                                    zas->bufory[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = zas->porty[1];
                                    zas->bufory[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = zas->porty[0];
                                    zas->bufory[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = zas->porty[1];

                                    zas->moznaRysowac = zas->wyswietlono = TRUE;
                                    zas->aktywnyBufor = 1;

                                    zas->predkosc = PREDKOSC_NORMALNA;
                                    zas->licznik  = 0;
                                    zas->blitter  = FALSE;

                                    if (zas->grafika = NewDTObject(nazwa,
                                        DTA_GroupID,    GID_PICTURE,
                                        PDTA_Screen,    zas->ekran,
                                        PDTA_Remap,     FALSE,
                                        TAG_DONE))
                                    {
                                        ULONG *kolory, liczba;
                                        WORD kolor;
                                        DoDTMethod(zas->grafika, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

                                        GetDTAttrs(zas->grafika,
                                            PDTA_BitMap,    &zas->bitmapa,
                                            PDTA_CRegs,     &kolory,
                                            PDTA_NumColors, &liczba,
                                            TAG_DONE);

                                        for (kolor = 0; kolor < liczba; kolor++)
                                        {
                                            SetRGB32CM(zas->ekran->ViewPort.ColorMap, kolor, kolory[0], kolory[1], kolory[2]);
                                            kolory += 3;
                                        }

                                        MakeScreen(zas->ekran);
                                        RethinkDisplay();
                                        narysujBelke(zas);
                                        WaitBlit();
                                        if (zas->okno = OpenWindowTags(NULL,
                                            WA_CustomScreen,    zas->ekran,
                                            WA_Left,            0,
                                            WA_Top,             0,
                                            WA_Width,           zas->ekran->Width,
                                            WA_Height,          zas->ekran->Height,
                                            WA_Backdrop,        TRUE,
                                            WA_Borderless,      TRUE,
                                            WA_Activate,        TRUE,
                                            WA_RMBTrap,         TRUE,
                                            WA_IDCMP,           FLAGI_IDCMP,
                                            WA_ReportMouse,     TRUE,
                                            WA_SimpleRefresh,   TRUE,
                                            WA_BackFill,        LAYERS_NOBACKFILL,
                                            TAG_DONE))
                                        {
                                            ScreenToFront(zas->ekran);
                                            UnlockPubScreen(NULL, pubs);
                                            return(TRUE);
                                        }
                                        DisposeDTObject(zas->grafika);
                                    }
                                    DeleteMsgPort(zas->porty[1]);
                                }
                                DeleteMsgPort(zas->porty[0]);
                            }
                            FreeScreenBuffer(zas->ekran, zas->bufory[1]);
                        }
                        FreeScreenBuffer(zas->ekran, zas->bufory[0]);
                    }
                    CloseScreen(zas->ekran);
                }
            }
            UnlockPubScreen(NULL, pubs);
        }
        zamknijBiblioteki();
    }
    return(FALSE);
}

VOID zwolnijZasoby(struct Zasoby *zas)
{
    CloseWindow(zas->okno);
    DisposeDTObject(zas->grafika);

    if (!zas->wyswietlono)
    {
        while (!GetMsg(zas->porty[1]))
        {
            WaitPort(zas->porty[1]);
        }
    }

    if (!zas->moznaRysowac)
    {
        while (!GetMsg(zas->porty[0]))
        {
            WaitPort(zas->porty[0]);
        }
    }

    DeleteMsgPort(zas->porty[1]);
    DeleteMsgPort(zas->porty[0]);

    FreeScreenBuffer(zas->ekran, zas->bufory[1]);
    FreeScreenBuffer(zas->ekran, zas->bufory[0]);

    CloseScreen(zas->ekran);

    zamknijBiblioteki();
}
