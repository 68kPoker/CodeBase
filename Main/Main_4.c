
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "System.h"
#include "Gra.h"

VOID narysujObiekt(struct Zasoby *zas, struct Pole *po, WORD x, WORD y, struct BitMap *bm)
{
    BltBitMap(zas->bitmapa, po->obiekt << ROZMIAR_X, 2 << ROZMIAR_Y, bm, x << ROZMIAR_X, y << ROZMIAR_Y, 1 << ROZMIAR_X, 1 << ROZMIAR_Y, 0xc0, 0xff, NULL);
}

VOID narysujPole(struct Zasoby *zas, struct Pole *po, WORD x, WORD y, struct BitMap *bm)
{
    BltBitMap(zas->bitmapa, po->typ << ROZMIAR_X, 1 << ROZMIAR_Y, bm, x << ROZMIAR_X, y << ROZMIAR_Y, 1 << ROZMIAR_X, 1 << ROZMIAR_Y, 0xc0, 0xff, NULL);
}

VOID narysujBohatera(struct Zasoby *zas, struct Plansza *pl, struct BitMap *bm)
{
    WORD x = pl->bohater_x, y = pl->bohater_y;

    BltBitMap(zas->bitmapa, 0, 3 << ROZMIAR_Y, bm, x << ROZMIAR_X, y << ROZMIAR_Y, 1 << ROZMIAR_X, 1 << ROZMIAR_Y, 0xc0, 0xff, NULL);
}

VOID wyswietlPlansze(struct Zasoby *zas, struct Plansza *pl)
{
    struct BitMap *bm = zas->bufory[zas->aktywnyBufor]->sb_BitMap;
    WORD x, y;

    for (y = 1; y < WYS; y++)
    {
        for (x = 0; x < SZER; x++)
        {
            struct Pole *po = pobierzPole(pl, x, y);

            if (po->obiekt)
            {
                narysujObiekt(zas, po, x, y, bm);
            }
            else
            {
                narysujPole(zas, po, x, y, bm);
            }
        }
    }
    narysujBohatera(zas, pl, bm);
}

VOID aktualizujPlansze(struct Zasoby *zas, struct Plansza *pl)
{
    WORD x, y;
    struct BitMap *bm = zas->bufory[zas->aktywnyBufor]->sb_BitMap;
    WORD dx = zas->dx, dy = zas->dy;

    if (zas->porusz & 1)
    {
        dx = -1;
        dy = 0;
    }
    else if (zas->porusz & 2)
    {
        dx = 1;
        dy = 0;
    }
    else if (zas->porusz & 4)
    {
        dx = 0;
        dy = -1;
    }
    else if (zas->porusz & 8)
    {
        dx = 0;
        dy = 1;
    }
    else
    {
        dx = 0;
        dy = 0;
    }
    if (zas->licznik == 0)
    {
        silnikGry(pl, zas->dx, zas->dy);
        if (zas->dx || zas->dy)
        {
            zas->licznik = zas->predkosc;
        }
    }
    else
    {
        zas->licznik--;
    }
    zas->dx = dx;
    zas->dy = dy;

    for (y = 0; y < WYS; y++)
    {
        for (x = 0; x < SZER; x++)
        {
            struct Pole *po = pobierzPole(pl, x, y);
            if (po->stan > 0)
            {
                if (po->obiekt)
                {
                    narysujObiekt(zas, po, x, y, bm);
                }
                else
                {
                    narysujPole(zas, po, x, y, bm);
                }
            }
        }
    }
    narysujBohatera(zas, pl, bm);
}

LONG gra(struct Zasoby *zas, struct Plansza *pl, struct Edytor *ed)
{
    BOOL koniec = FALSE;
    ULONG maski[] =
    {
        1L << zas->okno->UserPort->mp_SigBit,
        1L << zas->porty[0]->mp_SigBit,
        1L << zas->porty[1]->mp_SigBit
    }, maska = maski[0] | maski[1] | maski[2];

    BOOL narysowano = FALSE;

    wyswietlPlansze(zas, pl);
    zas->ekran->RastPort.BitMap = zas->bufory[zas->aktywnyBufor]->sb_BitMap;
    narysujBelke(zas);

    while (!ChangeScreenBuffer(zas->ekran, zas->bufory[zas->aktywnyBufor]))
    {
        WaitTOF();
    }

    zas->aktywnyBufor ^= 1;
    zas->wyswietlono = zas->moznaRysowac = FALSE;

    while (!koniec)
    {
        ULONG wynik;

        wynik = Wait(maska);

        if (wynik & maski[0])
        {
            struct IntuiMessage *msg;

            while (msg = (struct IntuiMessage *)GetMsg(zas->okno->UserPort))
            {
                ULONG klasa = msg->Class;
                UWORD kod = msg->Code;
                WORD mysz_x = msg->MouseX;
                WORD mysz_y = msg->MouseY;

                ReplyMsg((struct Message *)msg);

                if (klasa == IDCMP_RAWKEY)
                {
                    if (kod == 0x45)
                    {
                        koniec = TRUE;
                    }
                    else if (kod == 0x4f)
                    {
                        zas->porusz |= 1; /* -1, 0 */
                    }
                    else if (kod == 0x4e)
                    {
                        zas->porusz |= 2; /* 1, 0 */
                    }
                    else if (kod == 0x4c)
                    {
                        zas->porusz |= 4; /* 0, -1 */
                    }
                    else if (kod == 0x4d)
                    {
                        zas->porusz |= 8; /* 0, 1 */
                    }
                    else if (kod == 0xcf)
                    {
                        zas->porusz &= ~1;
                    }
                    else if (kod == 0xce)
                    {
                        zas->porusz &= ~2;
                    }
                    else if (kod == 0xcc)
                    {
                        zas->porusz &= ~4;
                    }
                    else if (kod == 0xcd)
                    {
                        zas->porusz &= ~8;
                    }
                }
            }
        }
        if (wynik & maski[1])
        {
            if (!zas->moznaRysowac)
            {
                while (!GetMsg(zas->porty[0]))
                {
                    WaitPort(zas->porty[0]);
                }
            }
            zas->moznaRysowac = TRUE;

            if (!narysowano)
            {
                wyswietlPlansze(zas, pl);
                narysowano = TRUE;
            }
            aktualizujPlansze(zas, pl);
        }
        if (wynik & maski[2])
        {
            if (!zas->wyswietlono)
            {
                while (!GetMsg(zas->porty[1]))
                {
                    WaitPort(zas->porty[1]);
                }
            }
            zas->wyswietlono = TRUE;

            WaitBlit();
            while (!ChangeScreenBuffer(zas->ekran, zas->bufory[zas->aktywnyBufor]))
            {
                WaitTOF();
            }
            zas->aktywnyBufor ^= 1;
            zas->wyswietlono = zas->moznaRysowac = FALSE;
        }
    }
    return(0);
}

int main()
{
    struct Zasoby zas = { 0 };

    if (przygotujZasoby(&zas, "Dane/Magazyn.pic"))
    {
        struct Plansza pl = { 0 };
        struct Edytor ed = { 0 };

        if (nowaPlansza(&pl))
        {
            if (przygotujEdytor(&ed))
            {
                gra(&zas, &pl, &ed);
            }
            usunPlansze(&pl);
        }
        zwolnijZasoby(&zas);
    }
    return(0);
}
