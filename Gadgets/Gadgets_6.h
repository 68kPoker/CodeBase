
/*
** Gad�et to prostok�tny element okna, kt�ry posiada w�asn� funkcj�
** obs�ugi wydarze� i animacji.
** Potrzebuj�gad�et - plansz�, kt�ry reprezentuje plansz�do gry,
** z funkcj� edycji i animacji. Ten gad�et wy�wietla zmienione kafle
** na podstawie tablicy i przekierowuje sygna�y z myszy.
*/

#ifndef GADGET_H
#define GADGET_H

#include "Window.h" /* Okno */
#include "Board.h" /* Definicja i deklaracje metod planszy */
#include "Rectangle.h" /* Prostok�t */

typedef struct SGadget
{
    CWindow    *Window; /* Okno, do kt�rego nale�y */
    CRectangle Bounds; /* Obszar gad�etu */
} CGadget;

typedef struct SBoardGadget
{
    CGadget Base;
    CBoard  *Board; /* Wska�nik do struktury planszy */
    BOOL EditMode;
    CTile CurTile;
    UWORD CurX, CurY;
} CBoardGadget;

VOID initBoardGadget (CBoardGadget *p, CWindow *w);
VOID editBoard (CBoardGadget *p, struct IntuiMessage *im);
VOID drawBoard (CBoardGadget *p);

#endif /* GADGET_H */
