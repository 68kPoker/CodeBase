
#ifndef BLITTER_H
#define BLITTER_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

VOID blitIcon( struct BitMap *srcBitMap, struct BitMap *destBitMap, WORD srcX, WORD srcY, WORD destX, WORD destY, WORD width, WORD height );
VOID blitBob( struct BitMap *srcBitMap, struct BitMap *destBitMap, WORD srcX, WORD srcY, WORD destX, WORD destY, WORD width, WORD height, PLANEPTR mask );

PLANEPTR createMask( struct BitMap *bitMap );
VOID freeMask( PLANEPTR mask, struct BitMap *bitMap );

WORD blitModulo( WORD rasWidth, WORD blitWidth );

#endif /* BLITTER_H */
