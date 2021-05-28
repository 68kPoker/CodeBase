
#ifndef IFF_H
#define IFF_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* Macros used by ILBM loading */
#define RGB(v) ((v)|((v)<<8)|((v)<<16)|((v)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

/* Load BitMap and ColorMap from ILBM file */
extern struct BitMap *loadBitMap(STRPTR name, struct ColorMap **cm);

#endif /* IFF_H */
