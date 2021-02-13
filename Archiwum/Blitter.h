
#ifndef BLITTER_H
#define BLITTER_H

#include <exec/types.h>

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define Word(x) (((x)+15)>>4) /* Oblicz wartoôê w kaflach */

/* Rysowanie kafli */

VOID drawTile(struct BitMap *srcBitMap, WORD srcX, WORD srcY, struct BitMap *destBitMap, WORD destX, WORD destY, WORD bltWidth, WORD bltHeight);
VOID drawTileLayers(struct BitMap *srcBitMap, WORD srcX, WORD srcY, struct BitMap *backBitMap, WORD backX, WORD backY, struct BitMap *destBitMap, WORD destX, WORD destY, PLANEPTR maskPlane, WORD mskX, WORD mskY, WORD maskBPR, WORD bltWidth, WORD bltHeight);

void drawTileRastPort(struct BitMap *srcBitMap, WORD srcX, WORD srcY, struct RastPort *destRPort, WORD destX, WORD destY, WORD bltWidth, WORD bltHeight);

#endif /* BLITTER_H */
