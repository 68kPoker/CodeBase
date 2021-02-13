
/* Drawing and animation */

#include "DrawAnim.h"
#include <graphics/rastport.h>

#include <clib/graphics_protos.h>

BOOL initRastPort(struct rastPortUser *rpu, struct RastPort *rp)
{
    if (rpu->update[0] = NewRegion())
    {
        if (rpu->update[1] = NewRegion())
        {
            rpu->rp = rp;
            rp->RP_User = (APTR)rpu;
            rpu->bs = NULL;
            rpu->frame = 1;
            return(TRUE);
        }
        DisposeRegion(rpu->update[0]);
    }
    return(FALSE);
}

void freeRastPort(struct rastPortUser *rpu)
{
    DisposeRegion(rpu->update[1]);
    DisposeRegion(rpu->update[0]);
}

/* Update background (icon drawing) */

void drawIcons(struct RastPort *rp)
{
    UBYTE text[5];
    static WORD counter = 0;

    sprintf(text, "%4d", counter);
    SetAPen(rp, 1);
    Move(rp, 0, rp->Font->tf_Baseline);
    Text(rp, text, 4);
    counter++;

    /* Note: Add area to RastPort's update region */
}

/* Restore background below non-rectangular objects except updated */

void restoreBack(struct RastPort *rp)
{
    /* Note: Add area to RastPort's update region */
}

/* Update buffer to reflect previous changes */

void updateBuffer(struct RastPort *rp)
{
    /* Note: Clear update region */
}

/* Store background below non-rectangular objects */

void storeBack(struct RastPort *rp)
{

}

/* Draw non-rectangular objects */

void drawGels(struct RastPort *rp)
{

}

/* General drawing and animation */

void drawAnim(struct RastPort *rp)
{
    drawIcons(rp);
    restoreBack(rp);
    updateBuffer(rp);
    storeBack(rp);
    drawGels(rp);
}
