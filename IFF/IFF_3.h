
#include <exec/types.h>

struct IFFHandle *openIFF(STRPTR name);
BOOL scanILBM(struct IFFHandle *iff);
struct BitMap *readILBM(struct IFFHandle *iff);
void closeIFF(struct IFFHandle *iff);
