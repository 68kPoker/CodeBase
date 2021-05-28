
#include <exec/types.h>

/* Wyliczenie podstawowych typów podîóg */
enum {
    PTP_TLO,
    PTP_PODLOGA,
    PTP_SCIANA,
    PTP_OZNACZONA /* Miejsce dla skrzyï */
};

/* Podstawowe typy obiektów */
enum {
    PTO_BRAK,
    PTO_SKRZYNIA,
    PTO_BOHATER
};

/* Typy kafelków w edytorze */
enum {
    TKE_TLO,
    TKE_PODLOGA,
    TKE_SCIANA,
    TKE_SKRZYNIA,
    TKE_OZNACZONA,
    TKE_TYPY
};

extern WORD podlogi[ TKE_TYPY ];
extern WORD obiekty[ TKE_TYPY ];
