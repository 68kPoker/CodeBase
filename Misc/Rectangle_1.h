
/*
** Prostokât moûemy zapisaê na dwa sposoby:
** 1. Wspóîrzëdne wierzchoîkow.
** 2. Wspóîrzëdne lewego górnego rogu i dîugoôê boków.
*/

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <graphics/gfx.h>

typedef struct Rectangle CRectangle;

typedef struct SPosSize
{
    UWORD X, Y, Width, Height; /* Pozycja i wymiary */
} CPosSize;

BOOL insideRectangle (CRectangle *p, UWORD x, UWORD y);
VOID getPosSize (CPosSize *p, CRectangle *a);
VOID getRectangle (CRectangle *p, CPosSize *a);

#endif /* RECTANGLE_H */
