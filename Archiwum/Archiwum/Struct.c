
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>

#include "Struct.h"
#include "Enum.h"

void nowaPlansza( struct Plansza *pl )
{
    WORD y;
    pl->skrzynie = 0;
    pl->ulozone = 0;

    for( y = 0; y < PLANSZA_WYS; y++ ) {
        WORD x;
        for( x = 0; x < PLANSZA_SZER; x++ ) {
            struct Pole *po = &pl->tab[ y ][ x ];
            if( x == 0 || x == PLANSZA_SZER - 1 || y == 0 || y == PLANSZA_WYS - 1 )
                po->ptp = PTP_SCIANA;
            else
                po->ptp = PTP_PODLOGA;
            po->pto = PTO_BRAK;
        }
    }
    ustawBohatera( pl, 1, 1 );
    NewList( &pl->obiekty );

    pl->bohater.x = 1;
    pl->bohater.y = 1;
    pl->bohater.pto = PTO_BOHATER;

    AddTail( &pl->obiekty, ( struct Node * ) &pl->bohater.mn );
}

void ustawBohatera( struct Plansza *pl, WORD bx, WORD by )
{
    pl->bx = bx;
    pl->by = by;

    pl->tab[ by ][ bx ].pto = PTO_BOHATER;
}

void wstawKafelek( struct Plansza *pl, WORD x, WORD y, WORD kaf )
{
    struct Pole *po = &pl->tab[ y ][ x ];

    po->ptp = podlogi[ kaf ];
    po->pto = obiekty[ kaf ];
}

struct Obiekt *dodajObiekt( struct Plansza *pl, WORD x, WORD y, struct Pole *po )
{
    struct Obiekt *ob;

    pl->tab[ y ][ x ].pto = po->pto;

    if( ob = AllocMem( sizeof( *ob ), MEMF_PUBLIC ) ) {
        ob->x = x;
        ob->y = y;
        AddTail( &pl->obiekty, ( struct Node * ) &ob->mn );
        return( ob );
    }
    return( NULL );
}

void usunObiekt( struct Obiekt *ob )
{
    Remove( ( struct Node * ) &ob->mn );
    FreeMem( ob, sizeof( *ob ) );
}

void przemiescBohatera( struct Plansza *pl, struct Obiekt *ob )
{
    WORD x = ob->x, y = ob->y;
    WORD dx = pl->dx, dy = pl->dy;
    struct Pole
        *po1 = &pl->tab[ y ][ x ],
        *po2 = &pl->tab[ y + dy ][ x + dx ],
        *po3 = &pl->tab[ y + ( dy * 2 ) ][ x + ( dx * 2 ) ];

    switch( po2->ptp ) {
        case PTP_SCIANA:
            return;
    }

    switch( po2->pto ) {
        case PTO_SKRZYNIA:
            switch( po3->ptp ) {
                case PTP_SCIANA:
                    return;
            }

            switch( po3->pto ) {
                case PTO_SKRZYNIA:
                    return;
            }
            po3->pto = po2->pto;
            po2->ob->x += dx;
            po2->ob->y += dy;

            switch( po3->ptp ) {
                case PTP_OZNACZONA:
                    pl->ulozone++;
            }

            switch( po2->ptp ) {
                case PTP_OZNACZONA:
                    pl->ulozone--;
            }
            break;

    }
    po2->pto = po1->pto;
    po1->pto = PTO_BRAK;
    ob->x += dx;
    ob->y += dy;
}

/* Skanuj listë obiektów (silnik) */
void skanujObiekty( struct Plansza *pl )
{
    struct Obiekt *ob;

    for( ob = ( struct Obiekt * ) pl->obiekty.lh_Head; ob->mn.mln_Succ != NULL; ob = ( struct Obiekt * ) ob->mn.mln_Succ ) {
        switch( ob->pto ) {
            case PTO_BOHATER:
                if( pl->dx || pl->dy )
                    przemiescBohatera( pl, ob );
                break;
        }
    }
}
