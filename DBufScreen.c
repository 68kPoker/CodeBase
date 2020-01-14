
/* Double-buffered screen */

#include <stdio.h>

#include <intuition/screens.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include "DBufScreen.h"

struct Screen *openScreen(WORD width, WORD height, UBYTE depth, ULONG modeid, struct BitMap *bm[], struct TextAttr *ta, ULONG *colors, struct BitMap *gfx)
{
    struct Screen *s;

    renderScreen(gfx, bm);
    WaitBlit();

    if (s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       width,
        SA_Height,      height,
        SA_Depth,       depth,
        SA_DisplayID,   modeid,
        SA_BitMap,      bm[0],
        SA_Font,        ta,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Colors32,    colors,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        struct DBufInfo *dbi;

        if (dbi = AllocDBufInfo(&s->ViewPort))
        {
            struct MsgPort *mp[2];

            if (mp[0] = CreateMsgPort())
            {
                if (mp[1] = CreateMsgPort())
                {
                    struct screenUser *su;

                    if (su = AllocMem(sizeof *su, MEMF_PUBLIC))
                    {
                        su->gfx   = gfx;
                        su->dbi   = dbi;

                        su->mp[0] = mp[0];
                        su->mp[1] = mp[1];

                        su->bm[0] = bm[0];
                        su->bm[1] = bm[1];

                        dbi->dbi_SafeMessage.mn_ReplyPort = mp[0];
                        dbi->dbi_DispMessage.mn_ReplyPort = mp[1];

                        su->safeToWrite = su->safeToChange = TRUE;

                        su->toggleFrame = 1;

                        NewList(&su->blits);
                        NewList(&su->rep); /* Repeat queue */

                        s->UserData = (APTR)su;
                        return(s);
                    }
                    else
                        printf("Couldn't alloc memory!\n");
                    DeleteMsgPort(mp[1]);
                }
                else
                    printf("Couldn't create msgport!\n");
                DeleteMsgPort(mp[0]);
            }
            else
                printf("Couldn't create msgport!\n");
            FreeDBufInfo(dbi);
        }
        else
            printf("Couldn't alloc dbufinfo!\n");
        CloseScreen(s);
    }
    else
        printf("Couldn't open screen!\n");

    return(NULL);
}

void closeScreen(struct Screen *s)
{
    struct screenUser *su = (struct screenUser *)s->UserData;
    struct blitOp *op, *next;

    if (!su->safeToChange)
        while (!GetMsg(su->mp[1]))
            Wait(1L << su->mp[1]->mp_SigBit);

    if (!su->safeToWrite)
        while (!GetMsg(su->mp[0]))
            Wait(1L << su->mp[0]->mp_SigBit);

    DeleteMsgPort(su->mp[1]);
    DeleteMsgPort(su->mp[0]);
    FreeDBufInfo(su->dbi);
    CloseScreen(s);

    op = (struct blitOp *)su->rep.lh_Head;
    while (next = (struct blitOp *)op->node.ln_Succ)
    {
        Remove(&op->node);
        FreeMem(op, sizeof(*op));
        op = next;
    }

    op = (struct blitOp *)su->blits.lh_Head;
    while (next = (struct blitOp *)op->node.ln_Succ)
    {
        Remove(&op->node);
        FreeMem(op, sizeof(*op));
        op = next;
    }

    FreeMem(su, sizeof *su);
}

void drawScreen(struct Screen *s)
{
    struct screenUser *su = (struct screenUser *)s->UserData;
    static WORD counter = 0;
    UBYTE text[5];

    if (!su->safeToWrite)
        while (!GetMsg(su->mp[0]))
            Wait(1L << su->mp[0]->mp_SigBit);

    su->safeToWrite = TRUE;

    /* Draw next screen frame here */

    struct RastPort rastport, *rp = &rastport;
    struct TextFont *tf = s->RastPort.Font;
    UBYTE toggleFrame = su->toggleFrame;

    InitRastPort(&rastport);
    rastport.BitMap = su->bm[toggleFrame];

    struct blitOp *op, *next;

    for (op = (struct blitOp *)su->rep.lh_Head; (next = (struct blitOp *)op->node.ln_Succ) != NULL; op = next)
    {
        if (op->type == DRAWICON)
        {
            BltBitMap(su->gfx, (op->tile % 20) << 4, (op->tile / 20) << 4, rastport.BitMap, op->x << 4, op->y << 4, op->width, op->height, 0xc0, 0xff, NULL);
            Remove(&op->node);
            FreeMem(op, sizeof(*op));
        }
    }

    for (op = (struct blitOp *)su->blits.lh_Head; (next = (struct blitOp *)op->node.ln_Succ) != NULL; op = next)
    {
        if (op->type == DRAWICON)
        {
            BltBitMap(su->gfx, (op->tile % 20) << 4, (op->tile / 20) << 4, rastport.BitMap, op->x << 4, op->y << 4, op->width, op->height, 0xc0, 0xff, NULL);
            Remove(&op->node);
            AddTail(&su->rep, &op->node); /* Repeat */
        }
    }
}

void changeBitMap(struct Screen *s)
{
    struct screenUser *su = (struct screenUser *)s->UserData;
    UBYTE toggleFrame = su->toggleFrame;

    if (!su->safeToChange)
        while (!GetMsg(su->mp[1]))
            Wait(1L << su->mp[1]->mp_SigBit);

    ChangeVPBitMap(&s->ViewPort, su->bm[toggleFrame], su->dbi);

    su->toggleFrame = toggleFrame ^ 1;

    su->safeToWrite = su->safeToChange = FALSE;
}

void renderScreen(struct BitMap *gfx, struct BitMap *bm[])
{
    WORD i;

    for (i = 0; i < 2; i++)
    {
        BltBitMap(gfx, 0, 16, bm[i], 0, 0, 16, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 16, 16, bm[i], 16, 0, 16, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 32, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 96, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 160, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 224, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 288, 0, 32, 16, 0xc0, 0xff, NULL);

        WORD j, k;

        for (j = 1; j < 15; j++)
        {
            for (k = 0; k < 20; k++)
            {
                if (j <= 1 || j >= 14 || k <= 0 || k >= 19)
                    BltBitMap(gfx, 128, 16, bm[i], k << 4, j << 4, 16, 16, 0xc0, 0xff, NULL);
                else
                    BltBitMap(gfx, 0, 0, bm[i], k << 4, j << 4, 16, 16, 0xc0, 0xff, NULL);
            }
        }
        BltBitMap(gfx, 0, 0, bm[i], 0, 240, 144, 16, 0xc0, 0xff, NULL);
    }
}
