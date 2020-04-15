
/* Graphics.h */

#include <graphics/gfx.h>

#define BUFFER_SIZE   1024 /* For buffered-reads */

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 256
#define SCREEN_DEPTH  5
#define SCREEN_MODEID LORES_KEY

#define BITMAP_WIDTH  320
#define BITMAP_HEIGHT 256
#define BITMAP_DEPTH  5

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

struct gfxInfo
{
    struct BitMap *bm;
    ULONG *colors32;
    PLANEPTR mask;
};

/* Attached to screen */

struct screenInfo
{
    struct DBufInfo *dbi;
    struct MsgPort *mp[2];
    BOOL safe[2];
    struct gfxInfo *gi;
    BOOL up, down, left, right; /* Hero movement */
    WORD floor, object; /* Current floor and object types */
    struct board *board; /* Pointer to board */
};

/* Load graphics data */

BOOL loadGraphics(struct gfxInfo *gi, STRPTR name);

void unloadGraphics(struct gfxInfo *gi);

/* Create mask */

BOOL createMask(struct gfxInfo *gi);

/* Alloc and blit screen bitmap */

struct BitMap *createBitMap(struct gfxInfo *gi);

/* Open screen using bitmap and colormap */

struct Screen *openScreen(struct BitMap *bm, struct gfxInfo *gi);

void closeScreen(struct Screen *s);
