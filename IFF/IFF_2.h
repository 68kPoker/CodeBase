
#ifndef IFF_H
#define IFF_H

#include <exec/types.h>

BOOL loadILBM(STRPTR name, struct ILBMInfo *ilbm);
void unloadILBM(struct ILBMInfo *ilbm);

#endif /* IFF_H */
