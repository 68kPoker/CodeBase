
#include <exec/interrupts.h>

#define COPPER_COMMANDS 3 /* Number of copper instructions */
#define COPPER_PRI      0 /* Copper interrupt server priority */

/* Components */

/* Screen additional information */
struct screen
{
    struct BitMap *bm[2]; /* Screen bitmaps */
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp; /* SafeMessage reply port */
    BOOL safe; /* Safe to draw? */
    WORD frame; /* Hidden frame */
    struct Interrupt is; /* Copper is used for "wake up" and buffer switching */
    struct copper /* Passed in Data */
    {
        WORD signal;
        struct Task *task;
    } cop;
    struct DrawInfo *dri;
};

/* Window additional data */
struct window
{
    struct Window *w;
};

BOOL openScreen(struct screen *s, WORD minX, WORD minY, WORD maxX, WORD maxY, ULONG modeID, ULONG *pal);
struct BitMap *obtainBitMap(struct screen *s);
void safeToDraw(struct screen *s);
void changeScreen(struct screen *s);
void closeScreen(struct screen *s);
BOOL openWindow(struct window *w, struct Screen *s, WORD minX, WORD minY, WORD maxX, WORD maxY, BOOL backdrop, BOOL activate, ULONG idcmp, struct Gadget *gads);
void closeWindow(struct window *w);
void myCopper();
