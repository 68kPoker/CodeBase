
/* Struktury planszy */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>

#define PLANSZA_SZER 20
#define PLANSZA_WYS  16

struct Pole {
    WORD ptp : 4; /* Podstawowy typ podîogi */
    WORD pto : 4; /* Podstawowy typ obiektu */
    struct Obiekt *ob;
};

struct Obiekt {
    struct MinNode mn;
    WORD pto;
    WORD x, y;
};

struct Plansza {
    struct Pole tab[ PLANSZA_WYS ][ PLANSZA_SZER ];
    struct List obiekty;
    struct Obiekt bohater;
    WORD bx, by; /* Poîoûenie bohatera */
    WORD skrzynie; /* Liczba skrzyï na planszy */
    WORD ulozone;
    WORD dx, dy; /* Kierunek ruchu bohatera */
};

extern void nowaPlansza( struct Plansza *pl );
extern void ustawBohatera( struct Plansza *pl, WORD bx, WORD by );
extern void wstawKafelek( struct Plansza *pl, WORD x, WORD y, WORD kaf );
extern struct Obiekt *dodajObiekt( struct Plansza *pl, WORD x, WORD y, struct Pole *po );
extern void usunObiekt( struct Obiekt *ob );
extern void przemiescBohatera( struct Plansza *pl, struct Obiekt *ob );
extern void skanujObiekty( struct Plansza *pl );
