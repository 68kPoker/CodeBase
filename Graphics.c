
#include "Graphics.h"
#include "Game.h"
#include "IFF.h"

#include <intuition/screens.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>

BOOL unpackRow(struct IFFHandle *iff, struct buffer *buffer, BYTE **destptr, WORD bpr, UBYTE cmp)
{
    BYTE *dest = *destptr;

    if (cmp == cmpNone)
    {
        if (readChunkBytes(iff, dest, bpr, buffer) == -1)
            return(FALSE);
        dest += bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (readChunkBytes(iff, &con, 1, buffer) == -1)
                return(FALSE);
            if (con >= 0)
            {
                WORD count = con + 1;
                if (bpr < count)
                    return(FALSE);
                if (readChunkBytes(iff, dest, count, buffer) == -1)
                    return(FALSE);
                bpr -= count;
                dest += count;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (bpr < count)
                    return(FALSE);
                if (readChunkBytes(iff, &data, 1, buffer) == -1)
                    return(FALSE);
                bpr -= count;
                while (count-- > 0)
                    *dest++ = data;
            }
        }
    }
    else
        return(FALSE);

    *destptr = dest;
    return(TRUE);
}

BOOL unpackBitMap(struct gfxInfo *gi, struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
    struct buffer buffer;

    if (buffer.buf = AllocMem(BUFFER_SIZE, MEMF_PUBLIC))
    {
        buffer.size = BUFFER_SIZE;
        initBuffer(&buffer);

        PLANEPTR planes[8];
        WORD i, j;
        WORD depth = bmhd->bmh_Depth, height = bmhd->bmh_Height;
        WORD bpr = RowBytes(bmhd->bmh_Width);
        BOOL success = FALSE;

        for (i = 0; i < depth; i++)
            planes[i] = gi->bm->Planes[i];

        for (j = 0; j < height; j++)
        {
            for (i = 0; i < depth; i++)
            {
                if (!(success = unpackRow(iff, &buffer, &planes[i], bpr, bmhd->bmh_Compression)))
                    break;
            }
            if (!success)
                break;
        }
        FreeMem(buffer.buf, BUFFER_SIZE);
        if (success)
            return(TRUE);
    }
    return(FALSE);
}

ULONG *loadColors(struct IFFHandle *iff)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        LONG size = sp->sp_Size;
        ULONG *colors;
        UBYTE *cmap = sp->sp_Data;

        if (colors = AllocVec((size + 2) * sizeof(ULONG), MEMF_PUBLIC))
        {
            WORD i;
            colors[0] = (size / 3) << 16;
            for (i = 0; i < size; i++)
            {
                UBYTE data = *cmap++;
                colors[i + 1] = RGB(data);
            }
            colors[size + 1] = 0L;
            return(colors);
        }
    }
    return(NULL);
}

/* Load graphics data */

BOOL loadGraphics(struct gfxInfo *gi, STRPTR name)
{
    struct IFFHandle *iff;
    struct scanInfo si;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        0
    };
    ULONG stops[] =
    {
        ID_ILBM, ID_BODY,
        0
    };

    si.props = props;
    si.stops = stops;

    if (iff = scanIFF(&si, name))
    {
        struct StoredProperty *sp;

        if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
        {
            struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
            printf("%d %d %d\n", bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth);

            if (gi->colors32 = loadColors(iff))
            {
                if (gi->bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, 0, NULL))
                {
                    if (unpackBitMap(gi, iff, bmhd))
                    {
                        closeIFF(iff, &si);
                        return(TRUE);
                    }
                    FreeBitMap(gi->bm);
                }
                FreeVec(gi->colors32);
            }
        }
        closeIFF(iff, &si);
    }
    return(FALSE);
}

void unloadGraphics(struct gfxInfo *gi)
{
    FreeBitMap(gi->bm);
    FreeVec(gi->colors32);
}

/* Create mask */

BOOL createMask(struct gfxInfo *gi)
{

}

/* Alloc and blit screen bitmap */

struct BitMap *createBitMap(struct gfxInfo *gi)
{
    struct BitMap *bm;

    if (bm = AllocBitMap(BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_DEPTH, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        BltBitMap(gi->bm, 0, 0, bm, 0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, 0xc0, 0xff, NULL);
        return(bm);
    }
    return(NULL);
}

/* Open screen using bitmap and colormap */

struct Screen *openScreen(struct BitMap *bm, struct gfxInfo *gi)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_BitMap,      bm,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       SCREEN_WIDTH,
        SA_Height,      SCREEN_HEIGHT,
        SA_Depth,       SCREEN_DEPTH,
        SA_DisplayID,   SCREEN_MODEID,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       "Magazyn",
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Colors32,    gi->colors32,
        TAG_DONE))
    {
        struct screenInfo *si;

        if (s->UserData = (APTR)si = AllocMem(sizeof(*si), MEMF_PUBLIC))
        {
            if (si->dbi = AllocDBufInfo(&s->ViewPort))
            {
                if (si->mp[0] = CreateMsgPort())
                {
                    if (si->mp[1] = CreateMsgPort())
                    {
                        si->dbi->dbi_SafeMessage.mn_ReplyPort = si->mp[0];
                        si->dbi->dbi_DispMessage.mn_ReplyPort = si->mp[1];
                        si->safe[0] = si->safe[1] = TRUE;
                        si->gi = gi;
                        si->up = si->down = si->left = si->right = FALSE;
                        si->floor = FLOOR_WALL;
                        si->object = -1;
                        if (si->board = AllocMem(sizeof(*si->board), MEMF_PUBLIC))
                        {
                            si->board->screen = s;
                            initBoard(si->board, TMP_WIDTH, TMP_HEIGHT);
                            return(s);
                        }
                        DeleteMsgPort(si->mp[1]);
                    }
                    DeleteMsgPort(si->mp[0]);
                }
                FreeDBufInfo(si->dbi);
            }
            FreeMem(si, sizeof(*si));
        }
        CloseScreen(s);
    }
    return(NULL);
}

void closeScreen(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    freeBoard(si->board);
    FreeMem(si->board, sizeof(*si->board));

    if (!si->safe[1])
        while (!GetMsg(si->mp[1]))
            WaitPort(si->mp[1]);

    if (!si->safe[0])
        while (!GetMsg(si->mp[0]))
            WaitPort(si->mp[0]);

    DeleteMsgPort(si->mp[1]);
    DeleteMsgPort(si->mp[0]);
    FreeDBufInfo(si->dbi);
    FreeMem(si, sizeof(*si));
    CloseScreen(s);
}
