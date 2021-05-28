
#include <datatypes/pictureclass.h>
#include <clib/datatypes_protos.h>

Object *openGraphics(STRPTR name, struct Screen *s, BOOL remap)
{
    Object *o;

    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Remap,     remap,
        PDTA_Screen,    s,
        TAG_DONE))
    {
        struct BitMap *bm;

        if (!remap)
        {
            ULONG *cregs, numcolors;
            WORD i;

            GetDTAttrs(o, PDTA_CRegs, &cregs, PDTA_NumColors, &numcolors, TAG_DONE);
            for (i = 0; i < numcolors; i++)
            {
                SetRGB32CM(s->ViewPort.ColorMap, i, cregs[0], cregs[1], cregs[2]);
                cregs += 3;
            }
            MakeScreen(s);
            RethinkDisplay();
        }

        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        GetDTAttrs(o, PDTA_BitMap, &bm, TAG_DONE);

        s->UserData = (APTR)bm;
        return(o);
    }
    return(NULL);
}
