
#include <exec/types.h>

UWORD *allocImageData(UWORD width, UWORD height, UBYTE depth);
void initImage(struct Image *img, UWORD *data, UWORD width, UWORD height, UBYTE depth, UBYTE planePick, UBYTE planeOnOff);
BOOL initBob(struct VSprite *vs, struct Bob *bob, WORD *data, UWORD width, UWORD height, UBYTE depth, UBYTE planePick, UBYTE planeOnOff);
void freeBob(struct Bob *bob);
void cutImageFromBitMap(UWORD *data, struct BitMap *bm, UWORD left, UWORD top, UWORD width, UWORD height, UBYTE depth);
