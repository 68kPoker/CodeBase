
#include <intuition/intuition.h>
#include <graphics/videocontrol.h>
#include <datatypes/pictureclass.h>
#include <intuition/icclass.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/datatypes_protos.h>

#define GUI_TOP     224
#define GUI_WIDTH   640
#define GUI_HEIGHT  64
#define GUI_DEPTH   5
#define GUI_MODEID  HIRES_KEY

struct ColorSpec colors[] = { { 0, 0, 0, 0 }, { -1 } };
UWORD pens[ NUMDRIPENS + 1 ] = { 0 };

/* Open GUI screen */

struct Window *openGUI(struct Screen *parent, Object **optr)
{
    struct Screen *s;
    struct TagItem vctags[] =
    {
        VC_NoColorPaletteLoad, TRUE,
        TAG_DONE
    };

    pens[ NUMDRIPENS ] = ~0;
    if (s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         GUI_TOP,
        SA_Width,       GUI_WIDTH,
        SA_Height,      GUI_HEIGHT,
        SA_Depth,       GUI_DEPTH,
        SA_DisplayID,   GUI_MODEID,
        SA_VideoControl,    vctags,
        SA_MinimizeISG, TRUE,
        SA_Parent,      parent,
        SA_Quiet,       TRUE,
        SA_Draggable,   FALSE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_SharePens,   TRUE,
        SA_Pens,        pens,
        SA_Colors,      colors,
        TAG_DONE))
    {
        struct Window *w;

        if (w = OpenWindowTags(NULL,
            WA_CustomScreen,    s,
            WA_Left,            0,
            WA_Top,             0,
            WA_Width,           s->Width,
            WA_Height,          s->Height,
            WA_Backdrop,        TRUE,
            WA_Borderless,      TRUE,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            WA_IDCMP,           IDCMP_RAWKEY|IDCMP_IDCMPUPDATE,
            TAG_DONE))
        {
            Object *o;

            if (o = NewDTObject("Data/Panel.pic",
                DTA_GroupID,    GID_PICTURE,
                PDTA_Screen,    s,
                PDTA_Remap,     FALSE,
                TAG_DONE))
            {
                ULONG *cregs, colors;
                WORD i;
                struct ColorMap *cm = parent->ViewPort.ColorMap;
                struct BitMap *bm;

                DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);

                GetDTAttrs(o,
                    PDTA_CRegs,  &cregs,
                    PDTA_NumColors,  &colors,
                    PDTA_BitMap,    &bm,
                    TAG_DONE);

                for (i = 0; i < colors; i++)
                {
                    SetRGB32CM(cm, i, cregs[0], cregs[1], cregs[2]);
                    cregs += 3;
                }
                RemakeDisplay();

                BltBitMapRastPort(bm, 0, 0, w->RPort, 0, 0, 640, 15, 0xc0);

                *optr = o;
                return(w);
            }
            CloseWindow(w);
        }
        CloseScreen(s);
    }
    return(NULL);
}

void closeGUI(struct Window *w, Object *o)
{
    struct Screen *s = w->WScreen;

    DisposeDTObject(o);
    CloseWindow(w);
    CloseScreen(s);
}
