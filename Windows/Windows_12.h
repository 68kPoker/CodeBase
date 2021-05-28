
#include <intuition/intuition.h>

enum windowKinds
    {
    WID_BACKDROP,
    WID_REQ
    };

enum
    {
    GID_CLOSE,
    GID_TILE,
    GID_TRIGGER,
    GID_OPTIONS,
    GID_SDRAG,
    GID_SDEPTH,
    GID_COUNT
    };

enum
    {
    TILE_HERO,
    TILE_CHERRY,
    TILE_COIN,
    TILE_WALL,
    TILE_BOX,
    TILE_FLOOR,
    TILE_FLAGSTONE,
    TILE_SKULL,
    TILE_COUNT
    };

struct gadgetInfo
    {
    struct Gadget gad;
    };

struct windowInfo
    {
    struct Window *w;
    WORD count;
    struct gadgetInfo *gads;
    BOOL paint;
    WORD tile;
    struct BitMap *gfx;
    WORD trigger;
    };

BOOL initGadget(struct windowInfo *wi, struct gadgetInfo *gi, WORD kind);
VOID linkGadgets(struct windowInfo *wi);
BOOL openWindow(struct windowInfo *wi, struct Screen *s, WORD kind);
VOID closeWindow(struct windowInfo *wi);
LONG handleWindow(struct windowInfo *wi);
