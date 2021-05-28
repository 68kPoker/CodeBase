
#include <exec/types.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

struct IFFHandle *openILBM(STRPTR name, BOOL *clip, LONG *err, struct BitMapHeader **bmhd);
struct ColorMap *loadColorMap(struct IFFHandle *iff, struct ColorMap *cm);
struct BitMap *loadBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd);
