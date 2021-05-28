
/* Gearwork Screen manipulation */

/* May use graphics.library Views or intuition.library Screens */

/* Provide double-buffering and Workbench Display Mode detection. */

#include <exec/types.h>

#define DEPTH 5

#define SAFE 0 /* Safe to write */
#define DISP 1 /* Safe to change */

struct GWscreen
{
    BOOL view; /* TRUE if full View, FALSE if Screen */
    union
    {
        struct GWintui
        {
            struct Screen *screen;
            struct ScreenBuffer *buffers[2];
            struct MsgPort *ports[2]; /* For Safe/Disp control */
            BOOL safe[2];
        } intui;
        /* Anchor (includes ViewExtra, ViewPort, RasInfo etc.) */
        struct View *view[2];
    } disp;
    struct BitMap *bitmaps[2]; /* Aux bitmaps */
    WORD frame; /* Current frame */
};

struct GWscreen *GWgetScreen(BOOL view); /* Get Screen or View */

VOID GWfreeScreen(VOID);
