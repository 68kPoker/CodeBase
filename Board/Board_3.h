
/* Definicja planszy */

#ifndef PLANSZA_H
#define PLANSZA_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define TYPY 4 /* Max. liczba podtypów */

#define SZER 20 /* Wymiary planszy */
#define WYS  16

typedef UBYTE TYP;

typedef struct Warstwa
{
    TYP typy[TYPY];
    UBYTE id; /* Identyfikator */
} WARSTWA;

typedef struct Pole
{
    WARSTWA podloga, obiekt; /* Dwie warstwy pola */
} POLE;

enum /* Typy podîóg */
{
    PT_BRAK, /* Brak podîogi (tîo) */
    PT_PODLOGA,
    PT_PRZYCISK /* Oznaczona podîoga */
};

enum /* Typy obiektów */
{
    OT_BRAK,
    OT_SCIANA,
    OT_SKRZYNIA,
    OT_BOHATER
};

struct Plansza
{
    POLE tablica[WYS][SZER];
    WORD b_x, b_y; /* Pozycja bohatera */
};

#endif /* PLANSZA_H */
