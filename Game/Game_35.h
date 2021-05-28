
#ifndef GAME_H
#define GAME_H

#include "Screen.h"
#include "Window.h"
#include "Gadget.h"

typedef struct SGame
{
    CScreen Screen;
    CWindow Window;
    CBoardGadget BoardGad;
} CGame;

BOOL initGame (CGame *p);

VOID closeGame (CGame *p);

#endif /* GAME_H */
