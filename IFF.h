
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id$
*/

#ifndef IFF_H
#define IFF_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(v) ((v)|((v)<<8)|((v)<<16)|((v)<<24))

struct IFFHandle *openIFF   (STRPTR name, LONG mode);
struct IFFHandle *openILBM  (STRPTR name, struct BitMapHeader* bmhd);

void closeIFF   (struct IFFHandle* iff);
LONG scanIFF    (struct IFFHandle* iff, ULONG type, ULONG* props, WORD count);

ULONG*  loadColors(struct IFFHandle* iff, WORD* colorCount);
void    freeColors(ULONG* colors, WORD colors);

struct BitMap* loadBitMap(struct IFFHandle* iff, struct BitMapHeader* bmhd);

#endif /* IFF_H */
