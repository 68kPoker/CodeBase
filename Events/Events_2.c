
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "EventSrc.h"

BOOL MyInitEventGen(struct MyEventGen *gen, struct MyEventSource *src)
{
    gen->mp[EVENTSRC_IDCMP] = src->window->UserPort;
    gen->mp[EVENTSRC_SAFE]  = src->buffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort;
    gen->mp[EVENTSRC_DISP]  = src->buffers[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort;
    gen->handler = NULL;
    gen->user    = NULL;
    gen->safe[0] = gen->safe[1] = TRUE;
    gen->src = src;
    return(TRUE);
}

VOID MyInstallEventHandler(struct MyEventGen *gen, MyEventHandler handler, APTR user)
{
    gen->handler = handler;
    gen->user    = user;
}

LONG MyEventLoop(struct MyEventGen *gen)
{
    ULONG signals[EVENTSRC_COUNT], total = 0UL, result;
    WORD i;

    for (i = 0; i < EVENTSRC_COUNT; i++)
        total |= signals[i] = 1L << gen->mp[i]->mp_SigBit;

    gen->done = FALSE;
    while (!gen->done)
    {
        result = Wait(total);
        for (i = 0; i < EVENTSRC_COUNT; i++)
        {
            if (result & signals[i])
            {
                struct MyEvent event;

                event.source = i;
                event.user = gen->user;
                event.gen = gen;
                switch (i)
                {
                    case EVENTSRC_IDCMP:
                        while ((!gen->done) && (event.data.msg = (struct IntuiMessage *)GetMsg(gen->mp[EVENTSRC_IDCMP])))
                        {
                            gen->done = gen->handler(&event);
                            ReplyMsg((struct Message *)event.data.msg);
                        }
                        break;
                    case EVENTSRC_SAFE:
                        APTR user;

                        if (!gen->safe[0])
                            while (!(user = GetMsg(gen->mp[EVENTSRC_SAFE])))
                                WaitPort(gen->mp[EVENTSRC_SAFE]);
                        gen->safe[0] = TRUE;

                        event.data.userdata = user;
                        gen->handler(&event);
                        break;
                    case EVENTSRC_DISP:
                        APTR user;

                        if (!gen->safe[1])
                            while (!(user = GetMsg(gen->mp[EVENTSRC_DISP])))
                                WaitPort(gen->mp[EVENTSRC_DISP]);
                        gen->safe[1] = TRUE;

                        event.data.userdata = user;
                        gen->handler(&event);
                        break;
                }
            }
        }
    }

    /* Clean up */
    if (!gen->safe[1])
        while (!(GetMsg(gen->mp[EVENTSRC_DISP])))
            WaitPort(gen->mp[EVENTSRC_DISP]);

    if (!gen->safe[0])
        while (!(GetMsg(gen->mp[EVENTSRC_SAFE])))
            WaitPort(gen->mp[EVENTSRC_SAFE]);

    gen->safe[0] = gen->safe[1] = TRUE;
    return(0);
}

BOOL MyInitEventSource(struct MyEventSource *src)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_Width,   SCREEN_WIDTH,
        SA_Height,  SCREEN_HEIGHT,
        SA_Depth,   SCREEN_DEPTH,
        SA_DisplayID,   SCREEN_MODEID,
        SA_Quiet,   TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        if (src->window = OpenWindowTags(NULL,
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
            WA_IDCMP,           IDCMP_RAWKEY,
            TAG_DONE))
        {
            if (src->buffers[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
            {
                if (src->buffers[1] = AllocScreenBuffer(s, NULL, SB_COPY_BITMAP))
                {
                    struct MsgPort *mp[2];

                    if (mp[0] = CreateMsgPort())
                    {
                        src->buffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = mp[0];
                        src->buffers[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = mp[0];
                        if (mp[1] = CreateMsgPort())
                        {
                            src->buffers[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = mp[1];
                            src->buffers[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = mp[1];
                            return(TRUE);
                        }
                        DeleteMsgPort(mp[0]);
                    }
                    FreeScreenBuffer(s, src->buffers[1]);
                }
                FreeScreenBuffer(s, src->buffers[0]);
            }
            CloseWindow(src->window);
        }
        CloseScreen(s);
    }
    return(FALSE);
}

VOID MyCloseEventSource(struct MyEventSource *src)
{
    struct Screen *s = src->window->WScreen;

    DeleteMsgPort(src->buffers[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort);
    DeleteMsgPort(src->buffers[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort);

    FreeScreenBuffer(s, src->buffers[1]);
    FreeScreenBuffer(s, src->buffers[0]);

    CloseWindow(src->window);
    CloseScreen(s);
}

LONG myHandler(struct MyEvent *event)
{
    if (event->source == EVENTSRC_IDCMP)
    {
        if (event->data.msg->Class == IDCMP_RAWKEY)
        {
            if (event->data.msg->Code == 0x45)
            {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

int main()
{
    struct MyEventSource src;

    if (MyInitEventSource(&src))
    {
        struct MyEventGen gen;
        if (MyInitEventGen(&gen, &src))
        {
            MyInstallEventHandler(&gen, myHandler, NULL);
            MyEventLoop(&gen);
        }
        MyCloseEventSource(&src);
    }
    return(0);
}
