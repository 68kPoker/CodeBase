
/*
** Prostok�t mo�emy zapisa� na dwa sposoby:
** 1. Wsp��rz�dne wierzcho�kow.
** 2. Wsp��rz�dne lewego g�rnego rogu i d�ugo�� bok�w.
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
