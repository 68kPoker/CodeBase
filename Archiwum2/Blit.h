
#ifndef BLIT_H
#define BLIT_H

#include <graphics/gfx.h>

void bltRastPort(struct BitMap *iconbm, WORD iconx, WORD icony, struct BitMap *bobbm, WORD bobx, WORD boby, PLANEPTR mask, struct RastPort *destrp, WORD destx, WORD desty, WORD width, WORD height, BOOL repeatmask);
PLANEPTR makeMask(struct BitMap *bm, WORD width, WORD height, UBYTE depth);

#endif /* BLIT_H */
