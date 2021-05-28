
#include <intuition/intuition.h>

#include <clib/utility_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#define DEPTH 5
#define IDCMP IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW

struct TagItem *cloneTagItems(ULONG tag1, ...)
{
    return(CloneTagItems((struct TagItem *)&tag1));
}

struct TagItem *screenTagItems()
{
    static struct Rectangle dclip = { 0, 0, 319, 255 };

    return(cloneTagItems(
        SA_DClip,       &dclip,
        SA_BitMap,      NULL,
        SA_DisplayID,   LORES_KEY,
        SA_Depth,       DEPTH,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_ShowTitle,   FALSE,
        SA_Quiet,       TRUE,
        SA_Title,       NULL,
        TAG_DONE));
}

struct TagItem *windowTagItems()
{
    return(cloneTagItems(
        WA_CustomScreen,    0,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           320,
        WA_Height,          256,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_REFRESHWINDOW,
        WA_ReportMouse,     TRUE,
        TAG_DONE));
}

void applyTagChanges(struct TagItem *tags, ULONG tag1, ...)
{
    ApplyTagChanges(tags, (struct TagItem *)&tag1);
}

struct Screen *openScreen(STRPTR title, struct BitMap *bm)
{
    struct TagItem *tags;

    if (tags = screenTagItems())
    {
        applyTagChanges(tags,
            SA_BitMap,  bm,
            SA_Title,   title,
            TAG_DONE);

        struct Screen *s = OpenScreenTagList(NULL, tags);
        FreeTagItems(tags);
        return(s);
    }
    return(NULL);
}

struct Window *openWindow(struct Screen *s, WORD left, WORD top, WORD width, WORD height, BOOL backdrop, ULONG idcmp)
{
    struct TagItem *tags;

    if (tags = windowTagItems())
    {
        applyTagChanges(tags,
            WA_CustomScreen,    s,
            WA_Left,            left,
            WA_Top,             top,
            WA_Width,           width,
            WA_Height,          height,
            WA_Backdrop,        backdrop,
            WA_IDCMP,           idcmp,
            TAG_DONE);

        struct Window *w = OpenWindowTagList(NULL, tags);
        FreeTagItems(tags);
        return(w);
    }
    return(NULL);
}

struct Window *initWindow()
{
    struct BitMap *bm;

    if (bm = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        struct Screen *s;

        if (s = openScreen("Magazyn", bm))
        {
            struct Window *w;

            s->UserData = (APTR)bm;

            if (w = openWindow(s, 0, 0, s->Width, s->Height, TRUE, IDCMP))
            {
                return(w);
            }
            CloseScreen(s);
        }
        FreeBitMap(bm);
    }
    return(NULL);
}

void freeWindow(struct Window *w)
{
    struct Screen *s = w->WScreen;
    struct BitMap *bm = (struct BitMap *)s->UserData;

    CloseWindow(w);
    CloseScreen(s);
    FreeBitMap(bm);
}

int main()
{
    struct Window *w;

    if (w = initWindow())
    {
        Delay(300);
        freeWindow(w);
    }
    return(0);
}
