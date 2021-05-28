
#include <dos/dos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>

#include "Screen.h"
#include "Windows.h"

int start(struct screenInfo *si, struct windowInfo *wi)
{
    ULONG signals[];
    struct IntuiMessage *msg;

    WaitPort(wi->w->UserPort);

    while (msg = GT_GetIMsg(wi->w->UserPort))
    {
        if (msg->Class == IDCMP_GADGETUP)
        {
            struct Gadget *gad = (struct Gadget *)msg->IAddress;

            printf("$%x\n", gad->GadgetID);
        }
        GT_ReplyIMsg(msg);
    }
}

int main(void)
{
    struct Screen *s;
    struct Window *w;
    struct screenInfo si;
    struct windowInfo wi;
    struct TextFont *tf;

    if (tf = OpenDiskFont(&ta))
    {
        if (s = openScreen(screenTags, TAG_DONE))
        {
            if (addDBuf(s, &si))
            {
                if (w = openWindow(wintags,
                    WA_CustomScreen,    s,
                    WA_IDCMP,           IDCMP_GADGETUP|IDCMP_RAWKEY,
                    TAG_DONE))
                {
                    addWindowInfo(w, &wi);
                    initGadgets(&wi);
                    addGadgets(&wi);
                    start(&si, &wi);
                    CloseWindow(w);
                }
            }
            closeScreen(s);
        }
        CloseFont(tf);
    }
    return(RETURN_OK);
}
