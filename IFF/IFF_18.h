
#ifndef IFF_H
#define IFF_H

#include <exec/types.h>

#define RowBytes(w) ((((w)+15)>>4)<<1) /* Row width in bytes */
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24)) /* CMAP to RGB32 */

#define MIN(a,b) ((a)<(b)?(a):(b))

/* Used for buffered reading */

#define IFF_BUFFER_SIZE 2048

struct Buffer
{
    BYTE *beg, *cur;
    LONG size, left;
};

typedef struct Graphics
{
    struct BitMap *bitmap;
    ULONG *colorsRGB32;
} GFX;

GFX *loadGraphics(STRPTR name);
VOID freeGraphics(GFX *gfx);

#endif /* IFF_H */
