
/*
** Gadûet to prostokâtny element okna, który posiada wîasnâ funkcjë
** obsîugi wydarzeï i animacji.
** Potrzebujë gadûet - planszë, który reprezentuje planszë do gry,
** z funkcjâ edycji i animacji. Ten gadûet wyôwietla zmienione kafle
** na podstawie tablicy i przekierowuje sygnaîy z myszy.
*/

#ifndef GADGET_H
#define GADGET_H

#include "Window.h" /* Okno */
#include "Board.h" /* Definicja i deklaracje metod planszy */
#include "Rectangle.h" /* Prostokât */

typedef struct SGadget
{
    CWindow    *Window; /* Okno, do którego naleûy */
    CRectangle Bounds; /* Obszar gadûetu */
} CGadget;

typedef struct SBoardGadget
{
    CGadget Base;
    CBoard  *Board; /* Wskaúnik do struktury planszy */
    BOOL EditMode;
    CTile CurTile;
    UWORD CurX, CurY;
} CBoardGadget;

VOID initBoardGadget (CBoardGadget *p, CWindow *w);
VOID editBoard (CBoardGadget *p, struct IntuiMessage *im);
VOID drawBoard (CBoardGadget *p);

#endif /* GADGET_H */
