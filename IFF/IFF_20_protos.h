
/* $Id$ */

#include <exec/types.h>

BOOL openIFF( struct IFFHandle *iff, ULONG s, BOOL dos, LONG mode );
BOOL readPalette( struct ILBMInfo *ii, struct StoredProperty *sp );
BOOL readILBM( struct ILBMInfo *ii, struct IFFHandle *iff );
BOOL readBitMap( struct ILBMInfo *ii, struct IFFHandle *iff );
LONG readChunkBytes( struct IFFHandle *iff, struct bodyBuffer *bb, BYTE *dest, WORD bytes );
BOOL unpackRow( struct IFFHandle *iff, struct bodyBuffer *bb, BYTE *dest, WORD bpr, UBYTE cmp );
