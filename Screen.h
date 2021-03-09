
/* $Id$ */

#ifndef SCREEN_H
#define SCREEN_H

/* Includes */

#include <exec/types.h>
#include <exec/interrupts.h>
#include <utility/tagitem.h>

/* Defines */

#define DEPTH 5

/* Structures */

struct copperInfo
{
    struct ViewPort *viewPort;  /* Our ViewPort */
    WORD            dispSignal; /* Signal on display */
    struct Task     *sigTask;   /* This task */
};

struct screenInfo
{
    struct Screen       *screen;    /* Backward link */
    struct ScreenBuffer *buffers[2];
    struct MsgPort      *safePort;
    BOOL                safeToDraw; /* Is it safe to draw into buffer? */
    UWORD               frame;      /* Current buffer */
    struct Interrupt    dispInt;    /* Display interrupt */
    struct copperInfo   copInfo;
};

/* External symbols */

/* Prototypes */

/* Obtain standard screen tags */

extern struct TagItem *getScreenTags(void);

/* Open screen with optional custom tags */

extern struct screenInfo *openScreen(struct TagItem *base, ULONG tag1, ...);
extern void closeScreen(struct screenInfo *s);

#endif /* SCREEN_H */
