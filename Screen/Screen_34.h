
#include <exec/types.h>
#include <graphics/gfx.h>
#include <intuition/classusr.h>

/* Screen related data */

struct ScreenData
    {
    struct Screen       *Screen;
    struct ScreenBuffer *Buffers[2];
    struct MsgPort      *SafePort;
    BOOL                SafeToDraw;
    UWORD               Frame;
    struct Window       *Window;
    };

/* GUI related data */

struct WindowData
    {
    struct ScreenData *SData;
    struct Rectangle  Rect;   /* "Window" rectangle */
    Object            *Group; /* Groupgclass object */
    void              (*Draw)(struct WindowData *wd); /* Draw method */
    };

BOOL initScreen(struct ScreenData *sd);
BOOL initWindow(struct WindowData *wd, struct ScreenData *sd);
