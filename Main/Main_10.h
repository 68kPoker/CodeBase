
#ifndef MAIN_H
#define MAIN_H

#include "Screen.h"

#define ESC_KEY 0x45

enum
{
    SIGSRC_COPPER,
    SIGSRC_SAFE,
    SIGSRC_IDCMP1,
    SIGSRC_COUNT
};

struct mainData
{
    struct screenData screen;
};

#endif /* MAIN_H */
