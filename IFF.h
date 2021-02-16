
#include <exec/types.h>

struct IFFHandle *openIFF(STRPTR name, LONG mode);
void closeIFF(struct IFFHandle *iff);
BOOL scanILBM(struct IFFHandle *iff);
BOOL loadCMAP(struct IFFHandle *iff, struct Screen *s);
struct BitMap *loadBitMap(struct IFFHandle *iff);
