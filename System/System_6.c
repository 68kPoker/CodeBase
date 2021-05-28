
/* $Header$ */

#include "Screen.h"

#include <stdio.h>

#include "debug.h"

#include <graphics/text.h>
#include <intuition/screens.h>
#include <exec/interrupts.h>

#include <clib/dos_protos.h>

#define DEPTH 8

struct screenData
{
    struct BitMap       *bm[2];
    struct TextFont     *font;
    ULONG               *pal;
    struct Screen       *s;
    struct DBufInfo     *dbi;
    BOOL                safe;
    UWORD               frame;
    struct Interrupt    is;
    struct copperData   cd;
};

struct TextAttr ta =
{
    "centurion.font",
    9,
    FS_NORMAL,
    FPF_DISKFONT | FPF_DESIGNED
};

BOOL openScreenData(struct screenData *sd)
{
    extern void myCopper(void);

    D(bug("+ Opening screen data.\n"));

    if (initBitMaps(sd->bm, 16, 256, DEPTH))
    {
        if (initFont(&sd->font, &ta))
        {
            if (sd->pal = initPal(DEPTH))
            {
                if (sd->s = openScreen("Magazyn", sd->bm, &ta, sd->pal))
                {
                    struct ViewPort *vp = &sd->s->ViewPort;

                    sd->s->UserData = (APTR)sd;

                    if (sd->dbi = addDBuf(vp, &sd->safe, &sd->frame))
                    {
                        if (addUCopList(vp))
                        {
                            if (addCopperIs(myCopper, &sd->is, &sd->cd, vp))
                            {
                                return(TRUE);
                            }
                        }
                        remDBuf(sd->dbi, sd->safe);
                    }
                    closeScreen(sd->s);
                }
                freePal(sd->pal);
            }
            closeFont(sd->font);
        }
        freeBitMaps(sd->bm);
    }
    return(FALSE);
}

VOID closeScreenData(struct screenData *sd)
{
    D(bug("- Closing screen data.\n"));

    remCopperIs(&sd->is);
    remDBuf(sd->dbi, sd->safe);
    closeScreen(sd->s);
    freePal(sd->pal);
    closeFont(sd->font);
    freeBitMaps(sd->bm);
}

int main(void)
{
    struct screenData sd;
    ULONG data[] = { 0x02020202, 0x02020202, 0x02020202, 0x02020202 };

    if (openScreenData(&sd))
    {
        extern void writePixelArray8(register __a0 ULONG *chunky, register __a1 PLANEPTR plane, register __a2 PLANEPTR eop, register __d0 WORD bpr);

        writePixelArray8(data, sd.bm[0]->Planes[0], sd.bm[0]->Planes[0] + 16, 16);

        Delay(400);
        closeScreenData(&sd);
    }
    return(0);
}
