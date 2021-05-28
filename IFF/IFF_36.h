
#include <exec/types.h>

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c)      ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define SIZE        4096

/* Makro */

#define loadILBM(name, ii) workOnIFF(ii, name, MODE_OLDFILE, IFFF_READ, readILBM)

struct ILBMInfo
{
    struct ColorMap *cm;
    struct BitMap   *bm;
};

struct IFFBuffer
{
    struct IFFHandle *iff;
    BYTE *beg, *cur;
    LONG size, left;
};

/* Prototypy */

LONG workOnIFF(APTR user, STRPTR name, LONG dosmode, LONG mode, LONG (*readWriteIFF)(struct IFFHandle *iff, APTR user));
LONG readILBM(struct IFFHandle *iff, struct ILBMInfo *ii);
LONG readBitMapColors(struct IFFHandle *iff, struct ILBMInfo *ii);
LONG readColors(struct IFFHandle *iff, struct ILBMInfo *ii);
LONG readBitMap(struct IFFHandle *iff, struct ILBMInfo *ii);
LONG unpackRow(struct IFFBuffer *buf, BYTE *plane, UWORD bpr, UBYTE cmp);
ULONG readChunkBytes(struct IFFBuffer *buf, BYTE *dest, ULONG size);
