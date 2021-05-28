
#ifndef GAME_H
#define GAME_H

#include <exec/types.h>

BOOL initAll(struct systemInfo *si);
void closeAll(struct systemInfo *si);
void play(struct systemInfo *si);

#endif /* GAME_H */
