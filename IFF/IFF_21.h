
#include <exec/types.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

/* UD stands for UserData */

struct IFFUD
{
    BOOL opened; /* OpenIFF succeeded? */
    BOOL clip;
    LONG err;
    LONG *props, *stops;
};

struct ILBMUD
{
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
    struct ColorMap *cm;
    BOOL allocated; /* ColorMap status */
};

BOOL loadImage(struct ILBMUD *bmud, STRPTR name);
void unloadImage(struct ILBMUD *bmud);
