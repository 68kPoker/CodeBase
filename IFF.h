
#include <exec/types.h>

struct BitMap *LoadPicture( STRPTR name, struct ColorMap **cm );
void UnloadPicture( struct BitMap *bm, struct ColorMap *cm );
