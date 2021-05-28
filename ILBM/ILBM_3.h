
#include <exec/types.h>

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c)      ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define ID_ILBM MAKE_ID('I','L','B','M')

struct ColorMap *getColorMap(struct IFFHandle *iff);
struct BitMap *getBitMap(struct IFFHandle *iff);

extern ULONG ilbmProps[];
