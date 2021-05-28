
/* $Id$ */

#include <exec/types.h>

BOOL openScreen( struct screenInfo *si, ULONG *pal, struct BitMap *bm, ULONG mode, struct Rectangle *dclip );
void closeScreen( struct screenInfo *si );
