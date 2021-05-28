
#include "Rectangle.h"

/* Sprawdú, czy podane wspóîrzëdne leûâ wewnâtrz prostokâta */

BOOL insideRectangle (CRectangle *p, UWORD x, UWORD y)
{
    return x >= p->MinX &&
           x <= p->MaxX &&
           y >= p->MinY &&
           y <= p->MaxY;
}

VOID getPosSize (CPosSize *p, CRectangle *a)
{
    p->X      = a->MinX;
    p->Y      = a->MinY;
    p->Width  = a->MaxX - a->MinX + 1;
    p->Height = a->MaxY - a->MinY + 1;
}

VOID getRectangle (CRectangle *p, CPosSize *a)
{
    p->MinX = a->X;
    p->MinY = a->Y;
    p->MaxX = a->X + a->Width  - 1;
    p->MaxY = a->Y + a->Height - 1;
}
