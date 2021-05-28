
#include <exec/types.h>

struct BitMap *loadBitMap(STRPTR name, struct ColorMap **cm, LONG flags, BOOL *mask);
void unloadBitMap(struct BitMap *bm, struct ColorMap *cm);
