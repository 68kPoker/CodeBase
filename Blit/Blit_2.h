
#include <exec/types.h>

void bltTileBitMap(struct BitMap *src, WORD sx, WORD sy, struct BitMap *dest, WORD dx, WORD dy, WORD width, WORD height);
void bltTileRastPort(struct BitMap *src, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height);
void bltBoardRastPort(struct BitMap *src, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height);
