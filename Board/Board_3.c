
#include "Plansza.h"
#include "Plansza_protos.h"

/* Tworzy pustâ planszë */
BOOL nowaPlansza(struct Plansza *pl, WORD szer, WORD wys)
{
    WORD x, y;

    WORD c_x = SZER - 1, c_y = WYS - 1; /* Pozycja centrum * 2 */

    for (y = 0; y < WYS; y++)
        for (x = 0; x < SZER; x++)
        {
            POLE *p = &pl->tablica[y][x];
            WORD od_x = (x * 2) - c_x, od_y = (y * 2) - c_y;

            od_x = ABS(od_x) + 1 - szer;
            od_y = ABS(od_y) + 1 - wys;

            if (od_x <= 0 && od_y <= 0)
            {
                /* Wewnâtrz */
                p->podloga.typy[0] = PT_PODLOGA;
                if (od_x == 0 && od_y == 0)
                {
                    p->obiekt.typy[0] = OT_SCIANA;
                }
            }
            else
            {
                p->podloga.typy[0] = PT_BRAK;
            }
        }
    return(TRUE);
}
