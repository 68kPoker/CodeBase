
/* Double-buffering */

#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <intuition/screens.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfxmacros.h>
#include <graphics/videocontrol.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/utility_protos.h>

#define PRI 120 /* Copper interrupt server priority */
#define COMMANDS 10 /* User-copperlist instructions */

#define ESC_KEY 0x45

__far extern struct Custom custom;
extern LONG myCopper();

void renderScreen(struct BitMap *bm, APTR context);
struct Window *openGUI(struct Screen *parent, Object **optr);

struct DBIData
{
    struct Screen *s;
    struct BitMap *bm[2];
    WORD toggleFrame;
    BOOL safe, change;
    struct Interrupt copis;
    struct Task *task;
    WORD signal;
    struct UCopList *ucl;
};

/*
 * Add copper interrupt server.
 */
BOOL addCopper(struct DBIData *data)
{
    struct Interrupt *copis = &data->copis;

    if (data->task = FindTask(NULL))
    {
        if ((data->signal = AllocSignal(-1)) != -1)
        {
            copis->is_Node.ln_Type = NT_INTERRUPT;
            copis->is_Node.ln_Name = "AmiCore Software";
            copis->is_Node.ln_Pri  = PRI;
            copis->is_Code = (void(*)())myCopper;
            copis->is_Data = (APTR)&data->task;

            AddIntServer(INTB_COPER, copis);
            return(TRUE);
        }
    }
    return(FALSE);
}

void remCopper(struct DBIData *data)
{
    RemIntServer(INTB_COPER, &data->copis);
    FreeSignal(data->signal);
}

/*
 * Add user-copperlist
 */
BOOL addUserCopperList(struct DBIData *data)
{
    struct UCopList *ucl;

    if (data->ucl = ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
    {
        CINIT(ucl, COMMANDS);
        CWAIT(ucl, 0, 0);
        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
        CEND(ucl);

        Forbid();
        data->s->ViewPort.UCopIns = ucl;
        Permit();

        RethinkDisplay();

        return(TRUE);
    }
    return(FALSE);
}

/*
 * Open double-buffered screen.
 */
struct DBufInfo *openScreen(UBYTE depth, APTR context)
{
    struct BitMap *bm[2];
    struct ColorSpec colors[] =
    {
        { 0, 0, 0, 0 },
        { -1 }
    };

    if (bm[0] = AllocBitMap(320, 256, depth, BMF_DISPLAYABLE, NULL))
    {
        if (bm[1] = AllocBitMap(320, 256, depth, BMF_DISPLAYABLE, NULL))
        {
            struct Screen *s;
            struct TagItem vctags[] =
            {
                VC_IntermediateCLUpdate, FALSE,
                TAG_DONE
            };

            renderScreen(bm[0], context);
            renderScreen(bm[1], context);

            if (s = OpenScreenTags(NULL,
                SA_Left,        0,
                SA_Top,         0,
                SA_Width,       320,
                SA_Height,      256,
                SA_Depth,       depth,
                SA_DisplayID,   LORES_KEY,
                SA_BitMap,      bm[0],
                SA_Quiet,       TRUE,
                SA_ShowTitle,   FALSE,
                SA_Exclusive,   FALSE,
                SA_Draggable,   FALSE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                SA_Behind,      FALSE,
                SA_VideoControl,    vctags,
                SA_Colors,      colors,
                TAG_DONE))
            {
                struct DBufInfo *dbi;

                if (dbi = AllocDBufInfo(&s->ViewPort))
                {
                    struct MsgPort *mp;

                    if (mp = CreateMsgPort())
                    {
                        struct DBIData *data;

                        dbi->dbi_SafeMessage.mn_ReplyPort = mp;

                        if (data = AllocMem(sizeof(*data), MEMF_PUBLIC))
                        {
                            data->s = s;
                            data->bm[0] = bm[0];
                            data->bm[1] = bm[1];
                            data->toggleFrame = 1;
                            data->safe = data->change = TRUE;
                            dbi->dbi_UserData1 = (APTR)data;
                            if (addCopper(data))
                            {
                                if (addUserCopperList(data))
                                {
                                    if (mp = CreateMsgPort())
                                    {
                                        dbi->dbi_DispMessage.mn_ReplyPort = mp;
                                        return(dbi);
                                    }
                                }
                                remCopper(data);
                            }
                            FreeMem(data, sizeof(*data));
                        }
                        DeleteMsgPort(mp);
                    }
                    FreeDBufInfo(dbi);
                }
                CloseScreen(s);
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    return(NULL);
}

void closeScreen(struct DBufInfo *dbi)
{
    struct DBIData *data = (struct DBIData *)dbi->dbi_UserData1;
    struct MsgPort *safemp = dbi->dbi_SafeMessage.mn_ReplyPort;
    struct MsgPort *dispmp = dbi->dbi_DispMessage.mn_ReplyPort;

    remCopper(data);

    if (!data->change)
    {
        while (!GetMsg(dispmp))
        {
            WaitPort(dispmp);
        }
    }

    if (!data->safe)
    {
        while (!GetMsg(safemp))
        {
            WaitPort(safemp);
        }
    }
    DeleteMsgPort(dispmp);
    DeleteMsgPort(safemp);
    FreeDBufInfo(dbi);
    CloseScreen(data->s);
    FreeBitMap(data->bm[1]);
    FreeBitMap(data->bm[0]);
    FreeMem(data, sizeof(*data));
}

void renderScreen(struct BitMap *bm, APTR context)
{
    WORD p;
    struct Custom *cust = &custom;

    OwnBlitter();
    for (p = 0; p < bm->Depth; p++)
    {
        WaitBlit();
        cust->bltcon0 = DEST | A_TO_D;
        cust->bltcon1 = 0;
        cust->bltadat = 0x0000;
        cust->bltdpt  = bm->Planes[p];
        cust->bltdmod = 0;
        cust->bltafwm = 0xffff;
        cust->bltalwm = 0xffff;
        cust->bltsize = (bm->Rows << 6) | (bm->BytesPerRow >> 1);
    }
    DisownBlitter();
}

struct DBufInfo *openScreens(UBYTE depth, APTR context, struct Window **w, Object **optr)
{
    struct DBufInfo *dbi;
    if (dbi = openScreen(depth, context))
    {
        struct DBIData *data = (struct DBIData *)dbi->dbi_UserData1;

        if (*w = openGUI(data->s, optr))
        {
            return(dbi);
        }
        closeScreen(dbi);
    }
    return(NULL);
}

void closeScreens(struct DBufInfo *dbi, struct Window *w, Object *o)
{
    closeGUI(w, o);
    closeScreen(dbi);
}

int main()
{
    struct DBufInfo *dbi;
    struct Window *w;
    Object *o;
    struct Task *task;

    task = FindTask(NULL);
    SetTaskPri(task, 10);

    if (dbi = openScreens(5, NULL, &w, &o))
    {
        BOOL done = FALSE;
        struct DBIData *data = (struct DBIData *)dbi->dbi_UserData1;
        struct MsgPort *safemp = dbi->dbi_SafeMessage.mn_ReplyPort,
            *dispmp = dbi->dbi_DispMessage.mn_ReplyPort;

        while (!done)
        {
            ULONG mask = Wait((1L << w->UserPort->mp_SigBit) | (1L << data->signal));

            if (mask & (1L << w->UserPort->mp_SigBit))
            {
                struct IntuiMessage *msg;

                while (msg = GT_GetIMsg(w->UserPort))
                {
                    ULONG class = msg->Class;
                    WORD code = msg->Code;
                    APTR iaddress = msg->IAddress;

                    GT_ReplyIMsg(msg);
                    if (class == IDCMP_RAWKEY)
                    {
                        if (code == ESC_KEY)
                        {
                            done = TRUE;
                        }
                    }
                    else if (class == IDCMP_IDCMPUPDATE)
                    {
                        struct TagItem *tags = (struct TagItem *)iaddress, *tag;

                        while (tag = NextTagItem(&tags))
                        {
                            if (tag->ti_Tag == DTA_Sync)
                            {
                                RefreshDTObjectA(o, w, NULL, NULL);
                            }
                        }
                    }
                }
            }
            if (mask & (1L << data->signal))
            {
                struct RastPort *rp = &data->s->RastPort;
                WORD frame = data->toggleFrame;

                if (!data->safe)
                {
                    while (!GetMsg(safemp))
                    {
                        Wait(1L << safemp->mp_SigBit);
                    }
                }
                data->safe = TRUE;
                rp->BitMap = data->bm[frame];
                UBYTE text[]="0000";
                static WORD counter = 0;

                SetABPenDrMd(rp, 3, 0, JAM2);
                if (counter < data->s->Height)
                {
                    Move(rp, 0, counter);
                    Draw(rp, 319, counter);
                    if (counter >= 1)
                    {
                        Move(rp, 0, counter - 1);
                        Draw(rp, 319, counter - 1);
                    }
                }

                sprintf(text, "%4d", counter);
                Move(rp, 100, 100);
                SetABPenDrMd(rp, 2, 0, JAM2);
                Text(rp, text, 4);
                counter++;

                if (!data->change)
                {
                    while (!GetMsg(dispmp))
                    {
                        Wait(1L << dispmp->mp_SigBit);
                    }
                }
                data->change = TRUE;

                WaitBlit();

                ChangeVPBitMap(&data->s->ViewPort, data->bm[frame], dbi);
                data->toggleFrame = frame ^= 1;
                data->safe = data->change = FALSE;
            }
        }
        closeScreens(dbi, w, o);
    }
    SetTaskPri(task, 0);
    return(RETURN_OK);
}
