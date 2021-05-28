
#ifndef WINDOW_H
#define WINDOW_H

#include "Screen.h"

#define ESC_KEY 0x45

typedef struct SWindow
{
    struct Window *Window;
} CWindow;

BOOL openBDWindow (CWindow *p, CScreen *s);
VOID closeWindow (CWindow *p);

#endif /* WINDOW_H */
