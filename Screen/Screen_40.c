
/* Open public screen */

#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

UWORD pens[NUMDRIPENS + 1] = { 0 };

struct ColorSpec colspec[] =
{
    { 0, 10, 10, 10 },
    { 1, 0, 0, 0 },
    { 2, 15, 15, 15 },
    { 3, 8, 8, 15 },
    { 4, 15, 8, 8 },
    { -1 }
};

struct Screen *openScreen()
{
    struct Screen *s;

    pens[DETAILPEN] = 2;
    pens[BLOCKPEN] = 3;
    pens[SHINEPEN] = 2;
    pens[SHADOWPEN] = 1;
    pens[BACKGROUNDPEN] = 0;
    pens[TEXTPEN] = 2;
    pens[FILLPEN] = 3;
    pens[FILLTEXTPEN] = 2;

    pens[BARDETAILPEN] = 2;
    pens[BARBLOCKPEN] = 4;
    pens[BARTRIMPEN] = 1;

    pens[NUMDRIPENS] = ~0;

    if (s = OpenScreenTags(NULL,
        SA_Left, 0,
        SA_Top, 0,
        SA_Width, 320,
        SA_Height, 256,
        SA_Depth, 5,
        SA_DisplayID, LORES_KEY,
        SA_Title, "Ekran gry Magazyn",
        SA_Pens, pens,
        SA_Colors, colspec,
        TAG_DONE))
    {
        return s;
    }
    return NULL;
}

void closeScreen(struct Screen *s)
{
    CloseScreen(s);
}

/* Back window */

struct Window *openWindow(struct Screen *s)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             s->BarHeight + 1,
        WA_Width,           s->Width,
        WA_Height,          s->Height - (s->BarHeight + 1),
        WA_Title,           "Plansza",
        WA_DragBar,         TRUE,
        WA_CloseGadget,     TRUE,
        WA_DepthGadget,     TRUE,
        WA_IDCMP,           IDCMP_CLOSEWINDOW,
        WA_SimpleRefresh,   TRUE,
        TAG_DONE))
    {
        return w;
    }
    return NULL;
}

void closeWindow(struct Window *w)
{
    CloseWindow(w);
}

main()
{
    struct Screen *s;

    if (s = openScreen())
    {
        struct Window *w;

        if (w = openWindow(s))
        {
            WaitPort(w->UserPort);
            closeWindow(w);
        }
        closeScreen(s);
    }
    return 0;
}
