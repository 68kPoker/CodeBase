
#include "Gra.h"

/* Deklaracje funkcji */

BOOL nowaPlansza(struct Plansza *pl)
{
    WORD x, y;
    struct Pole podloga = { PODLOGA, BRAK }, sciana = { PODLOGA, SCIANA }, skrzynia = { PODLOGA, SKRZYNIA },
        miejsce = { MIEJSCE, BRAK }, drzwi = { PODLOGA, DRZWI }, klucz = { PODLOGA, KLUCZ };

    pl->bufor = 0;
    for (y = 0; y < WYS; y++)
    {
        for (x = 0; x < SZER; x++)
        {
            if (x == 0 || y <= 1 || x == SZER - 1 || y == WYS - 1)
            {
                pl->obszar[0][y][x] = sciana;
            }
            else
            {
                pl->obszar[0][y][x] = podloga;
            }
        }
    }
    pl->obszar[0][5][5] = skrzynia;
    pl->obszar[0][6][6] = skrzynia;
    pl->obszar[0][7][7] = miejsce;
    pl->obszar[0][8][8] = drzwi;
    pl->obszar[0][9][9] = klucz;

    pl->bohater_x = pl->bohater_y = 2;
    pl->skrzynie = pl->ulozone = 0;
    pl->klucze = 0; /* Posiadane klucze */
    return(TRUE);
}

/* Zrób kopië planszy. Przydatne przy restarcie. */
VOID skopiujPlansze(struct Plansza *cel, struct Plansza *zrod)
{
    WORD x, y;

    for (y = 0; y < WYS; y++)
    {
        for (x = 0; x < SZER; x++)
        {
            cel->obszar[0][y][x] = zrod->obszar[0][y][x];
        }
    }
    cel->bohater_x = zrod->bohater_x;
    cel->bohater_y = zrod->bohater_y;
    cel->skrzynie = zrod->skrzynie;
    cel->ulozone = zrod->ulozone;
    cel->klucze = zrod->klucze;
}

VOID usunPlansze(struct Plansza *pl)
{
    /* Brak dealokacji */
}

struct Pole *pobierzPole(struct Plansza *pl, WORD x, WORD y)
{
    return(&pl->obszar[pl->bufor][y][x]);
}

VOID ustawBohatera(struct Plansza *pl, WORD x, WORD y)
{
    pl->bohater_x = x;
    pl->bohater_y = y;
}

BOOL skanujPlansze(struct Plansza *pl)
{
    WORD x, y;
    WORD skrzynie = 0, ulozone = 0;

    for (y = 0; y < WYS; y++)
    {
        for (x = 0; x < SZER; x++)
        {
            struct Pole *po = pobierzPole(pl, x, y);
            WORD typ = po->typ, obiekt = po->obiekt;
            if (obiekt == SKRZYNIA)
            {
                skrzynie++;
                if (typ == MIEJSCE)
                {
                    ulozone++;
                }
            }
        }
    }
    pl->skrzynie = skrzynie;
    pl->ulozone = ulozone;
    pl->klucze = 0;
    return(TRUE);
}

BOOL przygotujEdytor(struct Edytor *ed)
{
    ed->aktywne.typ = PODLOGA;
    ed->aktywne.obiekt = BRAK;
    ed->tryb = WKLEJ_OBIE;
    ed->x = ed->y = 0;
    return(TRUE);
}

VOID ustawTrybPracy(struct Edytor *ed, WORD tryb)
{
    ed->tryb = tryb;
}

BOOL ustawKursor(struct Edytor *ed, WORD x, WORD y)
{
    ed->x = x;
    ed->y = y;
}

VOID ustawAktywnePole(struct Edytor *ed, struct Pole *po)
{
    ed->aktywne = *po;
}

VOID wstawPole(struct Edytor *ed)
{
    struct Pole *po = &ed->plansza->obszar[ed->plansza->bufor][ed->y][ed->x];

    if (ed->tryb & WKLEJ_PODLOGE)
    {
        po->typ = ed->aktywne.typ;
    }

    if (ed->tryb & WKLEJ_OBIEKT)
    {
        po->obiekt = ed->aktywne.obiekt;
    }

    if (ed->tryb == GUMKA)
    {
        if (po->obiekt != BRAK)
        {
            po->obiekt = BRAK;
        }
        else
        {
            po->typ = TLO;
        }
    }
}

/* Silnik gry. Animuje planszë. Pobiera opc. kierunek ruchu bohatera */
BOOL silnikGry(struct Plansza *pl, WORD dx, WORD dy)
{
    WORD bx = pl->bohater_x, by = pl->bohater_y, cx, cy;
    WORD x, y;

    if ((cx = bx + dx) < 0 || cx >= SZER || (cy = by + dy) < 0 || cy >= WYS)
    {
        /* Nie moûna iôê */
    }
    else
    {
        /* Moûna iôê, obsîuû */
        struct Pole *po = pobierzPole(pl, bx, by);
        struct Pole *cpo = pobierzPole(pl, cx, cy);
        BOOL idz = FALSE;

        if (cpo->obiekt == BRAK)
        {
            /* Brak obiektu */
            idz = TRUE;
        }
        else if (cpo->obiekt == SKRZYNIA)
        {
            /* Skrzynia na drodze */
            struct Pole *ccpo = pobierzPole(pl, cx + dx, cy + dy);

            if (ccpo->obiekt == BRAK)
            {
                /* Moûna popchnâê */
                ccpo->obiekt = cpo->obiekt;
                cpo->obiekt = BRAK;
                ccpo->stan = 3;
                idz = TRUE;

                if (ccpo->typ == MIEJSCE)
                {
                    pl->ulozone++;
                }
                if (cpo->typ == MIEJSCE)
                {
                    pl->ulozone--;
                }
            }
        }
        else if (cpo->obiekt == KLUCZ)
        {
            /* Klucz do zebrania */
            pl->klucze++;
            idz = TRUE;
        }
        else if (cpo->obiekt == DRZWI)
        {
            /* Drzwi */
            if (pl->klucze > 0)
            {
                /* Moûna otworzyê */
                pl->klucze--;
                idz = TRUE;
            }
        }

        if (idz)
        {
            pl->bohater_x += dx;
            pl->bohater_y += dy;
            cpo->obiekt = BRAK;
            po->stan = 3;
            cpo->stan = 3;
        }
    }

    for (y = 0; y < WYS; y++)
    {
        for (x = 0; x < SZER; x++)
        {
            obsluzPole(pl, x, y);
        }
    }

    pl->bufor ^= 1;
    return(TRUE);
}

/* Obsîuû pole */
BOOL obsluzPole(struct Plansza *pl, WORD x, WORD y)
{
    /* Animacja pól */
    WORD bufor = pl->bufor;
    struct Pole *po = pobierzPole(pl, x, y);
    if (po->stan > 0)
    {
        po->stan--;
    }
    pl->obszar[bufor ^ 1][y][x] = *po;

    return(TRUE);
}
