
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>

#include "Game.h"

struct windowUser
    {
    APTR userData;
    LONG (*mouseButtons)(APTR user, UWORD code, WORD mx, WORD my);
    LONG (*mouseMove)(APTR user, UWORD code, WORD mx, WORD my);
    LONG (*rawKey)(APTR user, UWORD code, WORD qualifier, WORD mx, WORD my);
    LONG (*safeToDraw)(APTR user, struct RastPort *rp, WORD frame);
    };

struct screenUser
    {
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp[2];
    BOOL safe[2];
    WORD frame;
    };

LONG handleIDCMP(struct IntuiMessage *msg)
    {
    struct Window *w = msg->IDCMPWindow;
    struct windowUser *user = (struct windowUser *)w->UserData;

    switch (msg->Class)
        {
        case IDCMP_MOUSEBUTTONS:
            return(user->mouseButtons(user->userData, msg->Code, msg->MouseX, msg->MouseY));
        case IDCMP_MOUSEMOVE:
            return(user->mouseMove(user->userData, msg->Code, msg->MouseX, msg->MouseY));
        case IDCMP_RAWKEY:
            return(user->rawKey(user->userData, msg->Code, msg->Qualifier, msg->MouseX, msg->MouseY));
        case IDCMP_CLOSEWINDOW:
            return(FALSE);
        }
    return(TRUE);
    }

struct Screen *openScreen(struct screenUser *user)
    {
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_Width,   320,
        SA_Height,  256,
        SA_Depth,   5,
        SA_DisplayID,   LORES_KEY,
        SA_Title,   "Magazyn",
        SA_ShowTitle,   FALSE,
        SA_Quiet,   TRUE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
        {
        if (user->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
            {
            if (user->sb[1] = AllocScreenBuffer(s, NULL, 0))
                {
                if (user->mp[0] = CreateMsgPort())
                    {
                    if (user->mp[1] = CreateMsgPort())
                        {
                        user->safe[0] = user->safe[1] = TRUE;
                        user->frame = 1;
                        user->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = user->mp[0];
                        user->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = user->mp[0];
                        user->sb[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = user->mp[1];
                        user->sb[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = user->mp[1];
                        s->UserData = (APTR)user;
                        return(s);
                        }
                    DeleteMsgPort(user->mp[0]);
                    }
                FreeScreenBuffer(s, user->sb[1]);
                }
            FreeScreenBuffer(s, user->sb[0]);
            }
        CloseScreen(s);
        }
    return(NULL);
    }

void closeScreen(struct Screen *s)
    {
    struct screenUser *scrUser = (struct screenUser *)s->UserData;
    if (!scrUser->safe[1])
        {
        while (!GetMsg(scrUser->mp[1]))
            {
            WaitPort(scrUser->mp[1]);
            }
        }
    if (!scrUser->safe[0])
        {
        while (!GetMsg(scrUser->mp[0]))
            {
            WaitPort(scrUser->mp[0]);
            }
        }

    DeleteMsgPort(scrUser->mp[1]);
    DeleteMsgPort(scrUser->mp[0]);
    FreeScreenBuffer(s, scrUser->sb[1]);
    FreeScreenBuffer(s, scrUser->sb[0]);
    CloseScreen(s);
    }

struct Window *openWindow(struct Screen *s, struct windowUser *user)
    {
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_Width, 320,
        WA_Height, 256,
        WA_Borderless, TRUE,
        WA_Backdrop, TRUE,
        WA_ScreenTitle, "Amiga",
        WA_Activate, TRUE,
        WA_IDCMP, IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY,
        WA_ReportMouse, TRUE,
        WA_SimpleRefresh, TRUE,
        WA_CustomScreen, s,
        TAG_DONE))
        {
        w->UserData = (APTR)user;
        return(w);
        }
    return(NULL);
    }

void closeWindow(struct Window *w)
    {
    CloseWindow(w);
    }

LONG loop(struct Window *w, struct gameUser *game)
    {
    struct windowUser *winUser = (struct windowUser *)w->UserData;
    struct Screen *s = w->WScreen;
    struct screenUser *scrUser = (struct screenUser *)s->UserData;
    struct MsgPort *mp = w->UserPort;
    BOOL done = FALSE;
    ULONG signals[] =
        {
        1L << scrUser->mp[0]->mp_SigBit,
        1L << scrUser->mp[1]->mp_SigBit,
        1L << mp->mp_SigBit
        }, total;

    total = signals[0] | signals[1] | signals[2];

    winUser->mouseButtons = mouseButtons;
    winUser->mouseMove = mouseMove;
    winUser->rawKey = rawKey;
    winUser->safeToDraw = draw;
    winUser->userData = game;

    while (!ChangeScreenBuffer(s, scrUser->sb[scrUser->frame]))
        {
        WaitTOF();
        }
    scrUser->frame ^= 1;
    scrUser->safe[0] = scrUser->safe[1] = FALSE;

    while (!done)
        {
        ULONG result = Wait(total);

        if (result & signals[2])
            {
            struct IntuiMessage *msg;
            while (msg = GT_GetIMsg(mp))
                {
                done = !handleIDCMP(msg);
                GT_ReplyIMsg(msg);
                }
            }

        if (result & signals[0])
            {
            if (!scrUser->safe[0])
                {
                while (!GetMsg(scrUser->mp[0]))
                    {
                    WaitPort(scrUser->mp[0]);
                    }
                scrUser->safe[0] = TRUE;
                }
            w->RPort->BitMap = scrUser->sb[scrUser->frame]->sb_BitMap;
            winUser->safeToDraw(winUser->userData, w->RPort, scrUser->frame);
            }

        if (result & signals[1])
            {
            if (!scrUser->safe[1])
                {
                while (!GetMsg(scrUser->mp[1]))
                    {
                    WaitPort(scrUser->mp[1]);
                    }
                scrUser->safe[1] = TRUE;
                }
            WaitBlit();
            while (!ChangeScreenBuffer(s, scrUser->sb[scrUser->frame]))
                {
                WaitTOF();
                }
            scrUser->frame ^= 1;
            scrUser->safe[0] = scrUser->safe[1] = FALSE;
            }
        }
    return(0);
    }

int main(void)
    {
    struct screenUser scrUser;
    struct windowUser winUser;
    struct Screen *s;
    struct Window *w;

    struct gameUser gameUser = { 0 };

    gameUser.currentType = 1;
    winUser.userData = &gameUser;

    if (s = openScreen(&scrUser))
        {
        if (w = openWindow(s, &winUser))
            {
            loop(w, &gameUser);
            closeWindow(w);
            }
        closeScreen(s);
        }
    return(0);
    }
