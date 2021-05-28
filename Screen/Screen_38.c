
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/diskfont_protos.h>

#define DEPTH 5
#define ESC_KEY 0x45

/* Open double-buffered screen */

struct screenInfo
{
    struct TextFont *tf;
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *safemp;
    BOOL safe;
    WORD frame;
    Object *group, *drag, *frameimg, *o;
    struct Window *w;
    struct DrawInfo *dri;
};

UWORD pens[] = { ~0 };

struct TextAttr ta =
{
    "Dina.font", 10, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED
};

struct Screen *openScreen(ULONG modeID, struct Rectangle *dclip, UBYTE depth, struct ScreenBuffer *sb[], struct MsgPort **safemp, BOOL *safe)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_DisplayID,   modeID,
        SA_DClip,       dclip,
        SA_Depth,       depth,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Game Screen",
        SA_Pens,        pens,
        SA_Font,        &ta,
        TAG_DONE))
    {
        if (sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
        {
            if (sb[1] = AllocScreenBuffer(s, NULL, 0))
            {
                if (*safemp = CreateMsgPort())
                {
                    sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = *safemp;
                    sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = *safemp;
                    *safe = TRUE;
                    return(s);
                }
                FreeScreenBuffer(s, sb[1]);
            }
            FreeScreenBuffer(s, sb[0]);
        }
        CloseScreen(s);
    }
    return(NULL);
}

BOOL openScreenInfo(struct screenInfo *si)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (si->s = openScreen(LORES_KEY, &dclip, DEPTH, si->sb, &si->safemp, &si->safe))
    {
        si->frame = 1;
        return(TRUE);
    }
    return(FALSE);
}

VOID safeToWrite(struct screenInfo *si)
{
    if (!si->safe)
    {
        while (!GetMsg(si->safemp))
        {
            WaitPort(si->safemp);
        }
        si->safe = TRUE;
    }
}

VOID changeScreenBuffer(struct screenInfo *si)
{
    WORD frame = si->frame;

    while (!ChangeScreenBuffer(si->s, si->sb[frame]))
    {
        WaitTOF();
    }
    si->frame = frame ^ 1;
    si->safe = FALSE;
}

VOID closeScreenInfo(struct screenInfo *si)
{
    if (!si->safe)
    {
        while (!GetMsg(si->safemp))
        {
            WaitPort(si->safemp);
        }
    }
    DeleteMsgPort(si->safemp);
    FreeScreenBuffer(si->s, si->sb[1]);
    FreeScreenBuffer(si->s, si->sb[0]);
    CloseScreen(si->s);
}

BOOL openWindow(struct screenInfo *si)
{
    if (si->drag = NewObject(NULL, "buttongclass",
        GA_Left, 0,
        GA_Top,  160,
        GA_Width, 160,
        GA_Height, 16,
        GA_ID, 1,
        GA_Immediate, TRUE,
        GA_RelVerify, TRUE,
        ICA_TARGET, ICTARGET_IDCMP,
        TAG_DONE))
    {
        if (si->group = NewObject(NULL, "groupgclass",
            GA_Previous, si->drag,
            GA_Left, 0,
            GA_Top, 16,
            GA_Width, 160,
            GA_Height, 64,
            GA_ID, 2,
            GA_RelVerify, TRUE,
            TAG_DONE))
        {
            if (si->dri = GetScreenDrawInfo(si->s))
            {
                if (si->frameimg = NewObject(NULL, "frameiclass", TAG_DONE))
                {
                    if (si->o = NewObject(NULL, "frbuttonclass",
                        GA_Left, 0,
                        GA_Top, 0,
                        GA_Text, "Witaj",
                        GA_ID, 3,
                        GA_RelVerify, TRUE,
                        GA_DrawInfo, si->dri,
                        GA_Image, si->frameimg,
                        TAG_DONE))
                    {
                        DoMethod(si->group, OM_ADDMEMBER, si->o);
                        if (si->w = OpenWindowTags(NULL,
                            WA_CustomScreen,    si->s,
                            WA_Left,            0,
                            WA_Top,             0,
                            WA_Width,           si->s->Width,
                            WA_Height,          si->s->Height,
                            WA_Backdrop,        TRUE,
                            WA_Borderless,      TRUE,
                            WA_Activate,        TRUE,
                            WA_RMBTrap,         TRUE,
                            WA_BackFill,        LAYERS_NOBACKFILL,
                            WA_SimpleRefresh,   TRUE,
                            WA_IDCMP,           IDCMP_IDCMPUPDATE|IDCMP_RAWKEY|IDCMP_GADGETDOWN|IDCMP_GADGETUP,
                            WA_Gadgets,         si->drag,
                            TAG_DONE))
                        {
                            return(TRUE);
                        }
                        DisposeObject(si->o);
                    }
                    DisposeObject(si->frameimg);
                }
                FreeScreenDrawInfo(si->s, si->dri);
            }
            DisposeObject(si->group);
        }
        DisposeObject(si->drag);
    }
    return(FALSE);
}

VOID closeWindow(struct screenInfo *si)
{
    CloseWindow(si->w);
    DisposeObject(si->group);
    DisposeObject(si->frameimg);
    DisposeObject(si->drag);
    FreeScreenDrawInfo(si->s, si->dri);
}

VOID mainLoop(struct screenInfo *si)
{
    BOOL done = FALSE;
    struct IntuiMessage *msg;

    while (!done)
    {
        WaitPort(si->w->UserPort);
        while (msg = (struct IntuiMessage *)GetMsg(si->w->UserPort))
        {
            ULONG class = msg->Class;
            WORD code = msg->Code;
            APTR iaddress = msg->IAddress;
            WORD mx = msg->MouseX;
            WORD my = msg->MouseY;

            ReplyMsg((struct Message *)msg);

            if (class == IDCMP_RAWKEY)
            {
                if (code == ESC_KEY)
                {
                    done = TRUE;
                }
                else if (code == 0x50)
                {
                    struct GadgetInfo gi =
                    {
                        si->s,
                        si->w,
                        NULL,
                        si->w->RPort,
                        si->w->WLayer,
                        { 0, 0, 0, 0 },
                        { 1, 0 },
                        si->dri
                    };


                    SetGadgetAttrs((struct Gadget *)si->group, si->w, NULL,
                        GA_Left, 160,
                        TAG_DONE);

                    si->w->RPort->BitMap = si->sb[si->frame]->sb_BitMap;

                    safeToWrite(si);

                    DoGadgetMethod((struct Gadget *)si->group, si->w, NULL, GM_RENDER, &gi, si->w->RPort, GREDRAW_REDRAW);

                    changeScreenBuffer(si);

                    printf("%d\n", ((struct Gadget *)si->group)->Width);
                }
            }
            else if (class == IDCMP_IDCMPUPDATE)
            {
                struct TagItem *taglist = (struct TagItem *)iaddress, *tag;

                while (tag = NextTagItem(&taglist))
                {
                }
                Draw(si->w->RPort, mx, my);
            }
            else if (class == IDCMP_GADGETDOWN)
            {
                struct Gadget *gad = (struct Gadget *)iaddress;
                printf("Gadget Down (%d)\n", gad->GadgetID);
                if (gad->GadgetID == 1)
                {
                    SetAPen(si->w->RPort, 1);
                    Move(si->w->RPort, mx, my);
                }
            }
            else if (class == IDCMP_GADGETUP)
            {
                struct Gadget *gad = (struct Gadget *)iaddress;
                printf("Gadget Up (%d)\n", gad->GadgetID);
            }
        }
    }
}

int main(void)
{
    struct screenInfo si;

    if (si.tf = OpenDiskFont(&ta))
    {
        if (openScreenInfo(&si))
        {
            if (openWindow(&si))
            {
                mainLoop(&si);
                closeWindow(&si);
            }
            closeScreenInfo(&si);
        }
        CloseFont(si.tf);
    }
    return(0);
}
