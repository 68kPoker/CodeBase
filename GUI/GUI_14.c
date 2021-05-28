
/* Magazyn - GUI */

#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <datatypes/pictureclass.h>

#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>
#include <clib/datatypes_protos.h>

/* Open screens */

#define PREVIEW_TOP 134

UWORD pens[] = { ~0 };

BOOL openScreens(struct Screen *s[], struct ScreenBuffer *sb[], struct Window *w[])
{
    if (s[0] = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       640,
        SA_Height,      128,
        SA_Depth,       2,
        SA_DisplayID,   HIRES_KEY,
        SA_Pens,        pens,
        SA_Title,       "Magazyn",
        TAG_DONE))
    {
        if (s[1] = OpenScreenTags(NULL,
            SA_Parent,      s[0],
            SA_Left,        0,
            SA_Top,         PREVIEW_TOP,
            SA_Width,       320,
            SA_Height,      256,
            SA_Depth,       5,
            SA_DisplayID,   LORES_KEY,
            SA_Quiet,       TRUE,
            SA_ShowTitle,   FALSE,
            SA_Draggable,   FALSE,
            TAG_DONE))
        {
            if (w[0] = OpenWindowTags(NULL,
                WA_CustomScreen,    s[0],
                WA_Width,           s[0]->Width,
                WA_Height,          s[0]->Height - s[0]->BarHeight - 1,
                WA_DragBar,         TRUE,
                WA_CloseGadget,     TRUE,
                WA_DepthGadget,     TRUE,
                WA_IDCMP,           IDCMP_CLOSEWINDOW,
                WA_SimpleRefresh,   TRUE,
                WA_Title,           "Edytor/opcje",
                TAG_DONE))
            {
                if (w[1] = OpenWindowTags(NULL,
                    WA_CustomScreen,    s[1],
                    WA_Left,            0,
                    WA_Top,             0,
                    WA_Width,           s[1]->Width,
                    WA_Height,          s[1]->Height,
                    WA_Backdrop,        TRUE,
                    WA_Borderless,      TRUE,
                    WA_SimpleRefresh,   TRUE,
                    TAG_DONE))
                {
                    return(TRUE);
                }
                CloseWindow(w[0]);
            }
            CloseScreen(s[1]);
        }
        CloseScreen(s[0]);
    }
    return(FALSE);
}

void closeScreens(struct Screen *s[], struct Window *w[], struct ScreenBuffer *sb[])
{
    CloseWindow(w[1]);
    CloseWindow(w[0]);
    CloseScreen(s[1]);
    CloseScreen(s[0]);
}

Object *loadGraphics(struct Screen *s, STRPTR name, struct BitMap **bm)
{
    Object *o;

    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    s,
        PDTA_Remap,     FALSE,
        TAG_DONE))
    {
        ULONG *colorRegs, numColors;

        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        GetDTAttrs(o,
            PDTA_BitMap,    bm,
            PDTA_CRegs,     &colorRegs,
            PDTA_NumColors, &numColors,
            TAG_DONE);
        WORD i;
        for (i = 0; i < numColors; i++)
        {
            SetRGB32CM(s->ViewPort.ColorMap, i, colorRegs[0], colorRegs[1], colorRegs[2]);
            colorRegs += 3;
        }
        MakeScreen(s);
        RethinkDisplay();
        return(o);
    }
    return(NULL);
}

int main()
{
    struct Screen *s[2];
    struct Window *w[2];
    struct ScreenBuffer *sb[2];
    Object *o;
    struct BitMap *bm;
    WORD table[16]=  { 0, 1, 2, 3, 3, 3, 3, 3, 2, 1, 0, 0, 0, 0, 0, 0 };

    if (openScreens(s, sb, w))
    {
        if (o = loadGraphics(s[1], "Data/Magazyn.pic", &bm))
        {
            WORD i;

            for (i = 0; i < 400; i++)
            {
                BltBitMapRastPort(bm, ((i >> 2) % 6) << 4, 6 << 4, w[1]->RPort, 0, 0, 16, 16, 0xc0);
                BltBitMapRastPort(bm, ((i >> 3) % 4) << 4, 5 << 4, w[1]->RPort, 16, 0, 16, 16, 0xc0);
                BltBitMapRastPort(bm, (((i >> 2) % 2) + 4) << 4, 5 << 4, w[1]->RPort, 32, 0, 16, 16, 0xc0);
                BltBitMapRastPort(bm, table[((i >> 2) % 16)] << 4, 7 << 4, w[1]->RPort, 48, 0, 16, 16, 0xc0);
                WaitTOF();
                if (i == 200)
                    ScreenPosition(s[1], SPOS_MAKEVISIBLE|SPOS_FORCEDRAG, 0, 0, 319, 255);
            }
            DisposeDTObject(o);
        }
        closeScreens(s, w, sb);
    }
    return(0);
}
