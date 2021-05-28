
#include <exec/types.h>

enum types
{
    ILBM,
    SVX
};

typedef LONG IFFERR;

IFFERR scanIFF(struct IFFHandle *iff, ULONG type);
struct IFFHandle *openIFF(STRPTR name, ULONG type);
struct ColorMap *loadCMAP(struct IFFHandle *iff);
struct BitMap *readILBM(struct IFFHandle *iff);

void closeIFF(struct IFFHandle *iff);
