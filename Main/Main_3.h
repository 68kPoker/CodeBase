
#include "Screen.h"

#define RASWIDTH  320
#define RASHEIGHT 256
#define RASDEPTH  5
#define MODEID    LORES_KEY

enum
{
    SIG_SAFE,
    SIG_DISP,
    SIG_SOURCES
};

struct mainData
{
    struct screenData sd;
    struct textInfo ti;
    struct animationData ad;
};
