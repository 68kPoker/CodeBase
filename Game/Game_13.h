
#ifndef GRA_H
#define GRA_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* Podstawowe pojëcia gry */

/* 1. Plansza */

#define SZER 20
#define WYS  16

/* 2. Pole */

/* Zawiera informacjë o polu. */

struct Pole
{
    WORD typ; /* Podstawowy typ pola */
    WORD obiekt; /* Obiekt */
    WORD stan;
};

/* Prostokâtny obszar zîoûony z pól. */

struct Plansza
{
    struct Pole obszar[2][WYS][SZER]; /* Dwa bufory */
    WORD bufor;

    /* Dodatkowe, wyliczane atrybuty planszy: */
    /* - Poîoûenie bohatera */
    /* - Liczba skrzyï na planszy */

    WORD bohater_x, bohater_y;
    WORD skrzynie, ulozone, klucze;
};

/* Wyliczenie typów pól i obiektów: */

enum typy
{
    TLO,
    PODLOGA,
    MIEJSCE
};

enum obiekty
{
    BRAK,
    SKRZYNIA,
    KLUCZ,
    SCIANA,
    DRZWI
};

/* Struktury edytora plansz */

struct Edytor
{
    struct Plansza *plansza; /* Aktywna plansza */
    struct Pole aktywne; /* Aktywny rodzaj pola i obiektu */
    WORD tryb; /* Tryb pracy */
    WORD x, y; /* Aktywna pozycja kursora */
};

enum tryby
{
    GUMKA, /* Zmaû obiekt, nastëpnie podîogë */
    WKLEJ_PODLOGE, /* Wklej tylko podîogë */
    WKLEJ_OBIEKT, /* Wklej obiekt */
    WKLEJ_OBIE /* Wklej obie warstwy */
};

/* Deklaracje funkcji */

BOOL nowaPlansza(struct Plansza *pl);
VOID skopiujPlansze(struct Plansza *cel, struct Plansza *zrod);
VOID usunPlansze(struct Plansza *pl);
struct Pole *pobierzPole(struct Plansza *pl, WORD x, WORD y);
VOID ustawBohatera(struct Plansza *pl, WORD x, WORD y);

BOOL skanujPlansze(struct Plansza *pl);

BOOL przygotujEdytor(struct Edytor *ed);
VOID ustawTrybPracy(struct Edytor *ed, WORD tryb);
BOOL ustawKursor(struct Edytor *ed, WORD x, WORD y);
VOID ustawAktywnePole(struct Edytor *ed, struct Pole *po);

VOID wstawPole(struct Edytor *ed);

/* Silnik gry. Animuje planszë. Pobiera opc. kierunek ruchu bohatera */
BOOL silnikGry(struct Plansza *pl, WORD dx, WORD dy);

/* Obsîuû pole */
BOOL obsluzPole(struct Plansza *pl, WORD x, WORD y);

#endif /* GRA_H */
