
/*
 * Data.c - Graphics data loading, remapping etc.
 */

#include <graphics/gfx.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>

struct BitMap *loadBitMap(STRPTR name, struct Screen *s)
{
    Object *o;

    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    s,
        PDTA_Remap,     FALSE,
        TAG_DONE))
    {
        struct BitMapHeader *bmhd;
        ULONG *cregs, numColors;
        WORD i;
        WORD pens[256];
        struct ColorMap *cm = s->ViewPort.ColorMap;
        struct BitMap *bm, *bitmap;
        WORD depth = GetBitMapAttr(s->RastPort.BitMap, BMA_DEPTH);

        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        GetDTAttrs(o,
            PDTA_BitMapHeader, &bmhd,
            PDTA_BitMap,    &bitmap,
            PDTA_CRegs,     &cregs,
            PDTA_NumColors, &numColors,
            TAG_DONE);

        for (i = 0; i < numColors; i++)
        {
            pens[i] = ObtainBestPen(cm, cregs[0], cregs[1], cregs[2],
                OBP_Precision,  PRECISION_EXACT,
                TAG_DONE);

            if (pens[i] != -1)
            {
                printf("Pen $%X $%X $%X obtained (%d).\n", cregs[0], cregs[1], cregs[2], pens[i]);
            }
            cregs += 3;
        }

        if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, depth, 0, s->RastPort.BitMap))
        {
            struct BitMap *auxbm;

            if (auxbm = AllocBitMap(bmhd->bmh_Width, 1, depth, 0, NULL))
            {
                UBYTE *pixels;
                LONG count = bmhd->bmh_Width * bmhd->bmh_Height;

                if (pixels = AllocMem(count, MEMF_PUBLIC))
                {
                    struct RastPort rp, auxrp;
                    LONG i;

                    InitRastPort(&rp);
                    auxrp = rp;
                    rp.BitMap = bitmap;
                    auxrp.BitMap = auxbm;
                    ReadPixelArray8(&rp, 0, 0, bmhd->bmh_Width - 1, bmhd->bmh_Height - 1, pixels, &auxrp);
                    for (i = 0; i < count; i++)
                    {
                        pixels[i] = pens[pixels[i]];
                    }
                    rp.BitMap = bm;
                    WritePixelArray8(&rp, 0, 0, bmhd->bmh_Width - 1, bmhd->bmh_Height - 1, pixels, &auxrp);
                    WaitBlit();
                    FreeMem(pixels, count);
                    FreeBitMap(auxbm);
                    DisposeDTObject(o);
                    return(bm);
                }
                FreeBitMap(auxbm);
            }
            FreeBitMap(bm);
        }
        DisposeDTObject(o);
    }
    return(NULL);
}
