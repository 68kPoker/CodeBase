
#include <datatypes/pictureclass.h>
#include <intuition/intuition.h>
#include <graphics/videocontrol.h>

#include <clib/datatypes_protos.h>
#include <clib/intuition_protos.h>

enum
{
    TOOLBAR,
    BOARD,
    PREVIEW,
    OPTIONS,
    SCREENS
};

struct ColorSpec colspec[] =
{
    { 0, 0, 0, 0 },
    { -1 }
};

BOOL openScreens(struct Screen *s[], struct ScreenBuffer *sb[])
{
    struct TagItem vctags[] =
    {
        VC_NoColorPaletteLoad, TRUE,
        TAG_DONE
    };
    LONG err = 0;

    if (s[TOOLBAR] = OpenScreenTags(NULL,
        SA_Left, 0,
        SA_Top,  0,
        SA_Width, 320,
        SA_Height, 32,
        SA_Depth, 5,
        SA_DisplayID, LORES_KEY,
        SA_Quiet, TRUE,
        SA_Draggable, FALSE,
        SA_ShowTitle, FALSE,
        SA_BackFill, LAYERS_NOBACKFILL,
        SA_Colors,  colspec,
        TAG_DONE))
    {
        if (s[BOARD] = OpenScreenTags(NULL,
            SA_Left, 0,
            SA_Top, 33,
            SA_Width, 320,
            SA_Height, 224,
            SA_Depth, 5,
            SA_DisplayID, LORES_KEY,
            SA_Parent, s[TOOLBAR],
            SA_Quiet, TRUE,
            SA_VideoControl, vctags,
            SA_MinimizeISG, TRUE,
            SA_Draggable, FALSE,
            SA_ShowTitle, FALSE,
            SA_BackFill, LAYERS_NOBACKFILL,
            SA_Colors,  colspec,
            TAG_DONE))
        {
            if (s[PREVIEW] = OpenScreenTags(NULL,
                SA_Left, 0,
                SA_Top, 32,
                SA_Width, 320,
                SA_Height, 128,
                SA_Depth, 5,
                SA_DisplayID, LORES_KEY,
                SA_Parent, s[TOOLBAR],
                SA_Quiet, TRUE,
                SA_VideoControl, vctags,
                SA_MinimizeISG, TRUE,
                SA_Draggable, FALSE,
                SA_ShowTitle, FALSE,
                SA_BackFill, LAYERS_NOBACKFILL,
                SA_Behind, TRUE,
                SA_Colors,  colspec,
                TAG_DONE))
            {
                if (s[OPTIONS] = OpenScreenTags(NULL,
                    SA_Left, 0,
                    SA_Top, 160,
                    SA_Width, 640,
                    SA_Height, 96,
                    SA_Depth, 2,
                    SA_DisplayID, HIRES_KEY,
                    SA_Parent, s[TOOLBAR],
                    SA_Quiet, TRUE,
                    SA_VideoControl, vctags,
                    SA_MinimizeISG, TRUE,
                    SA_Draggable, FALSE,
                    SA_ShowTitle, FALSE,
                    SA_Behind, TRUE,
                    SA_Colors,  colspec,
                    TAG_DONE))
                {
                    if (sb[0] = AllocScreenBuffer(s[BOARD], NULL, SB_SCREEN_BITMAP))
                    {
                        if (sb[1] = AllocScreenBuffer(s[BOARD], NULL, SB_COPY_BITMAP))
                        {
                            return(TRUE);
                        }
                        FreeScreenBuffer(s[BOARD], sb[0]);
                    }
                    CloseScreen(s[OPTIONS]);
                }
                CloseScreen(s[PREVIEW]);
            }
            CloseScreen(s[BOARD]);
        }
        CloseScreen(s[TOOLBAR]);
    }
    printf("$%x\n", err);
    return(FALSE);
}

void loadColors(Object *o, struct Screen *s)
{
    ULONG *cregs, numcolors, c;

    GetDTAttrs(o,
        PDTA_CRegs, &cregs,
        PDTA_NumColors, &numcolors,
        TAG_DONE);

    cregs += 3;
    for (c = 1; c < numcolors; c++)
    {
        SetRGB32CM(s->ViewPort.ColorMap, c, cregs[0], cregs[1], cregs[2]);
        cregs += 3;
    }
    MakeScreen(s);
    RethinkDisplay();
}

void paintGraphics(Object *o, struct Screen *s[], struct ScreenBuffer *sb[])
{
    struct BitMap *bm;

    GetDTAttrs(o, PDTA_BitMap, &bm, TAG_DONE);

    BltBitMapRastPort(bm, 0, 0, &s[TOOLBAR]->RastPort, 0, 0, 16, 16, 0xc0);
    BltBitMapRastPort(bm, 0, 32, &s[TOOLBAR]->RastPort, 16, 0, 288, 16, 0xc0);
    BltBitMapRastPort(bm, 32, 0, &s[TOOLBAR]->RastPort, 304, 0, 16, 16, 0xc0);
    BltBitMapRastPort(bm, 0, 16, &s[TOOLBAR]->RastPort, 0, 16, 320, 16, 0xc0);


    WORD x, y;
    for (y = 0; y < 14; y++)
    {
        for (x = 0; x < 20; x++)
        {
            if (x == 0 || x == 19 || y == 0 || y == 13)
                BltBitMap(bm, 16, 16, sb[1]->sb_BitMap, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
            else
                BltBitMap(bm, 0, 16, sb[1]->sb_BitMap, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
        }
    }
    WaitBlit();
    while (!ChangeScreenBuffer(s[BOARD], sb[1]))
        WaitTOF();
}

BOOL loadGraphics(STRPTR name, struct Screen *s[], struct ScreenBuffer *sb[])
{
    Object *o;
    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    s[TOOLBAR],
        PDTA_Remap,     FALSE,
        TAG_DONE))
    {
        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        loadColors(o, s[TOOLBAR]);
        paintGraphics(o, s, sb);
        WaitBlit();
        DisposeObject(o);
        return(TRUE);
    }
    return(FALSE);
}

/* Create full-screen window */

struct Window *openWindow(struct Screen *s)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen, s,
        WA_Left, 0,
        WA_Top, 0,
        WA_Width, s->Width,
        WA_Height, s->Height,
        WA_Backdrop, TRUE,
        WA_Borderless, TRUE,
        WA_RMBTrap, TRUE,
        WA_Activate, TRUE,
        WA_SimpleRefresh, TRUE,
        WA_BackFill, LAYERS_NOBACKFILL,
        WA_IDCMP, IDCMP_MOUSEBUTTONS,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

BOOL openWindows(struct Window *w[], struct Screen *s[])
{
    if (w[TOOLBAR] = openWindow(s[TOOLBAR]))
    {
        return(TRUE);
    }
    return(FALSE);
}

void closeWindows(struct Window *w[])
{
    CloseWindow(w[TOOLBAR]);
}

void closeScreens(struct Screen *s[], struct ScreenBuffer *sb[])
{
    FreeScreenBuffer(s[BOARD], sb[1]);
    FreeScreenBuffer(s[BOARD], sb[0]);
    CloseScreen(s[OPTIONS]);
    CloseScreen(s[PREVIEW]);
    CloseScreen(s[BOARD]);
    CloseScreen(s[TOOLBAR]);
}

int main()
{
    struct Screen *s[SCREENS];
    struct Window *w[SCREENS];
    struct ScreenBuffer *sb[2];

    if (openScreens(s, sb))
    {
        if (loadGraphics("Data/Magazyn.pic", s, sb))
        {
            if (openWindows(w, s))
            {
                Delay(600);
                closeWindows(w);
            }
            closeScreens(s, sb);
        }
    }
    return(0);
}
