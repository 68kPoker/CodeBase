
#ifndef IFF_H
#define IFF_H

#include <exec/types.h>

#include "Engine.h"

#define BUF_SIZE 4096 /* Buffer size */

#define RowBytes(w) ((((w)+15)>>4)<<1) /* Row width in bytes */
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24)) /* CMAP to RGB32 */

#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct Graphics GFX;
typedef struct Sound    SFX;
typedef struct Level    LEV;

/* Read from graphics file */
struct Graphics
{
    struct BitMap *bitmap;
    ULONG *colorsRGB32;
};

/* Used for buffered reading */
struct Buffer
{
    BYTE *beg, *cur;
    LONG size, left;
};

/* Level structure */
struct Level
{
    LONG version;
    WORD width, height;
    struct editBoard eb;
};

#endif /* IFF_H */
