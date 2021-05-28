
#ifndef WINDOWS_H
#define WINDOWS_H

#include <intuition/classusr.h>

typedef struct windowData
{
    struct Window *w;
    Object *group, *frame;
    struct DrawInfo *dri;
} *WD;

BOOL initWindow(struct Window *w, WD wd);
BOOL addItem(WD wd, STRPTR name, WORD gid, WORD left, WORD top, WORD width, WORD height);
void closeWindow(WD wd);

#endif /* WINDOWS_H */
