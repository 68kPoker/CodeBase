
/* $Log$ */

#include <stdlib.h>

#include <exec/interrupts.h>
#include <exec/memory.h>

#include <graphics/gfxmacros.h>

#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <intuition/screens.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>

#include "Screen.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

#define MODEID (LORES_KEY)

struct BitMap
    *bitmaps[2],
    *graphics,
    *allocBitMap(WORD w, WORD h, UBYTE nPlanes),
    *loadBitMap(struct IFFHandle *iff); /* Read from IFF stream */

ULONG
    *colors,
    *loadColors(struct IFFHandle *iff);

struct Screen
    *screen,
    *openScreen(struct BitMap *bm, ULONG *colors);

struct DBufInfo
    *dbi; /* Double-buffering */

struct MsgPort
    *safePort; /* SafeToDraw port */

BOOL safeToDraw = TRUE;

extern struct board gameBoard;
extern WORD heroX, heroY, heroDir;

struct DBufInfo *prepDBufInfo(struct ViewPort *vp);

void freeDBufInfo(struct DBufInfo *dbi);

struct Interrupt
    *is, /* Copper interrupt server */
    *addCopper(struct ViewPort *vp);

void remCopper(struct Interrupt *is);

BOOL getCopper(struct ViewPort *vp);

__far extern struct Custom custom; /* Hardware registers */

extern void myCopper(void); /* Assembler interrupt server */

void freeScreen(void);

void initBoard(struct BitMap *gfx, struct BitMap *bm)
{
    WORD x, y, tile, tiley;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {

            if (y > 0 && gameBoard.tiles[y][x].flags > 0)
            {
                if (x == 0 || x == BOARD_WIDTH - 1 || y == 1 || y == BOARD_HEIGHT - 1)
                    tile = WALL;
                else
                    tile = gameBoard.tiles[y][x].type;

                tiley = gameBoard.tiles[y][x].counter;

                if (tile == HERO)
                {
                    tiley = heroDir;
                }

                BltBitMap(gfx, 80 + (tile << 4), tiley << 4, bm, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
                gameBoard.tiles[y][x].flags--;
            }
        }
    }
}

void refreshBoard(BOOL clear)
{
    WORD x, y;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            if (clear)
            {
                if (x == 0 || y == 1 || x == BOARD_WIDTH - 1 || y == BOARD_HEIGHT - 1)
                {
                    gameBoard.tiles[y][x].type = WALL;
                }
                else
                {
                    gameBoard.tiles[y][x].type = FLOOR;
                }
            }
            gameBoard.tiles[y][x].flags = 2;
        }
    }
    gameBoard.tiles[heroY][heroX].type = HERO;
}

/*=================================================================*/

BOOL initScreen(WORD w, WORD h, UBYTE nPlanes, struct IFFHandle *iff)
{
    if (bitmaps[0] = allocBitMap(w, h, nPlanes))
    {
        if (bitmaps[1] = allocBitMap(w, h, nPlanes))
        {
            if (graphics = loadBitMap(iff))
            {
                if (colors = loadColors(iff))
                {
                    /* Prepare BitMap here */
                    BltBitMap(graphics, 0, 0, bitmaps[0], 0, 0, 320, 16, 0xc0, 0xff, NULL);
                    refreshBoard(TRUE);
                    initBoard(graphics, bitmaps[0]);
                    WaitBlit();
                    if (screen = openScreen(bitmaps[0], colors))
                    {
                        struct ViewPort *vp = &screen->ViewPort;

                        if (dbi = prepDBufInfo(vp))
                        {
                            if (is = addCopper(vp))
                            {
                                if (getCopper(vp))
                                {
                                    atexit(freeScreen);
                                    return(TRUE);
                                }
                                remCopper(is);
                            }
                            freeDBufInfo(dbi);
                        }
                        CloseScreen(screen);
                    }
                    FreeVec(colors);
                }
                FreeBitMap(graphics);
            }
            FreeBitMap(bitmaps[1]);
        }
        FreeBitMap(bitmaps[0]);
    }
    return(FALSE);
}

void freeScreen(void)
{
    remCopper(is);
    freeDBufInfo(dbi);
    CloseScreen(screen);
    FreeVec(colors);
    FreeBitMap(graphics);
    FreeBitMap(bitmaps[1]);
    FreeBitMap(bitmaps[0]);
}

struct BitMap *allocBitMap(WORD w, WORD h, UBYTE nPlanes)
{
    return(AllocBitMap(w, h, nPlanes, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL));
}

UBYTE *findPropData(struct IFFHandle *iff, LONG type, LONG id)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, type, id))
    {
        return(sp->sp_Data);
    }
    return(NULL);
}

BOOL unpackRow(BYTE *plane, WORD bpr, BYTE **bufptr, LONG *sizeptr, UBYTE cmp)
{
    BYTE *buffer = *bufptr;
    LONG size = *sizeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        size -= bpr;
        CopyMem(buffer, plane, bpr);
        buffer += bpr;
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
            if ((c = *buffer++) >= 0)
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
                    *plane++ = *buffer++;
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
                data = *buffer++;
                while (count-- > 0)
                {
                    *plane++ = data;
                }
            }
        }
    }
    else
    {
        return(FALSE);
    }

    *bufptr = buffer;
    *sizeptr = size;
    return(TRUE);
}

BOOL unpackBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd, struct BitMap *bm)
{
    struct ContextNode *cn;
    BYTE *buffer;
    LONG size;

    if (cn = CurrentChunk(iff))
    {
        size = cn->cn_Size;
        if (buffer = AllocVec(size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, size) == size)
            {
                WORD w = bmhd->bmh_Width, h = bmhd->bmh_Height;
                UBYTE nPlanes = bmhd->bmh_Depth;
                UBYTE cmp = bmhd->bmh_Compression;
                WORD bpr = RowBytes(w);
                WORD i, j;
                BYTE *curbuffer = buffer;
                LONG cursize = size;

                PLANEPTR planes[8];

                for (i = 0; i < nPlanes; i++)
                {
                    planes[i] = bm->Planes[i];
                }

                for (j = 0; j < h; j++)
                {
                    for (i = 0; i < nPlanes; i++)
                    {
                        if (!unpackRow(planes[i], bpr, &curbuffer, &cursize, cmp))
                        {
                            FreeVec(buffer);
                            return(FALSE);
                        }
                        planes[i] += bm->BytesPerRow;
                    }
                }
                FreeVec(buffer);
                return(TRUE);
            }
            FreeVec(buffer);
        }
    }
    return(FALSE);
}

/* Read from IFF stream */
struct BitMap *loadBitMap(struct IFFHandle *iff)
{
    struct BitMapHeader *bmhd;
    struct BitMap *bm;

    if (bmhd = (struct BitMapHeader *)findPropData(iff, ID_ILBM, ID_BMHD))
    {
        WORD w = bmhd->bmh_Width, h = bmhd->bmh_Height;
        UBYTE nPlanes = bmhd->bmh_Depth;
        UBYTE msk = bmhd->bmh_Masking;

        if (msk == mskNone || msk == mskHasTransparentColor)
        {
            if (bm = AllocBitMap(w, h, nPlanes, BMF_INTERLEAVED, NULL))
            {
                /* Unpack BitMap here */
                if (unpackBitMap(iff, bmhd, bm))
                {
                    return(bm);
                }
                FreeBitMap(bm);
            }
        }
    }
    return(NULL);
}

ULONG *loadColors(struct IFFHandle *iff)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        LONG colors = sp->sp_Size / 3;
        ULONG *tab;

        if (tab = AllocVec(((3 * colors) + 2) * sizeof(ULONG), MEMF_PUBLIC|MEMF_CLEAR))
        {
            WORD c;
            tab[0] = colors << 16;
            for (c = 0; c < colors; c++)
            {
                UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
                tab[(c * 3) + 1] = RGB(red);
                tab[(c * 3) + 2] = RGB(green);
                tab[(c * 3) + 3] = RGB(blue);
            }
            tab[(colors * 3) + 1] = 0L;
            return(tab);
        }
    }
    return(NULL);
}

struct Screen *openScreen(struct BitMap *bm, ULONG *colors)
{
    struct Rectangle
        dclip = { 0, 0, 319, 255 };

    return(OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_DisplayID,   MODEID,
        SA_BitMap,      bm,
        SA_Colors32,    colors,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       "Magazyn",
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE));
}

struct DBufInfo *prepDBufInfo(struct ViewPort *vp)
{
    struct DBufInfo *dbi;

    if (dbi = AllocDBufInfo(vp))
    {
        struct MsgPort *safePort;

        if (dbi->dbi_SafeMessage.mn_ReplyPort = safePort = CreateMsgPort())
        {
            dbi->dbi_UserData1 = (APTR)TRUE; /* Safe to draw */
            dbi->dbi_UserData2 = (APTR)1; /* Frame */
            return(dbi);
        }
        FreeDBufInfo(dbi);
    }
    return(NULL);
}

void freeDBufInfo(struct DBufInfo *dbi)
{
    struct MsgPort *safePort = dbi->dbi_SafeMessage.mn_ReplyPort;
    BOOL safeToDraw = (BOOL)dbi->dbi_UserData1;

    if (!safeToDraw)
    {
        while (!GetMsg(safePort))
        {
            WaitPort(safePort);
        }
    }
    DeleteMsgPort(safePort);
    FreeDBufInfo(dbi);
}

struct Interrupt *addCopper(struct ViewPort *vp)
{
    struct Interrupt *is;

    if (is = AllocMem(sizeof(*is), MEMF_PUBLIC|MEMF_CLEAR))
    {
        struct copper *cop;

        is->is_Code = myCopper;
        is->is_Node.ln_Pri = 0;

        if (cop = AllocMem(sizeof(*cop), MEMF_PUBLIC|MEMF_CLEAR))
        {
            is->is_Data = (APTR)cop;

            if ((cop->signal = AllocSignal(-1)) != -1)
            {
                cop->task = FindTask(NULL);
                cop->viewPort = vp;

                AddIntServer(INTB_COPER, is);
                return(is);
            }
            FreeMem(cop, sizeof(*cop));
        }
        FreeMem(is, sizeof(*is));
    }
    return(NULL);
}

void remCopper(struct Interrupt *is)
{
    struct copper *cop = (struct copper *)is->is_Data;

    RemIntServer(INTB_COPER, is);
    FreeSignal(cop->signal);
    FreeMem(cop, sizeof(*cop));
    FreeMem(is, sizeof(*is));
}

BOOL getCopper(struct ViewPort *vp)
{
    struct UCopList *ucl;

    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
    {
        CINIT(ucl, 3);
        CWAIT(ucl, 0, 0);
        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
        CEND(ucl);

        Forbid();
        vp->UCopIns = ucl;
        Permit();

        RethinkDisplay();

        return(TRUE);
    }
    return(FALSE);

}

/** EOF **/
