
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

#define DEPTH  5
#define MODEID LORES_KEY

int main(void);
int wbmain(struct WBStartup *wbs);
struct Screen *openScreen(void);
struct Window *openWindow(void);

extern void game(void);

struct Screen *screen;
struct Window *window;
struct BitMap *bitmaps[2] = { 0 };
struct Rectangle display = { 0, 0, 319, 255 };

struct Screen *openScreen(void)
{
    if (screen = OpenScreenTags(NULL,
        bitmaps[0] ? SA_BitMap : TAG_IGNORE, bitmaps[0],
        SA_DClip,       &display,
        SA_Depth,       DEPTH,
        SA_DisplayID,   MODEID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Magazyn",
        TAG_DONE))
    {
        return(screen);
    }
    return(NULL);
}

struct Window *openWindow(void)
{
    if (window = OpenWindowTags(NULL,
        WA_CustomScreen,    screen,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           screen->Width,
        WA_Height,          screen->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_RMBTrap,         TRUE,
        WA_Activate,        TRUE,
        WA_IDCMP,           IDCMP_RAWKEY,
        TAG_DONE))
    {
        return(window);
    }
    return(NULL);
}

int main(void)
{
    if (openScreen())
    {
        if (openWindow())
        {
            game();
            CloseWindow(window);
        }
        CloseScreen(screen);
    }
    return(RETURN_OK);
}

int wbmain(struct WBStartup *wbs)
{
    return(main());
}
