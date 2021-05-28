
#include <exec/types.h>

struct copper_data
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screen
{
    struct BitMap *bm[ 2 ];
    struct TextFont *tf;
    struct Screen *s;
    struct Window *w;
    struct Interrupt *is;
    struct copper_data cop;
};

BOOL open_screen( struct screen *s, UWORD width, UWORD height, UBYTE depth, struct TextAttr *ta, struct Rectangle *dclip, ULONG modeID, ULONG *colors, STRPTR title, ULONG idcmp );
void close_screen( struct screen *s );
