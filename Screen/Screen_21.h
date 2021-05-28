
/* Video functionality */

#ifndef VIDEO_H
#define VIDEO_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef INTUITION_INTUITION_H
#include <intuition/intuition.h>
#endif

#define BUF_COUNT 2

enum
{
    SAFE,
    DISP
};

enum
{
    CLOSE_GAD,
    DEPTH_GAD,
    GAD_COUNT
};

struct screen
{
    struct config *config;
    struct TextFont *font;
    struct Screen *screen; /* Intuition screen */
    struct ScreenBuffer *sb[ BUF_COUNT ]; /* Screen buffers */
    struct MsgPort *mp[ 2 ]; /* Safe/Disp message ports */
    BOOL safe[ 2 ]; /* Safe to write/change? */
};

/* Game window */
struct window
{
    struct config *config;
    struct screen *screen; /* Pointer to screen */
    struct Window *window; /* Proper game window */
    struct Gadget gads[ GAD_COUNT ]; /* Gadgets */
};

/* Open game screen */
BOOL openScreen(struct screen *s, struct config *c);
VOID closeScreen(struct screen *s);

/* Open game window (may operate on public screen) */
BOOL openWindow(struct window *w, struct screen *s, struct config *c);
BOOL processWindow(struct window *w, struct graphics *gfx);
VOID closeWindow(struct window *w);

#endif /* VIDEO_H */
