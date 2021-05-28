
#include "screen.h"
#include "iff.h"

#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <intuition/screens.h>
#include <clib/intuition_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>

BOOL initSBuf(struct ScreenBuffer *sb[], struct DBufStatus *dbs, struct Screen *s)
{
    if (sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
        {
        if (sb[1] = AllocScreenBuffer(s, NULL, SB_COPY_BITMAP))
            {
            if (initDBuf(dbs))
                {
                bindDBuf(dbs, sb[0]->sb_DBufInfo);
                bindDBuf(dbs, sb[1]->sb_DBufInfo);
                return(TRUE);
                }
            FreeScreenBuffer(s, sb[1]);
            }
        FreeScreenBuffer(s, sb[0]);
        }
    return(FALSE);
}

VOID freeSBuf(struct ScreenBuffer *sb[], struct DBufStatus *dbs, struct Screen *s)
{
    safeDBuf(dbs, DISP);
    safeDBuf(dbs, SAFE);
    freeDBuf(dbs);
    FreeScreenBuffer(s, sb[1]);
    FreeScreenBuffer(s, sb[0]);
}

BOOL openScreen(struct screenInfo *si, struct ILBMInfo *ii)
{
    struct TextAttr ta =
        {
        "helvetica.font",
        11,
        FS_NORMAL,
        FPF_DISKFONT|FPF_DESIGNED
        };

    struct Rectangle dclip = { 0, 0, 319, 255 };
    ULONG modeID = LORES_KEY;
    UBYTE depth = 5;
    ULONG *palette;

    if (!(palette = AllocMem(((ii->cm->Count * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC)))
        {
        return(FALSE);
        }

    palette[0] = ii->cm->Count << 16;
    GetRGB32(ii->cm, 0, ii->cm->Count, palette + 1);
    palette[(ii->cm->Count * 3) + 1] = 0L;

    if (si->font = OpenDiskFont(&ta))
        {
        if (si->s = OpenScreenTags(NULL,
            SA_DClip,       &dclip,
            SA_DisplayID,   modeID,
            SA_Depth,       depth,
            SA_Quiet,       TRUE,
            SA_Exclusive,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Title,       "Game Screen",
            SA_Font,        &ta,
            SA_ShowTitle,   FALSE,
            SA_Colors32,    palette,
            TAG_DONE))
            {
            if (initSBuf(si->sb, &si->dbuf, si->s))
                {
                si->s->UserData = (APTR)si;
                FreeMem(palette, ((ii->cm->Count * 3) + 2) * sizeof(ULONG));
                return(TRUE);
                }
            CloseScreen(si->s);
            }
        CloseFont(si->font);
        }
    FreeMem(palette, ((ii->cm->Count * 3) + 2) * sizeof(ULONG));
    return(FALSE);
}

VOID closeScreen(struct screenInfo *si)
{
    freeSBuf(si->sb, &si->dbuf, si->s);
    CloseScreen(si->s);
    CloseFont(si->font);
}

VOID changeScreen(struct screenInfo *si)
{
    safeDBuf(&si->dbuf, DISP);

    while (!ChangeScreenBuffer(si->s, si->sb[si->dbuf.frame]))
        {
        WaitTOF();
        }

    si->dbuf.frame ^= 1;
    si->dbuf.safe[SAFE] = si->dbuf.safe[DISP] = FALSE;
}
