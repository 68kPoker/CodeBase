
#include <dos/rdargs.h>
#include <workbench/startup.h>
#include <exec/libraries.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <intuition/intuition.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "Main.h"

#define KICK_VERSION (36)
#define ARGUMENTS    (2)

#define BASIC_DATA   (0)

int main(void);
int wbmain(struct WBStartup *wbs);
BOOL openLibs(void);
BOOL openDevs(void);
void closeLibs(void);
void closeDevs(void);
BOOL loadGraphics(WORD kind);
void unloadGraphics(WORD kind, struct BitMap *gfx);
int startup(void);
BOOL init(void);
int run(void);
void cleanup(void);
BOOL unpackRow(BYTE **dataptr, LONG *sizeptr, BYTE *dest, WORD bpr, UBYTE cmp);
struct BitMap *unpackPicture(struct BitMapHeader *bmhd, struct IFFHandle *iff);
void readColors(UBYTE *data, ULONG *colors, WORD count);

__far extern struct Custom custom;
extern void myCopper(void);

extern struct Library *SysBase, *DOSBase, *GfxBase;
struct Library *IntuitionBase, *IFFParseBase, *DiskfontBase;

UBYTE version[] = "\n$VER: Magazyn v1.3\n";
UBYTE template[] = "DATA/K,EDITOR/S";

BPTR out; /* Output */

ULONG *colors; /* RGB */

struct BitMap *bitmaps[2], *gfx; /* Graphics */
struct Screen *screen;
struct Window *window;

struct Interrupt copper_is;
struct copperData cop_data;

extern int runGame(void);

int main(void)
{
    static UBYTE error[] = "This program requires OS2.0 (V36)!\n";
    int result = RETURN_OK;

    /* First step: determine system version */
    LONG v = DOSBase->lib_Version;

    if (!(out = Output()))
    {
        return(RETURN_ERROR);
    }

    Write(out, version, sizeof version);

    if (v < KICK_VERSION)
    {
        Write(out, error, sizeof error);
        return(RETURN_FAIL);
    }

    /* Second step, get args */
    struct RDArgs *rda;
    ULONG array[ARGUMENTS];

    if (rda = ReadArgs(template, array, NULL))
    {
        /* Run game! */
        result = startup();

        FreeArgs(rda);
    }
    else
    {
        FPrintf(out, "Arguments unsuitable for the template!\n");
        result = RETURN_ERROR;
    }

    return(result);
}

/* startup() - Arguments processed */
int startup(void)
{
    int result = RETURN_WARN;

    /* First, open libraries and devices (gameport, audio) */
    if (openLibs())
    {
        if (openDevs())
        {
            /* Init game (graphics, screen etc.) and run it */
            if (init())
            {
                result = RETURN_OK;
                run();
                cleanup();
            }
            closeDevs();
        }
        closeLibs();
    }
    return(result);
}

struct BitMap *allocBitMap(WORD width, WORD height, UBYTE depth)
{
    struct BitMap *bm;

    if (GfxBase->lib_Version >= 39L)
    {
        FPrintf(out, "V39 AllocBitMap.\n");
        if (bm = AllocBitMap(width, height, depth, BMF_DISPLAYABLE | BMF_CLEAR | BMF_INTERLEAVED, NULL))
        {
            return(bm);
        }
    }
    else if (bm = AllocMem(sizeof(*bm), MEMF_PUBLIC|MEMF_CLEAR))
    {
        FPrintf(out, "V36 AllocRaster.\n");
        if (bm->Planes[0] = AllocRaster(width, height * depth))
        {
            WORD i, bpr = RowBytes(width);

            bm->BytesPerRow = bpr * depth;
            bm->Rows = height;
            bm->Depth = depth;

            for (i = 1; i < depth; i++)
                bm->Planes[i] = bm->Planes[i - 1] + bpr;

            BltClear(bm->Planes[0], RASSIZE(width, height * depth), 0x01);

            return(bm);
        }
        FreeMem(bm, sizeof(*bm));
    }
    return(NULL);
}

/* init() - Libs/devs opened */
BOOL init(void)
{
    /* First, load graphics data to obtain resolution and screen depth */
    if (loadGraphics(BASIC_DATA))
    {
        WORD width = 320, height = 256, depth = gfx->Depth;

        if (bitmaps[0] = allocBitMap(width, height, depth))
        {
            if (bitmaps[1] = allocBitMap(width, height, depth))
            {
                /* Open screen */
                ULONG modeID = PAL_MONITOR_ID | LORES_KEY;
                struct Rectangle dclip = { 0, 0, 319, 255 };

                if (screen = OpenScreenTags(NULL,
                    SA_BitMap,      bitmaps[0],
                    SA_DisplayID,   modeID,
                    SA_DClip,       &dclip,
                    SA_Quiet,       TRUE,
                    SA_Exclusive,   TRUE,
                    SA_ShowTitle,   FALSE,
                    SA_BackFill,    LAYERS_NOBACKFILL,
                    SA_Colors32,    colors,
                    SA_Interleaved, TRUE,
                    SA_Title,       version,
                    TAG_DONE))
                {
                    struct UCopList *ucl;

                    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC | MEMF_CLEAR))
                    {
                        WORD coppos[] = { 0, 128 };
                        WORD coplen = 6;
                        BYTE pri = 0;

                        CINIT(ucl, coplen);
                        CWAIT(ucl, coppos[0], 0);
                        CMOVE(ucl, custom.intreq, INTF_SETCLR | INTF_COPER);
                        CWAIT(ucl, coppos[1], 0);
                        CMOVE(ucl, custom.intreq, INTF_SETCLR | INTF_COPER);
                        CEND(ucl);

                        Forbid();
                        screen->ViewPort.UCopIns = ucl;
                        Permit();

                        RethinkDisplay();

                        copper_is.is_Code = myCopper;
                        copper_is.is_Data = (APTR)&cop_data;
                        copper_is.is_Node.ln_Pri = pri;

                        cop_data.vp = &screen->ViewPort;
                        if ((cop_data.signal = AllocSignal(-1)) != -1)
                        {
                            cop_data.task = FindTask(NULL);

                            AddIntServer(INTB_COPER, &copper_is);
                            return(TRUE);
                        }
                    }
                    CloseScreen(screen);
                }
                else
                {
                    FPrintf(out, "Couldn't open screen!\n");
                }
                FreeBitMap(bitmaps[1]);
            }
            FreeBitMap(bitmaps[0]);
        }
        unloadGraphics(BASIC_DATA, gfx);
    }
    return(FALSE);
}

struct Window *openWindow(WORD left, WORD top, WORD width, WORD height)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    screen,
        WA_Left,            left,
        WA_Top,             top,
        WA_Width,           width,
        WA_Height,          height,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY|IDCMP_REFRESHWINDOW|IDCMP_MOUSEMOVE,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

int run(void)
{
    runGame();
    return(0);
}

void cleanup(void)
{
    RemIntServer(INTB_COPER, &copper_is);
    FreeSignal(cop_data.signal);
    CloseScreen(screen);
    FreeBitMap(bitmaps[1]);
    FreeBitMap(bitmaps[0]);
    unloadGraphics(BASIC_DATA, gfx);
}

/* Workbench */
int wbmain(struct WBStartup *wbs)
{
    return(0);
}

BOOL openLibs(void)
{
    if (!(IntuitionBase = OpenLibrary("intuition.library", KICK_VERSION)))
    {
        FPrintf(out, "Unable to open intuition.library V%ld!\n", KICK_VERSION);
    }
    else
    {
        if (!(IFFParseBase = OpenLibrary("iffparse.library", KICK_VERSION)))
        {
            FPrintf(out, "Unable to open iffparse.library V%ld!\n", KICK_VERSION);
        }
        else
        {
            if (!(DiskfontBase = OpenLibrary("diskfont.library", KICK_VERSION)))
            {
                FPrintf(out, "Unable to open diskfont.library V%ld!\n", KICK_VERSION);
            }
            else
            {
                return(TRUE);
            }
            CloseLibrary(IFFParseBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

void closeLibs(void)
{
    CloseLibrary(DiskfontBase);
    CloseLibrary(IFFParseBase);
    CloseLibrary(IntuitionBase);
}

BOOL openDevs(void)
{
    /* Open gameport and audio */
    return(TRUE);
}

void closeDevs(void)
{

}

BOOL loadGraphics(WORD kind)
{
    UBYTE name[] = "Data1/Gfx/Graphics.iff";
    struct IFFHandle *iff;
    LONG err;
    LONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        ID_ILBM, ID_CAMG
    };
    BOOL done = FALSE;

    if (iff = AllocIFF())
    {
        BPTR f;
        if (!(iff->iff_Stream = f = Open(name, MODE_OLDFILE)))
        {
            FPrintf(out, "Couldn't open '%s'!\n", name);
        }
        else
        {
            InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, IFFF_READ)) == 0)
            {
                if ((err = PropChunks(iff, props, 3)) == 0)
                {
                    if ((err = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
                    {
                        err = ParseIFF(iff, IFFPARSE_SCAN);
                        if (err == 0 || err == IFFERR_EOC || err == IFFERR_EOF)
                        {
                            struct StoredProperty *sp;

                            if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
                            {
                                struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;

                                FPrintf(out, "Graphics info:\n");
                                FPrintf(out, "Color depth: %ld\n", (ULONG)bmhd->bmh_Depth);
                                FPrintf(out, "Aspect ratio: %ld/%ld\n", bmhd->bmh_XAspect, bmhd->bmh_YAspect);
                                FPrintf(out, "Page size: %ld/%ld\n", bmhd->bmh_PageWidth, bmhd->bmh_PageHeight);

                                if (sp = FindProp(iff, ID_ILBM, ID_CAMG))
                                {
                                    ULONG modeID = *(ULONG *)sp->sp_Data;
                                    FPrintf(out, "Display mode: $%lX\n", modeID);

                                    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
                                    {
                                        WORD count = sp->sp_Size / 3;

                                        /* Obtain colormap */
                                        if (colors = AllocVec((sp->sp_Size + 2) * sizeof(ULONG), MEMF_PUBLIC))
                                        {
                                            readColors(sp->sp_Data, colors, count);

                                            /* Unpack picture */
                                            if (gfx = unpackPicture(bmhd, iff))
                                            {
                                                FPrintf(out, "Unpack done\n");
                                                done = TRUE;
                                            }
                                            else
                                            {
                                                FreeVec(colors);
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
    return(done);
}

void readColors(UBYTE *data, ULONG *colors, WORD count)
{
    WORD i;

    *colors++ = count << 16;

    for (i = 0; i < count; i++)
    {
        UBYTE red = *data++, green = *data++, blue = *data++;
        *colors++ = RGB(red);
        *colors++ = RGB(green);
        *colors++ = RGB(blue);
    }
    *colors = 0L;
}

struct BitMap *unpackPicture(struct BitMapHeader *bmhd, struct IFFHandle *iff)
{
    struct BitMap *bm;
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    UBYTE depth = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression;
    UBYTE msk = bmhd->bmh_Masking;
    BOOL success = FALSE;

    if (bm = AllocBitMap(width, height, depth, BMF_INTERLEAVED, NULL))
    {
        struct ContextNode *cn;

        if (cn = CurrentChunk(iff))
        {
            LONG size = cn->cn_Size;
            BYTE *buffer;

            if (buffer = AllocMem(size, MEMF_PUBLIC))
            {
                if (ReadChunkBytes(iff, buffer, size) == size)
                {
                    BYTE *cur = buffer;
                    PLANEPTR planes[9];
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
                            if (!(success = unpackRow(&cur, &size, planes[i], bpr, cmp)))
                                break;
                            planes[i] += bm->BytesPerRow;
                        }
                        if (!success)
                            break;
                    }
                    FreeMem(buffer, cn->cn_Size);
                }

            }
        }
        if (!success)
        {
            FreeBitMap(bm);
            bm = NULL;
        }
    }
    return(bm);
}

BOOL unpackRow(BYTE **dataptr, LONG *sizeptr, BYTE *dest, WORD bpr, UBYTE cmp)
{
    BYTE *data = *dataptr;
    LONG size = *sizeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        CopyMem(data, dest, bpr);
        data += bpr;
        size -= bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE c;
            if (size < 1)
            {
                FPrintf(out, "Underflow\n");
                return(FALSE);
            }
            size--;
            if ((c = *data++) >= 0)
            {
                WORD count = c + 1;
                if (size < count || bpr < count)
                {
                    FPrintf(out, "unpacked Overflow (%ld) (%ld) (%ld)\n", size, bpr, count);
                    return(FALSE);
                }
                size -= count;
                bpr -= count;
                while (count-- > 0)
                    *dest++ = *data++;
            }
            else if (c != -128)
            {
                WORD count = (-c) + 1;
                BYTE byte;
                if (size < 1 || bpr < count)
                {
                    FPrintf(out, "packed Overflow\n");
                    return(FALSE);
                }
                size--;
                bpr -= count;
                byte = *data++;
                while (count-- > 0)
                    *dest++ = byte;
            }
        }
    }
    else
    {
        return(FALSE);
    }

    *dataptr = data;
    *sizeptr = size;
    return(TRUE);
}

void unloadGraphics(WORD kind, struct BitMap *gfx)
{
    FreeBitMap(gfx);
    FreeVec(colors);
}
