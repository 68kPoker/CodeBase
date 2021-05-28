
/* Window - container for our environment */

/* May be any window including Smart/SuperBitMap etc. */

#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "Blit.h"

extern BOOL getGraphics(struct Window *w, struct BitMap *auxbm, STRPTR name);

UWORD zoom[] = { 0, 11, 640, 245 };

struct Window *openWindow(void)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_Left,        0,
        WA_Top,         11,
        WA_InnerWidth,  640,
        WA_InnerHeight, 160,
        /* WA_Title,       "Amiga Window System", */
        WA_ScreenTitle, "Amiga Window System",
        WA_Borderless,  TRUE,
        WA_DragBar,     FALSE,
        /* WA_Zoom,        zoom, */
        WA_DepthGadget, FALSE,
        WA_SizeGadget,  FALSE,
        WA_CloseGadget, FALSE,
        WA_MaxWidth,    ~0,
        WA_MaxHeight,   ~0,
        WA_Activate,    TRUE,
        WA_SimpleRefresh,    TRUE,
        WA_IDCMP,       IDCMP_VANILLAKEY|IDCMP_REFRESHWINDOW|IDCMP_NEWSIZE|IDCMP_MOUSEBUTTONS,
        WA_GimmeZeroZero,   FALSE,
        WA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

/* Open auxilliary bitmap for fast rendering */

struct BitMap *allocBitMap(struct Window *w)
{
    struct BitMap *bm;
    if (bm = AllocBitMap(w->GZZWidth, w->GZZHeight, GetBitMapAttr(w->RPort->BitMap, BMA_DEPTH), BMF_CLEAR, w->RPort->BitMap))
    {
        return(bm);
    }
    return(NULL);
}

LONG loop(struct Window *w, struct BitMap *bm)
{
    BOOL done = FALSE;
    struct MsgPort *mp = w->UserPort;

    getGraphics(w, bm, "Data/Magazyn.pic");

    while (!done)
    {
        struct IntuiMessage *msg;
        WaitPort(mp);

        while (msg = (struct IntuiMessage *)GetMsg(mp))
        {
            if (msg->Class == IDCMP_VANILLAKEY)
            {
                if (msg->Code == 'q')
                {
                    done = TRUE;
                }
                else if (msg->Code == 'd')
                {
                    MoveWindow(w, 320, 0);
                }
                else if (msg->Code == 'a')
                {
                    MoveWindow(w, -320, 0);
                }
                else if (msg->Code == 's')
                {
                    MoveWindow(w, 0, 16);
                }
                else if (msg->Code == 'w')
                {
                    MoveWindow(w, 0, -16);
                }
                else if (msg->Code == 'z')
                {
                    static BOOL zoomed = FALSE;

                    if (!zoomed)
                        ChangeWindowBox(w, 0, 0, 640, 256);
                    else
                        ChangeWindowBox(w, 0, 0, 640, 160);

                    zoomed = !zoomed;
                }
            }
            else if (msg->Class == IDCMP_REFRESHWINDOW)
            {
                WaitTOF();
                BeginRefresh(w);
                bltRastPort(bm, 0, 0, NULL, 0, 0, NULL, w->RPort, w->BorderLeft, w->BorderTop, 640, 160, FALSE);
                EndRefresh(w, TRUE);
            }
            else if (msg->Class == IDCMP_NEWSIZE)
            {
            }
            ReplyMsg((struct Message *)msg);
        }
    }
    FreeBitMap(bm);
    return(0);
}

int main(void)
{
    struct Window *w;

    if (w = openWindow())
    {
        struct BitMap *auxbm;

        if (auxbm = allocBitMap(w))
        {
            loop(w, auxbm);
        }
        CloseWindow(w);
    }
    return(0);
}
