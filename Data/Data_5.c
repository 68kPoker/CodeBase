
#include <datatypes/pictureclass.h>
#include <clib/datatypes_protos.h>
#include <clib/graphics_protos.h>

#include "Blit.h"

BOOL getGraphics(struct Window *w, struct BitMap *auxbm, STRPTR name)
{
    Object *o;

    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    w->WScreen,
        PDTA_Remap,     TRUE,
        OBP_Precision,  PRECISION_EXACT,
        TAG_DONE))
    {
        struct BitMap *bm;
        WORD x, y;

        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        GetDTAttrs(o,
            PDTA_BitMap,    &bm,
            TAG_DONE);

        BltBitMap(bm, 0, 0, auxbm, 0, 0, 640, 16, 0xc0, 0xff, NULL);

        for (y = 1; y < 10; y++)
        {
            for (x = 0; x < 20; x++)
            {
                if (x == 0 || x == 19 || y == 1 || y == 9)
                {
                    BltBitMap(bm, 32, 124, auxbm, x << 5, y << 4, 32, 16, 0xc0, 0xff, NULL);
                }
                else
                {
                    BltBitMap(bm, 0, 124, auxbm, x << 5, y << 4, 32, 16, 0xc0, 0xff, NULL);
                }
            }
        }

        bltRastPort(auxbm, 0, 0, NULL, 0, 0, NULL, w->RPort, w->BorderLeft, w->BorderTop, 640, 160, 0xc0);

        /*
        BltBitMap(bm, 0, 16, auxbm, 0, 0, 48, 128, 0xc0, 0xff, NULL);
        BltBitMapRastPort(auxbm, 0, 0, w->RPort, 32, 32, 48, 128, 0xc0);
        */



        DisposeDTObject(o);
        return(TRUE);
    }
    return(FALSE);
}
