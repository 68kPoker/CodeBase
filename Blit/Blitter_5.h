
#include <exec/types.h>
#include <graphics/gfx.h>

#define DEPTH 5

/* Used by line mode */
#define COMP   ( ABNC | NABC | NANBC )
#define NORMAL ( ABNC | NABC | NANBC | ABC)

/* Calc Word offset */
#define PlaneOffset(x, y, bpr) (((y) * bpr) + (((x) >> 4) << 1))
#define Sign(a) ((a)<0?1:0)
#define Abs(a) ((a)>=0?(a):-(a))

extern UWORD Octant(register __d0 WORD dx, register __d1 WORD dy, register __a0 UWORD *buf);
extern void LineMode(register __a0 UWORD *buf, register __a1 PLANEPTR plane, register __d0 WORD shift, register __d1 WORD minterm, register __d2 WORD bpr);
