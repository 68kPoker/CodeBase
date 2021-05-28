
#ifndef WIN_H
#define WIN_H

#include <graphics/gfx.h>

#define UPDATE 01
#define SYNC   02 /* Repeat redraw? */
#define REDRAW 04 /* Redraw whole window */

struct window
{
    struct Rectangle bounds[2]; /* Position in both buffers */
    struct window *back, *front;
    UWORD update;

    /* Draw or update the window into its bounds */
    void (*draw)(struct window *w, struct RastPort *rp, UWORD update);
};

struct screen
{
    struct Rectangle bounds;
    struct window back, *front; /* Backdrop and front-most */
};

struct rastport
{
    UWORD frame;
};

void closeWindow(struct screen *s, struct window *w, struct RastPort *rp);
void closeWindows(struct screen *s);
BOOL drawWindows(struct screen *s, struct RastPort *rp, struct window *clip, struct Region *aux);
BOOL drawWindow(struct window *w, struct RastPort *rp, UWORD update, struct Region *aux);
struct window *openWindow(struct screen *s, UWORD x, UWORD y, UWORD width, UWORD height, void (*draw)(struct window *w, struct RastPort *rp, UWORD update), struct RastPort *rp);
void initScreen(struct screen *s, UWORD width, UWORD height, void (*draw)(struct window *w, struct RastPort *rp, UWORD update), struct RastPort *rp);
BOOL initWindow(struct window *w, UWORD x, UWORD y, UWORD width, UWORD height, void (*draw)(struct window *w, struct RastPort *rp, UWORD update), struct RastPort *rp);
void initBounds(struct Rectangle *bounds, UWORD x, UWORD y, UWORD width, UWORD height);
void moveWindow(struct screen *s, struct window *w, WORD dx, WORD dy, struct RastPort *rp);

#endif /* WIN_H */
