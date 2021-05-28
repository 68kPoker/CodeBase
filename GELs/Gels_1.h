
#include <graphics/gfx.h>

void rysujIkone(struct BitMap *bm, WORD srcx, WORD srcy,
    struct RastPort *rp, WORD destx, WORD desty, WORD width, WORD height);

void rysujBoba(struct BitMap *bm, WORD srcx, WORD srcy, struct BitMap *back,
    WORD backx, WORD backy, struct RastPort *rp, WORD destx, WORD desty,
    WORD width, WORD height, PLANEPTR mask);
