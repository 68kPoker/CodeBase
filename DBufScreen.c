
/* Double-buffered screen */

#include <stdio.h>

#include <intuition/screens.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include "DBufScreen.h"
#include "Tiles.h"

/* Add blit operation to queue */
void addBlit(struct List *list, WORD type, WORD tile, WORD x, WORD y, WORD width, WORD height)
{
    struct blitOp *op;

    if (op = AllocMem(sizeof(*op), MEMF_PUBLIC)) {
        op->type = type;
        op->tile = tile;
        op->x    = x;
        op->y    = y;
        op->width = width;
        op->height = height;
        AddTail(list, &op->node);
    }
}

/* Open custom screen */
struct Screen *openScreen(WORD width, WORD height, UBYTE depth, ULONG modeid, struct BitMap *bm[], struct TextAttr *ta, ULONG *colors, struct BitMap *gfx, BOOL editPanel)
{
    struct Screen *s;

    renderScreen(gfx, bm, editPanel);
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
        SA_Interleaved, TRUE,
        TAG_DONE)) {
        struct DBufInfo *dbi;

        if (dbi = AllocDBufInfo(&s->ViewPort)) {
            struct MsgPort *mp[2];

            if (mp[0] = CreateMsgPort()) {
                if (mp[1] = CreateMsgPort()) {
                    struct screenUser *su;

                    if (su = AllocMem(sizeof *su, MEMF_PUBLIC)) {
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
    while (next = (struct blitOp *)op->node.ln_Succ) {
        Remove(&op->node);
        FreeMem(op, sizeof(*op));
        op = next;
    }

    op = (struct blitOp *)su->blits.lh_Head;
    while (next = (struct blitOp *)op->node.ln_Succ) {
        Remove(&op->node);
        FreeMem(op, sizeof(*op));
        op = next;
    }

    FreeMem(su, sizeof *su);
}

void drawScreen(struct Screen *s)
{
    struct screenUser *su = (struct screenUser *)s->UserData;
    struct RastPort rastport, *rp = &rastport;
    struct TextFont *tf = s->RastPort.Font;
    UBYTE toggleFrame = su->toggleFrame;
    struct blitOp *op, *next;

    if (!su->safeToWrite)
        while (!GetMsg(su->mp[0]))
            Wait(1L << su->mp[0]->mp_SigBit);

    su->safeToWrite = TRUE;

    /* Draw next screen frame here */

    InitRastPort(&rastport);
    rastport.BitMap = su->bm[toggleFrame];

    for (op = (struct blitOp *)su->rep.lh_Head; (next = (struct blitOp *)op->node.ln_Succ) != NULL; op = next) {
        if (op->type == DRAWICON) {
            BltBitMap(su->gfx, (op->tile % 20) << 4, (op->tile / 20) << 4, rastport.BitMap, op->x << 4, op->y << 4, op->width, op->height, 0xc0, 0xff, NULL);
            Remove(&op->node);
            FreeMem(op, sizeof(*op));
        }
    }

    for (op = (struct blitOp *)su->blits.lh_Head; (next = (struct blitOp *)op->node.ln_Succ) != NULL; op = next) {
        if (op->type == DRAWICON) {
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

void renderScreen(struct BitMap *gfx, struct BitMap *bm[], BOOL editPanel)
{
    WORD i;

    for (i = 0; i < 2; i++) {
        WORD j, k;

        /* Draw bar */
        BltBitMap(gfx, 0, 16, bm[i], 0, 0, 16, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 16, 16, bm[i], 16, 0, 16, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 32, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 96, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 160, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 224, 0, 64, 16, 0xc0, 0xff, NULL);
        BltBitMap(gfx, 64, 16, bm[i], 288, 0, 32, 16, 0xc0, 0xff, NULL);



        /* Draw board here */
        for (j = 1; j < 15; j++) {
            for (k = 0; k < 20; k++) {
            }
        }

        if (editPanel) {
            /* Draw panel */
            for (j = 0; j < ICONS; j++)
                BltBitMap(gfx, j << 4, 0, bm[i], j << 4, 240, 16, 16, 0xc0, 0xff, NULL);
        }
    }
}
