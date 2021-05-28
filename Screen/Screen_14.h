
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

struct Screen *openGameScreen();
void closeGameScreen(struct Screen *s);

ULONG safeSignal(struct Screen *s);
ULONG copperSignal(struct Screen *s);

struct BitMap *getBitMap(struct Screen *s, UWORD *frame);
void changeBitMap(struct Screen *s);

#endif /* SCREEN_H */
