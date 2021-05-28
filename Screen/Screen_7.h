
#include <exec/interrupts.h>

#define DEPTH 5

struct window
{
    struct Window *w;
};

struct windowParam
{
    struct Screen *s;
    WORD left, top, width, height;
    BOOL backdrop;
    ULONG idcmp;
};

struct copper
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screen
{
    struct TextFont *tf;
    struct BitMap *bm[2];
    struct Screen *s;
    struct DBufInfo *dbi;
    struct MsgPort *safemp;
    BOOL safe;
    UWORD frame;
    struct Interrupt is;
    struct copper cop;
    struct window bdw;
};

struct screenParam
{
    struct TextAttr *ta;
    ULONG *colors;
    STRPTR name;
};

struct Screen *openScreen(struct screenParam *p, struct screen *s);
struct Window *openWindow(struct windowParam *p, struct window *w);
void closeScreen(struct screen *s);
void closeWindow(struct window *w);

struct Screen *prepScreen(struct screen *s);
void freeScreen(struct screen *s);
