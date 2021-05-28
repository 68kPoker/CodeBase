
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>

#define DEPTH  5
#define COLORS (1 << DEPTH)
#define MODEID LORES_KEY

#define IDCMP (IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE)

ULONG colors[(COLORS * 3) + 2] = { 1 << 16 };

struct TextAttr ta =
{
    "tny.font",
    8,
    FS_NORMAL,
    FPF_DISKFONT|FPF_DESIGNED
};

struct Screen *openscreen(struct TextFont **tf)
{
    struct Screen *s;

    if (*tf = OpenDiskFont(&ta))
    {
        if (s = OpenScreenTags(NULL,
            SA_Font,        &ta,
            SA_PubName,     "MAGAZYN.1",
            SA_Title,       "Magazyn",
            SA_Left,        0,
            SA_Top,         0,
            SA_Width,       STDSCREENWIDTH,
            SA_Height,      STDSCREENHEIGHT,
            SA_Depth,       DEPTH,
            SA_DisplayID,   MODEID,
            SA_Quiet,       TRUE,
            SA_ShowTitle,   FALSE,
            SA_Exclusive,   TRUE,
            SA_Draggable,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Colors32,    colors,
            SA_SharePens,   TRUE,
            TAG_DONE))
        {
            return(s);
        }
        CloseFont(*tf);
    }
    return(NULL);
}

struct Window *openwindow(struct Screen *s)
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
        WA_IDCMP,           IDCMP,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

BOOL allocbuffers(struct ScreenBuffer *sb[], struct Screen *s)
{
    if (sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
    {
        if (sb[1] = AllocScreenBuffer(s, NULL, 0))
        {
            return(TRUE);
        }
        FreeScreenBuffer(s, sb[0]);
    }
    return(FALSE);
}

void freebuffers(struct Screen *s, struct ScreenBuffer *sb[])
{
    FreeScreenBuffer(s, sb[1]);
    FreeScreenBuffer(s, sb[0]);
}

void attachports(struct ScreenBuffer *sb, struct MsgPort *mp[])
{
    sb->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = mp[0];
    sb->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = mp[1];
}
