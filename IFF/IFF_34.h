
/* CIFF class */

#include <exec/types.h>
#include <datatypes/pictureclass.h>
#include <utility/hooks.h>

#define RGB( c ) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define RowBytes( w ) ((((w)+15)>>4)<<1)

enum {
    NO_MEM = 1,
    NO_FILE,
    NO_CN,
    NO_DATA,
    NO_GFXMEM,
    UNKNOWN_CMP,
    UNKNOWN_MSK
};

struct CChunk {
    struct CChunk *next;
    ULONG type, id;
    struct Hook hook;
    APTR user;
};

struct CBMHD {
    struct BitMapHeader bmhd;
};

struct CCMAP {
    struct ColorMap *cm;
};

struct CILBM {
    struct CBMHD bmhd;
    struct CCMAP cmap;
    struct BitMap *bm;
};

LONG loadILBM( STRPTR name, struct CILBM *ilbm );
void unloadILBM( struct CILBM *ilbm );

extern ULONG HookEntry();
