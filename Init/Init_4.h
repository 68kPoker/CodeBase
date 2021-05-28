
/*
 * $Log$
 */

#ifndef INIT_H
#define INIT_H

#include <exec/types.h>

enum
{
    NO_ERRORS,
    NO_INTUI, /* Couldn't open required libraries */
    NO_IFFPARSE,
    NO_DISKFONT,
    NO_SCREEN,
    NO_MEM,
    NO_GFXMEM,
    NO_MSGPORT,
    NO_SIGNAL,
    NO_WINDOW,
    NO_FILE,
    RESULT_ESC,
    RESULT_CLOSE,
    ERROR_CODES
};

struct initData
{
    struct TextFont *tf;
    struct Screen *s;
    struct Window *bdw, *reqw;
    struct BitMap *gfx;
};

VOID printError(ULONG err);
ULONG initAll(struct initData *id);
ULONG mainLoop(struct initData *id);
VOID cleanAll(struct initData *id);

#endif /* INIT_H */
