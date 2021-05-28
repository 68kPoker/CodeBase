
#include "Game.h"

#include <graphics/gfx.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <intuition/intuition.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>

struct gameTemplate *newGameTemplate(void)
{
    struct gameTemplate *p;

    if (p = AllocMem(sizeof(*p), MEMF_PUBLIC|MEMF_CLEAR))
    {
        return(p);
    }
    return(NULL);
}

struct gameBitMap *newGameBitMap(void)
{
    struct gameBitMap *p;

    if (p = AllocMem(sizeof(*p), MEMF_PUBLIC|MEMF_CLEAR))
    {
        if (p->bm = AllocBitMap(RAS_WIDTH, RAS_HEIGHT, RAS_DEPTH, BMF_DISPLAYABLE|RAS_FLAGS, NULL))
        {
            return(p);
        }
        FreeMem(p, sizeof(*p));
    }
    return(NULL);
}

BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMap *bm, struct BitMapHeader *bmhd)
{
    UBYTE cmp = bmhd->bmh_Compression;
    UBYTE msk = bmhd->bmh_Masking;
    WORD height = bmhd->bmh_Height;
    WORD depth = bmhd->bmh_Depth;

    if (msk == mskHasMask)
    {
        depth++;
    }

    if (cmp == cmpNone)
    {
        CopyMem(buffer, bm->Planes[0], size);
        return(TRUE);
    }
    else if (cmp == cmpByteRun1)
    {
        PLANEPTR plane = bm->Planes[0];
        WORD i, j;
        WORD bpr, savebpr;

        if (depth > 1)
            savebpr = bm->Planes[1] - bm->Planes[0];
        else
            savebpr = bm->BytesPerRow;

        for (i = 0; i < height; i++)
        {
            for (j = 0; j < depth; j++)
            {
                bpr = savebpr;
                while (bpr > 0)
                {
                    BYTE con;

                    if (size < 1)
                    {
                        return(FALSE);
                    }

                    size--;
                    if ((con = *buffer++) >= 0)
                    {
                        WORD count = con + 1;
                        if (size < count || bpr < count)
                        {
                            return(FALSE);
                        }
                        size -= count;
                        bpr -= count;
                        while (count-- > 0)
                        {
                            *plane++ = *buffer++;
                        }
                    }
                    else if (con != -128)
                    {
                        WORD count = (-con) + 1;
                        if (size < 1 || bpr < count)
                        {
                            return(FALSE);
                        }
                        size--;
                        bpr -= count;
                        UBYTE data = *buffer++;
                        while (count-- > 0)
                        {
                            *plane++ = data;
                        }
                    }
                }
            }
        }
        return(TRUE);
    }
    return(FALSE);
}

void loadColors(ULONG *pal, UBYTE *cmap, WORD count)
{
    WORD i;

    *pal++ = count << 16;
    for (i = 0; i < (count * 3); i++)
    {
        UBYTE val = *cmap++;
        *pal++ = RGB(val);
    }
    *pal = 0L;
}

struct gameGraphics *newGameGraphics(STRPTR name)
{
    struct gameGraphics *p;

    if (p = AllocMem(sizeof(*p), MEMF_PUBLIC|MEMF_CLEAR))
    {
        BPTR f;
        if (f = Open(name, MODE_OLDFILE))
        {
            struct IFFHandle *iff;
            if (iff = AllocIFF())
            {
                LONG err;
                iff->iff_Stream = (ULONG)f;
                InitIFFasDOS(iff);
                if ((err = OpenIFF(iff, IFFF_READ)) == 0)
                {
                    if ((err = ParseIFF(iff, IFFPARSE_STEP)) == 0)
                    {
                        struct ContextNode *cn;
                        if (cn = CurrentChunk(iff))
                        {
                            if (cn->cn_Type == ID_ILBM && cn->cn_ID == ID_FORM)
                            {
                                if ((err = PropChunk(iff, ID_ILBM, ID_BMHD)) == 0)
                                {
                                    if ((err = PropChunk(iff, ID_ILBM, ID_CMAP)) == 0)
                                    {
                                        if ((err = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
                                        {
                                            if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0)
                                            {
                                                struct StoredProperty *sp;
                                                if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
                                                {
                                                    struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
                                                    WORD depth = bmhd->bmh_Depth;
                                                    if (bmhd->bmh_Masking == mskHasMask)
                                                    {
                                                        depth++;
                                                    }
                                                    if (p->bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, depth, BMF_INTERLEAVED, NULL))
                                                    {
                                                        struct StoredProperty *sp;
                                                        if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
                                                        {
                                                            LONG size = (sp->sp_Size + 2) * sizeof(ULONG);
                                                            p->colsize = size;
                                                            if (p->colors = AllocMem(size, MEMF_PUBLIC))
                                                            {
                                                                loadColors(p->colors, sp->sp_Data, sp->sp_Size / 3);
                                                                if (cn = CurrentChunk(iff))
                                                                {
                                                                    if (cn->cn_Type == ID_ILBM && cn->cn_ID == ID_BODY)
                                                                    {
                                                                        BYTE *buffer;
                                                                        if (buffer = AllocMem(cn->cn_Size, MEMF_PUBLIC))
                                                                        {
                                                                            if (ReadChunkBytes(iff, buffer, cn->cn_Size) == cn->cn_Size)
                                                                            {
                                                                                if (unpackBitMap(buffer, cn->cn_Size, p->bm, bmhd))
                                                                                {
                                                                                    FreeMem(buffer, cn->cn_Size);
                                                                                    CloseIFF(iff);
                                                                                    Close(f);
                                                                                    FreeIFF(iff);
                                                                                    return(p);
                                                                                }
                                                                            }
                                                                            FreeMem(buffer, cn->cn_Size);
                                                                        }
                                                                    }
                                                                }
                                                                FreeMem(p->colors, size);
                                                            }
                                                        }
                                                        FreeBitMap(p->bm);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    CloseIFF(iff);
                }
                Close(f);
            }
            FreeIFF(iff);
        }
        FreeMem(p, sizeof(*p));
    }
    return(NULL);
}

struct gameScreen *newGameScreen(struct gameBitMap *bm, struct gameGraphics *gfx)
{
    struct gameScreen *p;

    if (p = AllocMem(sizeof(*p), MEMF_PUBLIC|MEMF_CLEAR))
    {
        if (p->s = OpenScreenTags(NULL,
            SA_BitMap,  bm->bm,
            SA_Colors32,    gfx->colors,
            SA_DisplayID,   MODEID,
            SA_Quiet,   TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_Draggable,   FALSE,
            TAG_DONE))
        {
            return(p);
        }
        FreeMem(p, sizeof(*p));
    }
    return(NULL);
}

void disposeGameTemplate(struct gameTemplate *p)
{
    FreeMem(p, sizeof(*p));
}

void disposeGameBitMap(struct gameBitMap *p)
{
    FreeBitMap(p->bm);
    FreeMem(p, sizeof(*p));
}

void disposeGameGraphics(struct gameGraphics *p)
{
    FreeBitMap(p->bm);
    FreeMem(p->colors, p->colsize);
    FreeMem(p, sizeof(*p));
}

void disposeGameScreen(struct gameScreen *p)
{
    CloseScreen(p->s);
    FreeMem(p, sizeof(*p));
}
