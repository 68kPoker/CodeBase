
/*
** Screen related
*/

#include <graphics/gfx.h>
#include <utility/tagitem.h>
#include <exec/interrupts.h>

struct copperInfo
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screenInfo
{
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp;
    BOOL safe;
    UWORD frame;
    struct Interrupt is;
    struct copperInfo ci;
};

extern struct Rectangle dclip;
extern struct TextAttr ta;
extern struct TagItem screenTags[];

struct Screen *openScreen(struct TagItem *taglist, ULONG tag1, ...);
BOOL addDBuf(struct Screen *s, struct screenInfo *si);
void closeScreen(struct Screen *s);
