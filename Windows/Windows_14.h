
#include <exec/lists.h>

struct windowInfo
{
    struct Window *window;
    struct MinList gadList;
};

extern struct TagItem *getWindowTags(struct Screen *s);
extern struct windowInfo *openWindow(struct TagItem *base, ULONG tag1, ...);
extern void closeWindow(struct windowInfo *wi);
