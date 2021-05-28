
#ifndef WINDOW_H
#define WINDOW_H

#include <intuition/intuition.h>

#define MAX_GADS 4

enum
{
    GID_MENU1,
    GID_MENU2,
    GID_MENU3
};

struct windowData
{
    struct Window *w;
    struct Gadget gads[MAX_GADS];
    struct IntuiText it[MAX_GADS];
    struct Border rb, sb;
    WORD selected;
};

BOOL initMenu(struct windowData *wd);

#endif /* WINDOW_H */
