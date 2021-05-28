
#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/modeid.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <intuition/screens.h>

#include <clib/intuition_protos.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

#define RWIDTH  (320) /* Rozmiary rastra */
#define RHEIGHT (256)
#define DEPTH   (5)

#define DWIDTH  (320) /* Rozmiar obrazu */
#define DHEIGHT (256)

#define BWIDTH  (20) /* Rozmiar planszy */
#define BHEIGHT (15)

#define TWIDTH  (4) /* Szerokoôê i wysokoôê kafla (2^n) */
#define THEIGHT (4)

#define BUFLEN  (63) /* Rozmiar bufora na znaki */

UBYTE dir[] = "Data1"; /* Katalog roboczy */

ULONG modeID = LORES_KEY; /* Domyôlny tryb ekranu */

LONG ifferr = 0; /* Bîâd IFF */

enum
{
    BMHD,
    CMAP,
    PROPS
};

struct IFFHandle *openIFF(STRPTR name)
{
    UBYTE buffer[BUFLEN + 1];
    struct IFFHandle *iff;

    strncpy(buffer, dir, BUFLEN);
    AddPart(buffer, name, BUFLEN);

    if (iff = AllocIFF())
    {
        BPTR f = Open(buffer, MODE_OLDFILE);
        if (iff->iff_Stream = f)
        {
            InitIFFasDOS(iff);
            if ((ifferr = OpenIFF(iff, IFFF_READ)) == 0)
            {
                if ((ifferr = ParseIFF(iff, IFFPARSE_STEP)) == 0)
                {
                    return(iff);
                }
                CloseIFF(iff);
            }
            Close(f);
        }
        else
            printf("Couldn't open '%s'!\n", buffer);
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

struct IFFHandle *openILBM(STRPTR name)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name))
    {
        if ((ifferr = PropChunk(iff, ID_ILBM, ID_BMHD)) == 0)
        {
            if ((ifferr = PropChunk(iff, ID_ILBM, ID_CMAP)) == 0)
            {
                if ((ifferr = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
                {
                    if ((ifferr = ParseIFF(iff, IFFPARSE_SCAN)) == 0)
                    {
                        return(iff);
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

BOOL findILBMProps(struct IFFHandle *iff, struct StoredProperty *sp[])
{
    if (sp[BMHD] = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        if (sp[CMAP] = FindProp(iff, ID_ILBM, ID_CMAP))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

ULONG *loadColors(struct StoredProperty *sp)
{
    ULONG *colors;
    ULONG size = sp->sp_Size;
    WORD count = size / 3, c;
    UBYTE *cmap = sp->sp_Data;

    if (colors = AllocVec((size + 2) * sizeof(ULONG), MEMF_PUBLIC))
    {
        colors[0] = count << 16;
        for (c = 0; c < size; c++)
        {
            UBYTE data = *cmap++;
            colors[c + 1] = RGB(data);
        }
        colors[size + 1] = 0L;
        return(colors);
    }
    return(NULL);
}

BYTE *loadBODY(struct IFFHandle *iff, LONG *psize)
{
    struct ContextNode *cn;
    BYTE *buffer;
    LONG size;

    if (cn = CurrentChunk(iff))
    {
        size = cn->cn_Size;
        if (buffer = AllocMem(size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, size) == size)
            {
                *psize = size;
                return(buffer);
            }
            FreeMem(buffer, size);
        }
    }
    return(NULL);
}

BOOL unpackRow(BYTE **bufptr, LONG *sizeptr, BYTE **planeptr, WORD bpr, UBYTE cmp)
{
    BYTE *buf = *bufptr;
    LONG size = *sizeptr;
    BYTE *plane = *planeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        CopyMem(buf, plane, bpr);
        size -= bpr;
        buf += bpr;
        plane += bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE c;
            if (size < 1)
            {
                return(FALSE);
            }
            size--;
            if ((c = *buf++) >= 0)
            {
                WORD count = c + 1;
                if (size < count || bpr < count)
                {
                    return(FALSE);
                }
                size -= count;
                bpr -= count;
                while (count-- > 0)
                {
                    *plane++ = *buf++;
                }
            }
            else if (c != -128)
            {
                WORD count = (-c) + 1;
                BYTE data;
                if (size < 1 || bpr < count)
                {
                    return(FALSE);
                }
                size--;
                bpr -= count;
                data = *buf++;
                while (count-- > 0)
                {
                    *plane++ = data;
                }
            }
        }
    }

    *bufptr = buf;
    *sizeptr = size;
    *planeptr = plane;
    return(TRUE);
}

BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMap *bm, WORD width, WORD height, UBYTE depth, UBYTE cmp)
{
    PLANEPTR planes[8];
    WORD i, j;
    WORD bpr = RowBytes(width);

    for (i = 0; i < depth; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < depth; i++)
        {
            if (!unpackRow(&buffer, &size, &planes[i], bpr, cmp))
            {
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

struct BitMap *loadBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
    struct BitMap *bm;
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    UBYTE depth = bmhd->bmh_Depth;
    BYTE *buffer;
    LONG size;

    if (bm = AllocBitMap(width, height, depth, 0, NULL))
    {
        if (buffer = loadBODY(iff, &size))
        {
            if (unpackBitMap(buffer, size, bm, width, height, depth, bmhd->bmh_Compression))
            {
                FreeMem(buffer, size);
                return(bm);
            }
            FreeMem(buffer, size);
        }
        FreeBitMap(bm);
    }
    return(NULL);
}

BOOL loadILBM(STRPTR name, struct BitMap **gfx, ULONG **pal)
{
    struct IFFHandle *iff;
    struct StoredProperty *sp[PROPS];

    if (iff = openILBM(name))
    {
        if (findILBMProps(iff, sp))
        {
            if (*pal = loadColors(sp[CMAP]))
            {
                if (*gfx = loadBitMap(iff, (struct BitMapHeader *)sp[BMHD]->sp_Data))
                {
                    closeIFF(iff);
                    return(TRUE);
                }
                FreeVec(*pal);
            }
        }
        closeIFF(iff);
    }
    return(FALSE);
}

void drawTitle(struct BitMap *gfx, struct BitMap *bm)
{
    WORD x, y;

    BltBitMap(gfx, 0, 0, bm, 0, 0, 320, 16, 0xc0, 0xff, NULL);

    for (y = 0; y < BHEIGHT; y++)
    {
        for (x = 0; x < BWIDTH; x++)
        {
            if (x == 0 || x == BWIDTH - 1 || y == 0 || y == BHEIGHT - 1)
            {
                BltBitMap(gfx, 16, 32, bm, x << TWIDTH, (y + 1) << THEIGHT, 1 << TWIDTH, 1 << THEIGHT, 0xc0, 0xff, NULL);
            }
            else
            {
                BltBitMap(gfx, 0, 32, bm, x << TWIDTH, (y + 1) << THEIGHT, 1 << TWIDTH, 1 << THEIGHT, 0xc0, 0xff, NULL);
            }
        }
    }
}

int main()
{
    struct BitMap *gfx;
    ULONG *pal;

    if (loadILBM("Graphics.iff", &gfx, &pal))
    {
        struct BitMap *bm;

        if (bm = AllocBitMap(RWIDTH, RHEIGHT, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
        {
            struct Screen *s;

            drawTitle(gfx, bm); /* Narysuj winietë i pustâ planszë */

            WaitBlit();

            if (s = OpenScreenTags(NULL,
                SA_Width,       DWIDTH,
                SA_Height,      DHEIGHT,
                SA_DisplayID,   modeID,
                SA_BitMap,      bm,
                SA_Colors32,    pal,
                SA_ShowTitle,   FALSE,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                TAG_DONE))
            {
                struct Window *w;

                if (w = OpenWindowTags(NULL,
                    WA_CustomScreen,    s,
                    WA_Left,            0,
                    WA_Top,             0,
                    WA_Width,           DWIDTH,
                    WA_Height,          DHEIGHT,
                    WA_Backdrop,        TRUE,
                    WA_Borderless,      TRUE,
                    WA_Activate,        TRUE,
                    WA_RMBTrap,         TRUE,
                    WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
                    WA_ReportMouse,     TRUE,
                    WA_SmartRefresh,    TRUE,
                    WA_BackFill,        LAYERS_NOBACKFILL,
                    TAG_DONE))
                {
                    struct Window *msgwin;

                    BltBitMapRastPort(gfx, 0, 0, w->RPort, 0, 0, 320, s->BarHeight + 1, 0xc0);

                    if (msgwin = OpenWindowTags(NULL,
                        WA_CustomScreen,    s,
                        WA_Left,            31,
                        WA_Top,             16,
                        WA_Width,           100,
                        WA_Height,          128,
                        WA_Borderless,      TRUE,
                        WA_Activate,        TRUE,
                        WA_RMBTrap,         TRUE,
                        WA_IDCMP,           IDCMP_MOUSEBUTTONS,
                        WA_SimpleRefresh,   TRUE,
                        WA_BackFill,        LAYERS_NOBACKFILL,
                        TAG_DONE))
                    {
                        WORD y, step = 8;

                        BltBitMapRastPort(gfx, 32, 16, w->RPort, 32, 0, 80, 16, 0xc0);

                        for (y = 0; y < msgwin->Height; y += step)
                        {
                            BltBitMapRastPort(gfx, 197, 48 + y, msgwin->RPort, 0, y, msgwin->Width, step, 0xc0);
                            WaitTOF();
                        }
                        BltBitMapRastPort(gfx, 0,  48, msgwin->RPort, 4, 20, 80, 16, 0xc0);
                        BltBitMapRastPort(gfx, 80, 80, msgwin->RPort, 4, 40, 80, 16, 0xc0);

                        Delay(400);
                        CloseWindow(msgwin);

                        if (msgwin = OpenWindowTags(NULL,
                            WA_CustomScreen,    s,
                            WA_Left,            111,
                            WA_Top,             16,
                            WA_Width,           100,
                            WA_Height,          128,
                            WA_Borderless,      TRUE,
                            WA_Activate,        TRUE,
                            WA_RMBTrap,         TRUE,
                            WA_IDCMP,           IDCMP_MOUSEBUTTONS,
                            WA_SimpleRefresh,   TRUE,
                            WA_BackFill,        LAYERS_NOBACKFILL,
                            TAG_DONE))
                        {
                            WORD y, step = 8;

                            BltBitMapRastPort(gfx, 32, 0, w->RPort, 32, 0, 80, 16, 0xc0);
                            BltBitMapRastPort(gfx, 112, 16, w->RPort, 112, 0, 80, 16, 0xc0);

                            for (y = 0; y < msgwin->Height; y += step)
                            {
                                BltBitMapRastPort(gfx, 197, 48 + y, msgwin->RPort, 0, y, msgwin->Width, step, 0xc0);
                                WaitTOF();
                            }
                            BltBitMapRastPort(gfx, 0, 176, msgwin->RPort, 4, 20, 60, 60, 0xc0);

                            Delay(400);
                            CloseWindow(msgwin);
                        }
                    }
                    CloseWindow(w);
                }
                CloseScreen(s);
            }
            FreeBitMap(bm);
        }
        FreeBitMap(gfx);
        FreeVec(pal);
    }
    return(0);
}
