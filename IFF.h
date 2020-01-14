
#include <exec/types.h>

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

struct IFFHandle *openIFF(STRPTR name);
VOID closeIFF(struct IFFHandle *iff);
BOOL parseILBM(struct IFFHandle *iff);
struct BitMapHeader *findBMHD(struct IFFHandle *iff);
BOOL loadColorMap(struct IFFHandle *iff, struct ColorMap *cm);
struct BitMap *loadBitMap(struct IFFHandle *iff);

struct BitMap *loadPicture(STRPTR name, struct ColorMap *cm);
