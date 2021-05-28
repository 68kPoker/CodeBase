
/* Event sources */

/* RCS Log: */
/* $Log$ */

#ifndef EVENTSRC_H
#define EVENTSRC_H

#include <exec/types.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 256
#define SCREEN_DEPTH  5
#define SCREEN_MODEID LORES_KEY

enum {
    EVENTSRC_IDCMP, /* Window's UserPort */
    EVENTSRC_SAFE,  /* Safe to write into screen buffer */
    EVENTSRC_DISP,  /* Buffer was displayed at least once */
    EVENTSRC_COUNT
};

typedef LONG (*MyEventHandler)(struct MyEvent *event);

struct MyEvent {
    WORD source; /* Event source */
    union {
        struct IntuiMessage *msg;
        APTR userdata; /* Safe or disp user data */
    } data;
    APTR user;
    struct MyEventGen *gen;
};

struct MyEventGen {
    struct MsgPort *mp[EVENTSRC_COUNT];
    MyEventHandler handler;
    BOOL done;
    APTR user;
    BOOL safe[2];
    struct MyEventSource *src;
};

struct MyEventSource {
    struct Window *window; /* One or more windows */
    struct ScreenBuffer *buffers[2]; /* Double-buffered */
};

BOOL MyInitEventSource(struct MyEventSource *src);
VOID MyCloseEventSource(struct MyEventSource *src);

BOOL MyInitEventGen(struct MyEventGen *gen, struct MyEventSource *src);

VOID MyInstallEventHandler(struct MyEventGen *gen, MyEventHandler handler, APTR user);

LONG MyEventLoop(struct MyEventGen *gen);

#endif /* EVENTSRC_H */
