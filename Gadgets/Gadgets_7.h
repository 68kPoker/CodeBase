
/* $Id$ */

#ifndef GADGETS_H
#define GADGETS_H

/* Includes */

#include <intuition/intuition.h>

struct gadgetInfo
{
    struct MinNode node;
    struct Gadget gad;
    void (*handleUp)(struct gadgetInfo *gi);
};

/* Prototypes */

/* Create empty gadget */

struct gadgetInfo *newGadget(void);

/* Set gadget attributes */

void setGadgetBox(struct gadgetInfo *gi, WORD left, WORD top, WORD width, WORD height);
void setGadgetID(struct gadgetInfo *gi, WORD gid);
void setGadgetHandle(struct gadgetInfo *gi, void (*handle)(struct gadgetInfo *gi));

/* Link gadgets on the list */

void linkGadgets(struct MinList *list);

void disposeGadget(struct gadgetInfo *gi);

#endif /* GADGETS_H */
