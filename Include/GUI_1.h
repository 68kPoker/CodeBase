
#include <intuition/classusr.h>
#include "iffp/ilbmapp.h"

#define IDCMP IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY

#define ESC_KEY 0x45

enum {
    GID_TILE,
    GID_BOARD,
    GADGETS
};

struct GUI {
    struct TextFont *font;
    struct Screen *s;
    struct ScreenBuffer *sb[ 2 ];
    struct Window *w;
    Object *gadgets[ GADGETS ];
    struct ILBMInfo ii;

    BOOL paintMode;
    WORD tile;
    WORD maxTiles;
};
