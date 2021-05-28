
#include <stdio.h>

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/intuition_protos.h>

/* Screen bitmap size */
#define SBM_WIDTH  320
#define SBM_HEIGHT 256
#define SBM_DEPTH  5

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define RowBytes(w) ((((w)+15)>>4)<<1)

void readColors(UBYTE *cmap, ULONG colorCount, ULONG *colors)
{
    ULONG i;

    *colors++ = colorCount << 16;

    for (i = 0; i < colorCount; i++)
    {
        UBYTE red = *cmap++;
        UBYTE green = *cmap++;
        UBYTE blue = *cmap++;

        *colors++ = RGB(red);
        *colors++ = RGB(green);
        *colors++ = RGB(blue);
    }

    *colors = 0L;
}

BOOL unpackRow(BYTE **bufferPtr, LONG *sizePtr, BYTE **planePtr, WORD bpr, UBYTE cmp)
{
    BYTE *buffer = *bufferPtr, *plane = *planePtr;
    LONG size = *sizePtr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
            return(FALSE);

        CopyMem(buffer, plane, bpr);
        buffer += bpr;
        plane += bpr;
        size -= bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (size < 1)
                return(FALSE);
            size--;
            if ((con = *buffer++) >= 0)
            {
                WORD count = con + 1;
                if (size < count || bpr < count)
                    return(FALSE);
                size -= count;
                bpr -= count;
                while (count-- > 0)
                    *plane++ = *buffer++;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (size < 1 || bpr < count)
                    return(FALSE);
                size--;
                bpr -= count;
                data = *buffer++;
                while (count-- > 0)
                    *plane++ = data;
            }
        }
    }

    *bufferPtr = buffer;
    *planePtr = plane;
    *sizePtr = size;
    return(TRUE);
}

struct BitMap *unpackBitMap(struct BitMapHeader *bmhd, BYTE *bodyBuffer, LONG bodySize)
{
    WORD w = bmhd->bmh_Width, h = bmhd->bmh_Height, d = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression, masking = bmhd->bmh_Masking;
    struct BitMap *bm;
    WORD bpr = RowBytes(w);

    if (masking == mskHasMask)
        /* Alloc additional bitplane for mask */
        d++;

    if (bm = AllocBitMap(w, h, d, 0, NULL))
    {
        WORD p, r;
        PLANEPTR planes[9];
        BOOL success = FALSE;

        for (p = 0; p < d; p++)
            planes[p] = bm->Planes[p];

        for (r = 0; r < h; r++)
        {
            for (p = 0; p < d; p++)
                if (!(success = unpackRow(&bodyBuffer, &bodySize, &planes[p], bpr, cmp)))
                    break;
            if (!success)
                break;
        }

        if (success)
            return(bm);
        else
            printf("Decompression error!\n");
        FreeBitMap(bm);
    }
    else
        printf("Couldn't AllocBitMap!\n");
    return(NULL);
}

/* Load bitmap and colors from graphics file */
struct BitMap *loadBitMap(STRPTR name, ULONG **colors)
{
    /* Open file */
    BPTR f = Open(name, MODE_OLDFILE);

    if (f)
    {
        struct IFFHandle *iff = AllocIFF();

        if (iff)
        {
            LONG err;

            iff->iff_Stream = f;
            InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, IFFF_READ)) == 0)
            {
                if ((err = ParseIFF(iff, IFFPARSE_STEP)) == 0)
                {
                    struct ContextNode *cn = CurrentChunk(iff);

                    if (cn)
                    {
                        if (cn->cn_Type == ID_ILBM && cn->cn_ID == ID_FORM)
                        {
                            /* Scan IFF ILBM */
                            if ((err = PropChunk(iff, ID_ILBM, ID_BMHD)) == 0)
                            {
                                if ((err = PropChunk(iff, ID_ILBM, ID_CMAP)) == 0)
                                {
                                    if ((err = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
                                    {
                                        if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0)
                                        {
                                            struct StoredProperty *sp = FindProp(iff, ID_ILBM, ID_BMHD);

                                            if (sp)
                                            {
                                                struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;

                                                if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
                                                {
                                                    UBYTE *cmap = sp->sp_Data;
                                                    ULONG colorCount = sp->sp_Size / 3;

                                                    if (*colors = AllocVec(((colorCount * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC))
                                                    {
                                                        /* Read colors */
                                                        readColors(cmap, colorCount, *colors);

                                                        if (cn = CurrentChunk(iff))
                                                        {
                                                            LONG bodySize = cn->cn_Size;
                                                            BYTE *bodyBuffer = AllocMem(bodySize, MEMF_PUBLIC);

                                                            if (bodyBuffer)
                                                            {
                                                                if (ReadChunkBytes(iff, bodyBuffer, bodySize) == bodySize)
                                                                {
                                                                    struct BitMap *bm;

                                                                    /* Decompress buffer */
                                                                    if (bm = unpackBitMap(bmhd, bodyBuffer, bodySize))
                                                                    {
                                                                        FreeMem(bodyBuffer, bodySize);
                                                                        CloseIFF(iff);
                                                                        FreeIFF(iff);
                                                                        Close(f);
                                                                        return(bm);
                                                                    }
                                                                }
                                                                else
                                                                    printf("Couldn't ReadChunkBytes!\n");
                                                                FreeMem(bodyBuffer, bodySize);
                                                            }
                                                            else
                                                                printf("Couldn't AllocMem!\n");
                                                        }
                                                        else
                                                            printf("Couldn't CurrentChunk!\n");
                                                        FreeVec(*colors);
                                                    }
                                                    else
                                                        printf("Couldn't AllocVec!\n");
                                                }
                                                else
                                                    printf("Couldn't FindProp CMAP!\n");
                                            }
                                            else
                                                printf("Couldn't FindProp BMHD!\n");
                                        }
                                        else
                                            printf("Couldn't ParseIFF Scan!\n");
                                    }
                                    else
                                        printf("Couldn't StopChunk!\n");
                                }
                                else
                                    printf("Couldn't PropChunk!\n");
                            }
                            else
                                printf("Couldn't PropChunk!\n");
                        }
                        else
                            printf("Not an ILBM file!\n");
                    }
                    else
                        printf("Couldn't CurrentChunk!\n");
                }
                else
                    printf("Couldn't ParseIFF Step!\n");
                CloseIFF(iff);
            }
            else
                printf("Couldn't OpenIFF!\n");
            FreeIFF(iff);
        }
        else
            printf("Couldn't AllocIFF!\n");
        Close(f);
    }
    else
        printf("Couldn't Open %s!\n", name);
    return(NULL);
}

void freeBitMap(struct BitMap *bm, ULONG *colors)
{
    FreeBitMap(bm);
    FreeVec(colors);
}

/* Draw contents of the bitmap */
void drawContents(struct BitMap *bm, struct BitMap *gfxbm)
{
    BltBitMap(gfxbm, 0, 0, bm, 0, 0, 320, 256, 0xc0, 0xff >> (8 - SBM_DEPTH), NULL);
}

int main(void)
{
    struct BitMap *sbm;

    if (sbm = AllocBitMap(SBM_WIDTH, SBM_HEIGHT, SBM_DEPTH, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        struct BitMap *gfxbm;
        ULONG *colors;

        if (gfxbm = loadBitMap("Data/Graphics.iff", &colors))
        {
            struct Screen *pubs;
            struct Screen *s;

            /* Draw initial contents of the bitmap */
            drawContents(sbm, gfxbm);

            if (pubs = LockPubScreen("Workbench"))
            {
                if (s = OpenScreenTags(NULL,
                    SA_Top, 128,
                    SA_BitMap,  sbm,
                    SA_Colors32, colors,
                    SA_Width, 320,
                    SA_Height, 256,
                    SA_Depth, SBM_DEPTH,
                    SA_DisplayID, LORES_KEY,
                    SA_Quiet, TRUE,
                    SA_ShowTitle, FALSE,
                    SA_BackFill, LAYERS_NOBACKFILL,
                    SA_Parent, pubs,
                    TAG_DONE))
                {
                    Delay(100);
                    ScreenPosition(s, SPOS_MAKEVISIBLE, 0, 0, 319, 255);
                    Delay(300);
                    CloseScreen(s);
                }
                else
                    printf("Couldn't OpenScreen!\n");
                UnlockPubScreen(NULL, pubs);
            }
            else
                printf("Couldn't LockPubScreen!\n");
            freeBitMap(gfxbm, colors);
        }
        FreeBitMap(sbm);
    }
    else
        printf("Couldn't AllocBitMap!\n");
    return(RETURN_OK);
}
