
#ifndef WINDOWS_H
#define WINDOWS_H

#include <intuition/intuition.h>

#define MAX_TOP_GADS    5
#define MAX_LEFT_GADS   5
#define MAX_CENTER_GADS 2
#define MAX_RIGHT_GADS  1
#define MAX_BOTTOM_GADS 1

enum
{
    MID_TOP,
    MID_LEFT,
    MID_CENTER,
    MID_RIGHT,
    MID_BOTTOM
};

enum center_gads
{
    GID_PLAY,
    GID_EDIT
};

struct windowInfo
{
    struct Window *w;
    struct Gadget
        top_gads    [MAX_TOP_GADS],
        left_gads   [MAX_LEFT_GADS],
        center_gads [MAX_CENTER_GADS],
        right_gads  [MAX_RIGHT_GADS],
        bottom_gads [MAX_BOTTOM_GADS];
};

extern struct TagItem wintags[];

void initGadgets(struct windowInfo *wi);
struct Window *openWindow(struct TagItem *taglist, ULONG tag1, ...);
void addWindowInfo(struct Window *w, struct windowInfo *wi);
void addGadgets(struct windowInfo *wi);

#endif /* WINDOWS_H */
