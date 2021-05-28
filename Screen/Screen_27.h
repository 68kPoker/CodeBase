
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/interrupts.h>

#define DEPTH 5
#define COP_PRI 0
#define COP_LEN 3

typedef struct screenData
{
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp;
    BOOL safe;
    WORD frame;
    struct Interrupt is;
    struct
    {
        WORD signal;
        struct Task *task;
    } cop;
    void (*draw)(struct Screen *s, struct BitMap *bm, WORD frame);
    void (*handle)(struct Window *w, struct IntuiMessage *msg);
    struct Window *w;
} *SD;

typedef struct gadgetData
{
    void (*handle)(struct Gadget *gad, struct IntuiMessage *msg);
} *GD;

struct Screen *openScreen();
void closeScreen(struct Screen *s);
void handleScreenChange(struct Screen *s);
void handleScreenDraw(struct Screen *s);
void handleScreen(struct Screen *s, ULONG sigmask);
ULONG obtainScreenSignal(struct Screen *s);
struct Window *obtainWindow(struct Screen *s);

#endif /* SCREEN_H */
