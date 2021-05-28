
#include <exec/types.h>
#include <graphics/gfx.h>

struct IFFHandle *otworzIFF(STRPTR name);
void zamknijIFF(struct IFFHandle *iff);
BOOL przeskanujILBM(struct IFFHandle *iff);

BOOL wczytajKolory(struct IFFHandle *iff, struct ColorMap *cm);
BOOL wczytajObrazek(struct IFFHandle *iff, struct BitMap *bm, PLANEPTR mask);
