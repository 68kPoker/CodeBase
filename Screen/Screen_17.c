
/* $Log$ */

#include <string.h>
#include <intuition/screens.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

/* Get font */
BOOL getFont(struct screenData *sd)
{
    struct TextAttr *ta = &sd->ta;

    ta->ta_Name  = "tny.font";
    ta->ta_YSize = 8;
    ta->ta_Style = FS_NORMAL;
    ta->ta_Flags = FPF_DISKFONT|FPF_DESIGNED;

    if (sd->font = OpenDiskFont(ta))
    {
        return(TRUE);
    }
    return(FALSE);
}

BOOL getColors(struct screenData *sd, UBYTE depth)
{
    WORD colors = 1 << depth;
    LONG size = (colors * 3) + 2;

    if (sd->colors = AllocMem(size * sizeof(ULONG), MEMF_PUBLIC|MEMF_CLEAR))
    {
        ULONG *pal = sd->colors;
        *pal = colors << 16;

        return(TRUE);
    }
    return(FALSE);
}

/* Obtain screen bitmap */
BOOL getBitMap(struct screenData *sd, WORD i, UWORD width, UWORD height, UBYTE depth)
{
    if (sd->bm[i] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        return(TRUE);
    }
    return(FALSE);
}

BOOL getScreen(struct screenData *sd, ULONG modeID)
{
    if (sd->s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_DisplayID,   modeID,
        SA_BitMap,      sd->bm[0],
        SA_Colors32,    sd->colors,
        SA_Font,        &sd->ta,
        SA_Exclusive,   TRUE,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        TAG_DONE))
    {
        /* Link user data */
        sd->s->UserData = (APTR)sd;
        return(TRUE);
    }
    return(FALSE);
}

BOOL getPorts(struct screenData *sd)
{
    if (sd->mp[0] = CreateMsgPort())
    {
        if (sd->mp[1] = CreateMsgPort())
        {
            sd->safe[0] = sd->safe[1] = TRUE;
            return(TRUE);
        }
        DeleteMsgPort(sd->mp[0]);
    }
    return(FALSE);
}

BOOL getScreenBuffer(struct screenData *sd, WORD i)
{
    if (sd->sb[i] = AllocScreenBuffer(sd->s, sd->bm[i], 0))
    {
        sd->sb[i]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->mp[0];
        sd->sb[i]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = sd->mp[1];
        return(TRUE);
    }
    return(FALSE);
}

void dropScreenBuffer(struct screenData *sd, WORD i)
{
    FreeScreenBuffer(sd->s, sd->sb[i]);
}

void dropPorts(struct screenData *sd)
{
    if (!sd->safe[1])
    {
        while (!GetMsg(sd->mp[1]))
        {
            WaitPort(sd->mp[1]);
        }
    }

    if (!sd->safe[0])
    {
        while (!GetMsg(sd->mp[0]))
        {
            WaitPort(sd->mp[0]);
        }
    }

    DeleteMsgPort(sd->mp[1]);
    DeleteMsgPort(sd->mp[0]);
}

void dropScreen(struct screenData *sd)
{
    CloseScreen(sd->s);
}

void dropBitMap(struct screenData *sd, WORD i)
{
    FreeBitMap(sd->bm[i]);
}

void dropColors(struct screenData *sd)
{
    UWORD colors = sd->colors[0] >> 16;
    LONG size = (colors * 3) + 2;

    FreeMem(sd->colors, size * sizeof(ULONG));
}

void dropFont(struct screenData *sd)
{
    CloseFont(sd->font);
}

BOOL getScreenData(struct screenData *sd, UWORD width, UWORD height, UBYTE depth, ULONG modeID)
{
    if (getFont(sd))
    {
        if (getColors(sd, depth))
        {
            if (getBitMap(sd, 0, width, height, depth))
            {
                if (getBitMap(sd, 1, width, height, depth))
                {
                    if (getScreen(sd, modeID))
                    {
                        if (getPorts(sd))
                        {
                            if (getScreenBuffer(sd, 0))
                            {
                                if (getScreenBuffer(sd, 1))
                                {
                                    return(TRUE);
                                }
                                dropScreenBuffer(sd, 0);
                            }
                            dropPorts(sd);
                        }
                        dropScreen(sd);
                    }
                    dropBitMap(sd, 1);
                }
                dropBitMap(sd, 0);
            }
            dropColors(sd);
        }
        dropFont(sd);
    }
    return(FALSE);
}

void dropScreenData(struct screenData *sd)
{
    dropScreenBuffer(sd, 1);
    dropScreenBuffer(sd, 0);
    dropPorts(sd);
    dropScreen(sd);
    dropBitMap(sd, 1);
    dropBitMap(sd, 0);
    dropColors(sd);
    dropFont(sd);
}

void drawText(struct animationData *ad, struct RastPort *rp)
{
    struct textInfo *ti = ad->info;

    Move(rp, ti->x, ti->y);
    Text(rp, ti->text, strlen(ti->text));
}
