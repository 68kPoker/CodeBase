
#ifndef SYSTEM_H
#define SYSTEM_H

#ifndef INTUITION_CLASSUSR_H
#include <intuition/classusr.h>
#endif

/* Podstawowe pojëcia systemowe */

#define WERSJA 39L

#define SZER_EKRANU 320
#define WYS_EKRANU  256
#define GLEB_EKRANU 5

#define FLAGI_IDCMP IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE

#define ROZMIAR_X 4
#define ROZMIAR_Y 4

enum
{
    PREDKOSC_NORMALNA=8,
};

/* Zasoby systemowe */

struct Zasoby
{
    struct Screen *ekran; /* Ekran */
    struct Window *okno; /* Okno gîówne */
    struct ScreenBuffer *bufory[2]; /* Bufory ekranu */
    WORD aktywnyBufor;
    struct MsgPort *porty[2]; /* Porty komunikacyjne */
    BOOL moznaRysowac, wyswietlono;
    WORD predkosc; /* Prëdkoôê gry */
    WORD licznik;
    BOOL porusz;
    WORD dx, dy;
    BOOL blitter; /* Uûywaj Blittera? */
    ULONG trybEkranu;
    UWORD gleb;
    Object *grafika; /* Obiekt z grafikâ */
    struct BitMap *bitmapa;
};

BOOL przygotujZasoby(struct Zasoby *zas, STRPTR nazwa);

VOID zwolnijZasoby(struct Zasoby *zas);

VOID narysujBelke(struct Zasoby *zas);

#endif /* SYSTEM_H */
