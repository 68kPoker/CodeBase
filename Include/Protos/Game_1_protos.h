
#ifndef GAME_PROTOS_H
#define GAME_PROTOS_H

#include "IFF.h"
#include "WinGad.h"

BOOL changeLevel(SYSINFO *data);
void saveEditedLevel(SYSINFO *data);

void drawBoard(GFX *gfx, WIN *win, struct editBoard *eb);
WORD convertTile(struct field *f);
void animateBoard(GFX *gfx, WIN *win, struct board *bd);
void cleanBoard(struct editBoard *eb);

#endif /* GAME_PROTOS_H */
