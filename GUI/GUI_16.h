
#include <graphics/gfx.h>

struct screenParams
{
    ULONG modeID; /* Display ID */
    struct Rectangle dclip;
    UWORD depth;
};
