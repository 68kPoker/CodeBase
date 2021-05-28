
#include <stdio.h>

#include "Data.h"
#include "Video.h"
#include "Config.h"

#include <graphics/scale.h>
#include <intuition/icclass.h>
#include <datatypes/pictureclass.h>
#include <clib/datatypes_protos.h>
#include <clib/graphics_protos.h>

BOOL getBitMap(struct graphics *gfx, struct config *c)
{
    struct BitMap *bitmap;
    struct BitMapHeader *bmhd;

    GetDTAttrs(gfx->o, PDTA_BitMapHeader, &bmhd, PDTA_BitMap, &bitmap, TAG_DONE);

    if (gfx->bitmap = AllocBitMap(c->width, c->height, bmhd->bmh_Depth, 0, NULL))
    {
        struct BitScaleArgs bsa = { 0 };

        bsa.bsa_SrcX = 0;
        bsa.bsa_SrcY = 0;
        bsa.bsa_SrcWidth = bmhd->bmh_Width;
        bsa.bsa_SrcHeight = bmhd->bmh_Height;
        bsa.bsa_DestX = 0;
        bsa.bsa_DestY = 0;
        bsa.bsa_SrcBitMap = bitmap;
        bsa.bsa_DestBitMap = gfx->bitmap;
        bsa.bsa_XSrcFactor = bmhd->bmh_Width;
        bsa.bsa_XDestFactor = c->width;
        bsa.bsa_YSrcFactor = bmhd->bmh_Height;
        bsa.bsa_YDestFactor = c->height;
        BitMapScale(&bsa);
        return(TRUE);
    }
    return(FALSE);
}

BOOL loadGraphics(struct graphics *gfx, STRPTR name, struct window *w)
{
    Object *o;
    BOOL remap;
    struct Screen *screen = w->screen->screen;

    /* Remap if public screen or depth greater than required */
    if (w->config->pub || GetBitMapAttr(screen->RastPort.BitMap, BMA_DEPTH) > w->config->depth)
    {
        printf("Remapping enabled.\n");
        remap = TRUE;
    }
    else
    {
        printf("Remapping disabled.\n");
        remap = FALSE;
    }

    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    screen,
        PDTA_Remap,     remap,
        ICA_TARGET,     ICTARGET_IDCMP,
        TAG_DONE))
    {
        DoDTMethod(o, w->window, NULL, GM_LAYOUT, NULL, TRUE);

        if (!remap)
        {
            /* Load color palette */
            ULONG *cregs, numColors;

            GetDTAttrs(o, PDTA_CRegs, &cregs, PDTA_NumColors, &numColors, TAG_DONE);

            struct ColorMap *cm = screen->ViewPort.ColorMap;
            WORD i;

            for (i = 0; i < numColors; i++)
            {
                SetRGB32CM(cm, i, cregs[0], cregs[1], cregs[2]);
                cregs += 3;
            }
            MakeScreen(screen);
            RethinkDisplay();
        }
        gfx->o = o;
        gfx->window = w->window;
        return(TRUE);

    }
    else
        printf("Couldn't load %s!\n", name);
    return(FALSE);
}

VOID unloadGraphics(struct graphics *gfx)
{
    if (gfx->bitmap)
    {
        FreeBitMap(gfx->bitmap);
    }
    RemoveDTObject(gfx->window, gfx->o);
    DisposeDTObject(gfx->o);
}
