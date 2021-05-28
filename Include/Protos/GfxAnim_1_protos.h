
#ifndef GFXANIM_PROTOS_H
#define GFXANIM_PROTOS_H

#include <exec/types.h>

/*==================== Grafika i animacja ====================*/

typedef struct RastPort RP;
typedef struct Tile     TILE;
typedef struct MyBob    BOB;

void drawTile(RP *, TILE *, WORD x, WORD y);
void drawBOB (RP *, BOB  *, WORD x, WORD y);

#endif /* GFXANIM_PROTOS_H */
