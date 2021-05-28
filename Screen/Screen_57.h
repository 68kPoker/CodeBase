
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/interrupts.h>

#include "Window.h"

#define WIDTH   320
#define HEIGHT  256
#define DEPTH   5
#define MODEID  LORES_KEY

enum
{
    WID_BOARD,
    WID_MENU1,
    WID_COUNT
};

struct screenData
{
    struct TextFont *tf;
    struct BitMap *bm[2];
    struct Screen *s;
    struct MsgPort *mp;
    struct ScreenBuffer *sb[2];
    BOOL safe;
    UWORD frame; /* Obecnie wyôwietlany bufor */
    struct Interrupt copis;
    struct copperData
    {
        struct ViewPort *vp;
        UWORD signal;
        struct Task *task;
    } copData;
    struct Window *bdw, *menuw; /* Okno w tle */
    struct windowData wd[WID_COUNT];
    BOOL update;
};

BOOL openScreenFont(struct mainData *md, struct screenData *sd);
void closeScreenFont(struct screenData *sd);
BOOL openScreen(struct mainData *md, struct screenData *sd);
void closeScreen(struct screenData *sd);

BOOL openBDWindow(struct screenData *sd);
BOOL openMenuWindow(struct screenData *sd, WORD wid);
void closeWindow(struct screenData *sd, struct Window *w);

#endif /* SCREEN_H */
