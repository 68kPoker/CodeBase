
#include <exec/types.h>

/* Wyliczenie podstawowych typ�w pod��g */
enum {
    PTP_TLO,
    PTP_PODLOGA,
    PTP_SCIANA,
    PTP_OZNACZONA /* Miejsce dla skrzy� */
};

/* Podstawowe typy obiekt�w */
enum {
    PTO_BRAK,
    PTO_SKRZYNIA,
    PTO_BOHATER
};

/* Typy kafelk�w w edytorze */
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
