
#include <exec/types.h>

void drawBoard(struct RastPort *rp, struct Board *board, struct BitMap *bm, struct BitMap *gfx, WORD sx, WORD sy, WORD ex, WORD ey);

void drawTile(struct BitMap *gfx, UWORD sx, UWORD sy, struct BitMap *bm, UWORD dx, UWORD dy, UWORD width, UWORD height);
BOOL cutImage(struct Image *img, struct BitMap *bm, WORD x, WORD y, WORD width, WORD height);
void freeImage(struct Image *img);

void updateStatus(struct RastPort *rp, struct Board *board);
