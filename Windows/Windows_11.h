
#include <exec/types.h>

struct windowData
{
    struct Window *win;
    struct Gadget *gad; /* Root gadget */
    struct Gadget *active;
    void (*onRawKey)(struct windowData *wd, UWORD code, UBYTE qualifier);
};

struct gadgetData
{
    struct Window *win;
    struct Gadget *gad;
    void (*onGadgetDown)(struct gadgetData *gd, WORD mx, WORD my);
    void (*onGadgetUp)(struct gadgetData *gd, WORD mx, WORD my);
    void (*onMouseMove)(struct gadgetData *gd, WORD mx, WORD my);
};

struct Window *openWindow(struct Screen *s, WORD left, WORD top, WORD width, WORD height, ULONG idcmp, BOOL backdrop);
void closeWindow(struct Window *w);

struct windowData *newWindowData(struct Window *w);
void disposeWindowData(struct windowData *wd);

struct Gadget *newGadget(struct windowData *wd);
void disposeGadget(struct Gadget *gad);

void handleWindow(struct IntuiMessage *msg);
