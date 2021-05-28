
#include <exec/types.h>

#define ABS(a)   ((a)>=0?(a):-(a))
#define MIN(a,b) ((a)<=(b)?(a):(b))
#define MAX(a,b) ((a)>=(b)?(a):(b))

#define WordWidth(w) (((w)+15)>>4)

struct windowInfo
{
    struct Window *w;
    struct BitMap *back;
    WORD cx, cy;
};

BOOL openWindow(struct windowInfo *wi, struct screenInfo *si, WORD left, WORD top, WORD width, WORD height, ULONG idcmp);
void closeWindow(struct windowInfo *wi);
BOOL moveWindow(struct windowInfo *wi, WORD dx, WORD dy);
