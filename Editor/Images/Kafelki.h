
/* Dla edytora plansz */

#include <exec/types.h>

#define P_WYS  16
#define P_SZER 16

enum KafelTyp {
    EK_TLO,
    EK_PODLOGA,
    EK_SCIANA,
    EK_SKRZYNIA,
    EK_MIEJSCE,
    EK_DRZWI,
    EK_KLUCZ,
    EK_SKARB,
    EK_BOHATER,
    EK_TYPY
};

extern struct KafelInfo {
    STRPTR nazwa; /* Nazwa kafla (dla edytora) */
} kafle[ EK_TYPY ];

struct PlanszaInfo {
    enum KafelTyp plansza[ P_WYS ][ P_SZER ];
    WORD bohater_x, bohater_y;
};

VOID nowaPlansza( struct PlanszaInfo *pi, WORD szer, WORD wys );
