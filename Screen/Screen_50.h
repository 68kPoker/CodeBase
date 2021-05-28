
#ifndef SCREEN_H
#define SCREEN_H

typedef struct SScreen
{
    struct Screen *Screen;
} CScreen;

BOOL openScreen (CScreen *p);
VOID closeScreen (CScreen *p);

#endif /* SCREEN_H */
