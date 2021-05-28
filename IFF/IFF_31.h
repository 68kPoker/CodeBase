
#ifndef IFF_H
#define IFF_H

#include <exec/types.h>

struct BitMap *loadILBM(STRPTR name, struct ColorMap *cm);
BOOL saveBoard(struct gameBoard *gb, STRPTR name);
BOOL loadBoard(struct gameBoard *gb, STRPTR name);

#endif /* IFF_H */
