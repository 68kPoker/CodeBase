
#include <stdio.h>

#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "debug.h"

struct TextAttr textAttr =
{
    "ld.font",
    8,
    FS_NORMAL,
    FPF_DISKFONT|FPF_DESIGNED
};

struct Rectangle displayClip =
{
    0, 0, 319, 255
};

ULONG modeID = LORES_KEY;

UWORD pens[] = { ~0 };

struct TextFont *font;
struct Screen   *screen;
struct Window   *window;
struct VisualInfo *vi;

BOOL openScreen()
{
    if (!(font = OpenDiskFont(&textAttr)))
    {
        printf("Couldn't open %s size %d!\n", textAttr.ta_Name, textAttr.ta_YSize);
    }
    else
    {
        if (!(screen = OpenScreenTags(NULL,
            SA_Font,        &textAttr,
            SA_DClip,       &displayClip,
            SA_DisplayID,   modeID,
            SA_Quiet,       TRUE,
            SA_ShowTitle,   FALSE,
            SA_Pens,        pens,
            SA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE)))
        {
            printf("Couldn't open screen (mode = $%X)!\n", modeID);
        }
        else
        {
            if (!(window = OpenWindowTags(NULL,
                WA_CustomScreen,    screen,
                WA_Left,            0,
                WA_Top,             0,
                WA_Width,           screen->Width,
                WA_Height,          screen->Height,
                WA_Backdrop,        TRUE,
                WA_Borderless,      TRUE,
                WA_Activate,        TRUE,
                WA_RMBTrap,         TRUE,
                WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
                WA_BackFill,        LAYERS_NOBACKFILL,
                WA_SimpleRefresh,   TRUE,
                TAG_DONE)))
            {
                printf("Couldn't open window!\n");
            }
            else
            {
                if (!(vi = GetVisualInfoA(screen, NULL)))
                {
                    printf("Couldn't get visual info!\n");
                }
                else
                {
                    D(bug("openScreen OK.\n"));
                    return(TRUE);
                }
                CloseWindow(window);
            }
            CloseScreen(screen);
        }
        CloseFont(font);
    }
    return(FALSE);
}

VOID closeScreen()
{
    FreeVisualInfo(vi);
    CloseWindow(window);
    CloseScreen(screen);
    CloseFont(font);
}

int main(void)
{
    if (openScreen())
    {
        struct RastPort *rp = window->RPort;
        Move(rp, 0, font->tf_Baseline);
        Text(rp, "Amiga!", 6);
        Delay(300);
        closeScreen();
    }
    return(0);
}
