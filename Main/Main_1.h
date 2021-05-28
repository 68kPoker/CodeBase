
#ifndef MAIN_H
#define MAIN_H

#include <exec/interrupts.h>
#include <intuition/intuition.h>

#define DEPTH   5
#define COLORS  (1 << DEPTH)

#define TITLE "Magazyn"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

/* Gadgets in main window */
enum mainGadgets
{
    MAIN_BOARDGAD,  /* Board gadget (used for editing) */
    MAIN_PANELGAD,  /* Panel gadget */
    MAIN_GADGETS
};

struct copperData
{
    struct ViewPort     *vport;

    /* Signal sent when copper interrupt occurs */
    WORD                signal;

    struct Task         *task;
};

struct mainData
{
    struct BitMap       *bitmaps[2];
    struct TextFont     *font;
    ULONG               *colors;

    struct Screen       *screen;

    /* Copper interrupt is used to sync video buffer switching */
    struct Interrupt    copInt;
    struct copperData   copData;

    struct DBufInfo     *dbi;
    struct MsgPort      *safePort;
    BOOL                safe;
    UWORD               frame;

    /* Main window */
    struct Window       *window;

    /* Main window's gadgets */
    struct Gadget       glist[MAIN_GADGETS];

    /* Game graphics */
    struct BitMap       *gfx;
};

/* Requester data */
struct reqData
{
    struct mainData     *root;

    struct Window       *window;

    /* Pointer to gadget array */
    struct Gadget       *glist;
};

BOOL initMain(struct mainData *md);
void freeMain(struct mainData *md);

#endif /* MAIN_H */
