
#include <exec/types.h>

#define getSafe(s, i) \
    if (!(s)->safe[i]) \
        while (!GetMsg((s)->ports[i])) \
            WaitPort((s)->ports[i]);

struct screen
{
    struct TextAttr     ta;
    struct TextFont     *font;
    ULONG               *colors;
    struct BitMap       *bm[2];
    struct Screen       *s;
    struct MsgPort      *ports[2];
    BOOL                safe[2];
    struct ScreenBuffer *buf[2];
};

struct screen *newScreen(struct TextAttr *ta);
void disposeScreen(struct screen *s);
ULONG *blankColors(struct screen *s, WORD colorCount);
struct BitMap *screenBitMap(struct screen *s, WORD width, WORD height, UBYTE depth);
struct Screen *openScreen(struct screen *s, ULONG modeid);
BOOL doubleBufPorts(struct screen *s);
struct ScreenBuffer *screenBuffer(struct screen *s, struct BitMap *bm);
