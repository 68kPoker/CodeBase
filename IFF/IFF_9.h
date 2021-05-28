
#include <exec/types.h>

#define findBMHD(iff) ((struct BitMapHeader *)findPropData(iff, ID_ILBM, ID_BMHD))
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

enum stream_types
{
    STREAM_DOS,
    STREAM_CLIP,
    STREAM_CUSTOM
};

/* Open ILBM file. Close with closeIFF. */

struct IFFHandle *openILBM(struct BitMapHeader **bmhd, ULONG stream, WORD type);
struct ColorMap *readCMAP(struct IFFHandle *iff, struct ColorMap *cm);
struct BitMap *readBitMap(struct IFFHandle *iff);

struct IFFHandle *openIFF(ULONG stream, WORD type, LONG mode);
BOOL scanIFF(struct IFFHandle *iff, ULONG *props, ULONG *colls, ULONG *stops);
void closeIFF(struct IFFHandle *iff);
