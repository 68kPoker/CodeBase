
#include "Kafelki.h"

#define ABS(a) ((a)>=(0)?(a):-(a))

enum {
    ZEWN,
    SROD,
    WEWN
};

WORD odlegloscCentrum( WORD x, WORD y, WORD szer, WORD wys )
{
    WORD dx = ( x * 2 ) - P_SZER;
    WORD dy = ( y * 2 ) - P_WYS;

    dx = ABS( dx );
    dy = ABS( dy );

    if( dx > szer && dy > wys )
        return( ZEWN );
    else if( dx == szer && dy == wys )
        return( SROD );
    else
        return( WEWN );
}

VOID nowaPlansza( struct PlanszaInfo *pi, WORD szer, WORD wys )
{
    WORD x, y;
    WORD n;

    for( y = 0; y < P_WYS; y++ )
        for( x = 0; x < P_SZER; x++ )
            if( ( n = odlegloscCentrum( x, y, szer, wys ) ) == ZEWN )
                pi->plansza[ y ][ x ] = EK_TLO;
            else if( n == SROD )
                pi->plansza[ y ][ x ] = EK_SCIANA;
            else if( n == WEWN )
                pi->plansza[ y ][ x ] = EK_PODLOGA;

    pi->plansza[ 1 ][ 1 ] = EK_BOHATER;
    pi->bohater_x = 1;
    pi->bohater_y = 1;
}
