
#include <exec/types.h>

#define scanILBM(iff) scanIFF(iff, ilbmProps, ilbmStops)
#define closeIFile(iff) closeIFF(iff, Close)
#define closeIClip(iff) closeIFF(iff, CloseClipboard)

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

struct IFFHandle *openIFF(ULONG s, void (*init)(struct IFFHandle *), LONG mode);
struct IFFHandle *openIFile(STRPTR name, LONG mode);
struct IFFHandle *openIClip(UBYTE unit, LONG mode);
void closeIFF(struct IFFHandle *iff, LONG (*close)(LONG s));

WORD countChunks(LONG *chunks);
BOOL scanIFF(struct IFFHandle *iff, LONG *props, LONG *stops);
BOOL obtainCMAP(struct IFFHandle *iff, UBYTE **cmap, WORD *colors);
void loadCMAP(struct ColorMap *cm, UBYTE *cmap, WORD colors);
struct BitMapHeader *obtainBMHD(struct IFFHandle *iff, UBYTE *cmp, WORD *bpr, WORD *rows, UBYTE *depth);
struct BitMap *loadILBM(struct IFFHandle *iff, UBYTE cmp, WORD bpr, WORD rows, UBYTE depth);
BOOL unpackILBM(BYTE *buffer, LONG size, struct BitMap *bm, UBYTE cmp, WORD bpr, WORD rows, UBYTE depth);
BOOL unpackRow(BYTE **bufptr, LONG *sizeptr, BYTE *plane, UBYTE cmp, WORD bpr);

extern LONG ilbmProps[], ilbmStops[];
