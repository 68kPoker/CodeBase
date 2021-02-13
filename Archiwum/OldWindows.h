
#ifndef WINDOWS_H
#define WINDOWS_H

#include <exec/types.h>

#include "Game.h"

#define ESC_KEY (0x45)

enum
{
    SIGNAL_WINDOW,
    SIGNAL_SAFE, /* Safe to write to screen */
    SIGNAL_COPPER,
    SIGNAL_COUNT
};


struct Window *openWindow(struct Screen *screen);
BOOL mainLoop(struct Window *win, struct boardInfo *board);

#endif /* WINDOWS_H */
