
#include "Video.h"
#include "Config.h"

#include <datatypes/pictureclass.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>

struct TextAttr ta =
{
    "tny.font",
    8,
    FS_NORMAL,
    FPF_DISKFONT|FPF_DESIGNED
};

UWORD pens[] = {~0};

/* Open game screen */
BOOL openScreen(struct screen *s, struct config *c)
{
    s->config = c;
    if (c->pub)
    {
        /* Lock public screen */
        if (s->screen = LockPubScreen(c->pubName))
        {
            ULONG modeID = BestModeID(
                BIDTAG_ViewPort, &s->screen->ViewPort,
                BIDTAG_NominalWidth,    c->width,
                BIDTAG_NominalHeight,   c->height,
                BIDTAG_Depth,           c->depth,
                TAG_DONE);
            UnlockPubScreen(NULL, s->screen);

            if (modeID == INVALID_ID)
            {
                printf("Couldn't get screen ModeID!\n");
                return(FALSE);
            }
        }
        else
        {
            printf("Couldn't lock %s public screen!\n", c->pubName ? c->pubName : "default");
            return(FALSE);
        }
    }

    if (s->font = OpenDiskFont(&ta))
    {
        /* Open custom screen */
        if (s->screen = OpenScreenTags(NULL,
            SA_DetailPen,   21,
            SA_BlockPen,    16,
            SA_Font,        &ta,
            SA_Left,        0,
            SA_Top,         0,
            SA_Width,       STDSCREENWIDTH,
            SA_Height,      STDSCREENHEIGHT,
            SA_Depth,       c->depth,
            SA_DisplayID,   c->modeID,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Quiet,       FALSE,
            SA_Exclusive,   FALSE,
            SA_Draggable,   FALSE,
            SA_ShowTitle,   TRUE,
            SA_Title,       "Warehouse",
            SA_SharePens,   TRUE,
            SA_Pens,        pens,
            SA_FullPalette, TRUE,
            TAG_DONE))
        {
            return(TRUE);
        }
        else
            printf("Couldn't open custom screen!\n");
        CloseFont(s->font);
    }
    else
        printf("Couldn't open %s size %d!\n", ta.ta_Name, ta.ta_YSize);
    return(FALSE);
}

VOID closeScreen(struct screen *s)
{
    CloseScreen(s->screen);
    CloseFont(s->font);
}

/* Open game window (may operate on public screen) */
BOOL openWindow(struct window *w, struct screen *s, struct config *c)
{
    w->screen = s;
    w->config = c;
    if (w->window = OpenWindowTags(NULL,
        c->pub ? WA_PubScreen : WA_CustomScreen, s->screen,
        WA_Left,        0,
        WA_Top,         s->screen->BarHeight + 1,
        WA_InnerWidth,       c->width,
        WA_InnerHeight,      c->height,
        WA_Borderless,  !c->pub,
        WA_GimmeZeroZero, TRUE,
        WA_BackFill,    LAYERS_BACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,       IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_RAWKEY|IDCMP_IDCMPUPDATE,
        WA_Activate,    TRUE,
        WA_RMBTrap,     TRUE,
        WA_AutoAdjust,  FALSE,
        TAG_DONE))
    {
        return(TRUE);
    }
    else
        printf("Couldn't open window!\n");
    return(FALSE);
}

BOOL processWindow(struct window *w, struct graphics *gfx)
{
    struct Window *win = w->window;
    BOOL done = FALSE;

    while (!done)
    {
        struct IntuiMessage *msg;
        WaitPort(w->window->UserPort);

        while (msg = GT_GetIMsg(w->window->UserPort))
        {
            if (msg->Class == IDCMP_IDCMPUPDATE)
            {
                struct TagItem *tag, *taglist = (struct TagItem *)msg->IAddress;

                while (tag = NextTagItem(&taglist))
                {
                    if (tag->ti_Tag == DTA_Sync)
                    {
                        getBitMap(gfx, w->config);
                        done = TRUE;
                    }
                }
            }
            else if (msg->Class == IDCMP_RAWKEY)
            {
                if (msg->Code == 0x45)
                {
                    done = TRUE;
                }
            }
            GT_ReplyIMsg(msg);
        }
    }
    return(TRUE);
}

VOID closeWindow(struct window *w)
{
    CloseWindow(w->window);
}
