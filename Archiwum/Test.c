
#include <stdio.h>

#include "Struct.h"
#include "Enum.h"

void narysuj(struct Plansza *pl)
{
    WORD x, y;

    for( y = 0; y < PLANSZA_WYS; y++ ) {
        for( x = 0; x < PLANSZA_SZER; x++ ) {
            struct Pole *po = &pl->tab[ y ][ x ];

            switch( po->pto ) {
                case PTO_SKRZYNIA:
                    if( po->ptp == PTP_OZNACZONA)
                        putchar('*');
                    else
                        putchar('o');
                    break;
                case PTO_BOHATER:
                    putchar('@'); break;
                default:

                switch( po->ptp ) {
                    case PTP_TLO:
                        putchar(' '); break;
                    case PTP_PODLOGA:
                        putchar('·'); break;
                    case PTP_SCIANA:
                        putchar('#'); break;
                    case PTP_OZNACZONA:
                        putchar('X'); break;
                        break;
                }
            }
        }
        putchar('\n');
    }
}

int main()
{
    struct Plansza pl;

    nowaPlansza(&pl);

    pl.dy = 0;
    pl.dx = 1;

    wstawKafelek( &pl, 3, 1, TKE_SKRZYNIA );
    wstawKafelek( &pl, 5, 1, TKE_OZNACZONA );

    skanujObiekty(&pl);
    skanujObiekty(&pl);
    skanujObiekty(&pl);

    narysuj( &pl );
    return( 0 );
}
