
/* Drawing and animation */

#ifndef DRAW_ANIM_H
#define DRAW_ANIM_H

#include <graphics/gfx.h>

/* Background storing record */

struct backStore
{
    struct backStore *next; /* Linked list */
    struct Rectangle rect; /* Area */
    struct BitMap    *bm;  /* BitMap */
};

struct rastPortUser
{
    struct RastPort  *rp;
    struct backStore *bs;
    WORD             frame;
    struct Region    *update[2]; /* Update regions */
};

BOOL initRastPort(struct rastPortUser *rpu, struct RastPort *rp);
void freeRastPort(struct rastPortUser *rpu);

void drawAnim(struct RastPort *rp);

#endif /* DRAW_ANIM_H */
