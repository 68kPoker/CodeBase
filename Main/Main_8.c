
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

enum
{
    BOARD_WIN, /* Okno z planszâ */
    MENU_WIN, /* Okno menu */
    REQUESTER_MIN, /* Okno dodatkowego menu */
    WINDOWS
};

enum
{
    IMAGE_CLS, /* Klasa obrazka wycinanego z BitMapy */
    CLASSES
};

enum
{
    CLOSE_GID,
    DEPTH_GID, /* Podstawowe gadûety */
    BASIC_GADGETS
};

enum /* Sygnaîy */
{
    BOARD_SIG,
    MENU_SIG,
    REQUESTER_SIG,
    SAFE_SIG,
    COPPER_SIG,
    SIGNALS
};

struct mainData /* Dane potrzebne podczas dziaîania programu */
{
    struct screenData
    {
        struct Screen *s; /* Ekran */
        struct ScreenBuffer *sb[2]; /* Bufory ekranu */
        struct MsgPort *safePort;
        BOOL safe;
        WORD frame;
        struct Interrupt is; /* Copper */
        struct copperData
        {
            UWORD signal;
            struct Task *task;
        } copper;
    } screen;

    struct windowsData
    {
        struct Window *w[WINDOWS]; /* Okienka */
        Class *cls[CLASSES]; /* Moje klasy */
    } windows;

    struct gfxData
    {
        struct BitMap *gfx; /* Grafika */
        ULONG *pal;
    } gfx;

    struct joyData
    {
        struct IOStdReq *joyIO; /* Joystick */
        struct InputEvent ie;
    } joy;

    ULONG signals[SIGNALS];
};

struct windowData;

struct IFFData
{
    struct IFFHandle *iff;
    LONG err;
    struct bufferData
    {
        BYTE *beg, *cur;
        LONG size, left;
    } buffer;
};

__far extern struct Custom custom;;

void myCopper();

prepScreen(struct screenData *data)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };
    struct UCopList *ucl;

    if (data->s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        if (data->sb[0] = AllocScreenBuffer(data->s, NULL, SB_SCREEN_BITMAP))
        {
            if (data->sb[1] = AllocScreenBuffer(data->s, NULL, 0))
            {
                if (data->safePort = CreateMsgPort())
                {
                    data->safe = TRUE;
                    data->frame = 1;
                    data->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = data->safePort;
                    data->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = data->safePort;

                    if ((data->copper.signal = AllocSignal(-1)) != -1)
                    {
                        data->copper.task = FindTask(NULL);
                        data->is.is_Data = (APTR)&data->copper;
                        data->is.is_Code = myCopper;
                        data->is.is_Node.ln_Pri = 0;
                        data->is.is_Node.ln_Name = "Magazyn";

                        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                        {
                            CINIT(ucl, 3);
                            CWAIT(ucl, 0, 0);
                            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                            CEND(ucl);

                            Forbid();
                            data->s->ViewPort.UCopIns = ucl;
                            Permit();

                            RethinkDisplay();

                            AddIntServer(INTB_COPER, &data->is);

                            return(TRUE);
                        }
                        FreeSignal(data->copper.signal);
                    }
                    DeleteMsgPort(data->safePort);
                }
                FreeScreenBuffer(data->s, data->sb[1]);
            }
            FreeScreenBuffer(data->s, data->sb[0]);
        }
        CloseScreen(data->s);
    }
    return(FALSE);
}

closeScreen(struct screenData *data)
{
    RemIntServer(INTB_COPER, &data->is);
    FreeSignal(data->copper.signal);

    if (!data->safe)
    {
        while (!GetMsg(data->safePort))
        {
            WaitPort(data->safePort);
        }
    }
    DeleteMsgPort(data->safePort);
    FreeScreenBuffer(data->s, data->sb[1]);
    FreeScreenBuffer(data->s, data->sb[0]);
    CloseScreen(data->s);
}

prepWindows(struct windowsData *data, struct Screen *s)
{
    if (data->w[BOARD_WIN] = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Width,
        WA_Height,          s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(TRUE);
    }
    return(FALSE);
}

closeWindows(struct windowsData *data)
{
    CloseWindow(data->w[BOARD_WIN]);
}

prepIFF(struct IFFData *data, STRPTR name)
{
    struct IFFHandle *iff;
    LONG err = 0;
    const LONG bufSize = 2048;

    if (data->iff = iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, IFFF_READ)) == 0)
            {
                if (data->buffer.beg = AllocMem(bufSize, MEMF_PUBLIC))
                {
                    data->buffer.size = bufSize;
                    data->buffer.left = 0;
                    return(TRUE);
                }
                CloseIFF(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    data->err = err;
    return(FALSE);
}

closeIFF(struct IFFData *data)
{
    struct IFFHandle *iff = data->iff;

    FreeMem(data->buffer.beg, data->buffer.size);
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

LONG readChunkBytes(struct IFFData *iff, BYTE *dest, WORD bytes)
{
    LONG actual = 0;
    WORD min;

    while (bytes > 0)
    {
        if (iff->buffer.left == 0)
        {
            if ((iff->buffer.left = ReadChunkBytes(iff->iff, iff->buffer.beg, iff->buffer.size)) == 0)
            {
                break;
            }
            iff->buffer.cur = iff->buffer.beg;
        }
        if (bytes < iff->buffer.left)
        {
            min = bytes;
        }
        else
        {
            min = iff->buffer.left;
        }
        CopyMem(iff->buffer.cur, dest, min);
        iff->buffer.cur += min;
        iff->buffer.left -= min;
        dest += min;
        actual += min;
        bytes -= min;
    }
    return(actual);
}

unpackRow(struct IFFData *iff, BYTE *dest, WORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        if (readChunkBytes(iff, dest, bpr) != bpr)
        {
            return(FALSE);
        }
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;

            if (readChunkBytes(iff, &con, 1) != 1)
            {
                return(FALSE);
            }
            if (con >= 0)
            {
                WORD count = con + 1;
                if (bpr < count || readChunkBytes(iff, dest, count) != count)
                {
                    return(FALSE);
                }
                dest += count;
                bpr -= count;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE rep;
                if (bpr < count || readChunkBytes(iff, &rep, 1) != 1)
                {
                    return(FALSE);
                }
                bpr -= count;
                while (count-- > 0)
                {
                    *dest++ = rep;
                }
            }
        }
    }
    else
    {
        return(FALSE);
    }
    return(TRUE);
}

unpackBitMap(struct IFFData *iff, struct BitMapHeader *bmhd, struct BitMap *bm)
{
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    UBYTE depth = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression;
    UBYTE msk = bmhd->bmh_Masking;
    WORD row, plane;
    PLANEPTR planes[9];
    WORD bpr = RowBytes(width);

    if (msk != mskNone && msk != mskHasTransparentColor)
    {
        return(FALSE);
    }

    for (plane = 0; plane < depth; plane++)
    {
        planes[plane] = bm->Planes[plane];
    }

    for (row = 0; row < height; row++)
    {
        for (plane = 0; plane < depth; plane++)
        {
            if (!unpackRow(iff, planes[plane], bpr, cmp))
            {
                return(FALSE);
            }
            planes[plane] += bm->BytesPerRow;
        }
    }
    return(TRUE);
}

loadPal(struct StoredProperty *sp, ULONG *pal)
{
    WORD colors = sp->sp_Size / 3;
    WORD i;
    UBYTE *cmap = sp->sp_Data;

    *pal++ = colors << 16;

    for (i = 0; i < sp->sp_Size; i++)
    {
        UBYTE data = *cmap++;
        *pal++ = RGB(data);
    }
    *pal = 0L;
}

prepGfx(struct gfxData *data)
{
    struct IFFData iff;
    LONG err;
    struct BitMapHeader *bmhd;

    if (prepIFF(&iff, "Dane/Grafika.iff"))
    {
        if ((err = PropChunk(iff.iff, ID_ILBM, ID_BMHD)) == 0   &&
            (err = PropChunk(iff.iff, ID_ILBM, ID_CMAP)) == 0   &&
            (err = StopChunk(iff.iff, ID_ILBM, ID_BODY)) == 0)
        {
            err = ParseIFF(iff.iff, IFFPARSE_SCAN);

            if (err == 0 || err == IFFERR_EOC || err == IFFERR_EOF)
            {
                struct StoredProperty *sp;

                if (sp = FindProp(iff.iff, ID_ILBM, ID_BMHD))
                {
                    bmhd = (struct BitMapHeader *)sp->sp_Data;

                    if (data->gfx = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, 0, NULL))
                    {
                        if (sp = FindProp(iff.iff, ID_ILBM, ID_CMAP))
                        {
                            if (data->pal = AllocVec((sp->sp_Size + 2) * sizeof(ULONG), MEMF_PUBLIC|MEMF_CLEAR))
                            {
                                loadPal(sp, data->pal);
                                if (unpackBitMap(&iff, bmhd, data->gfx))
                                {
                                    closeIFF(&iff);
                                    return(TRUE);
                                }
                                FreeVec(data->pal);
                            }
                        }
                        FreeBitMap(data->gfx);
                    }
                }
            }
        }
        closeIFF(&iff);
    }
    return(FALSE);
}

closeGfx(struct gfxData *data)
{
    FreeBitMap(data->gfx);
    FreeVec(data->pal);
}

prepJoy(struct joyData *data)
{
    return(TRUE);
}

closeJoy(struct joyData *data)
{
}

prep(struct mainData *data)
{
    if (prepScreen(&data->screen))
    {
        data->signals[SAFE_SIG] = 1L << data->screen.safePort->mp_SigBit;
        data->signals[COPPER_SIG] = 1L << data->screen.copper.signal;

        if (prepWindows(&data->windows, data->screen.s))
        {
            data->signals[BOARD_SIG] = 1L << data->windows.w[BOARD_WIN]->UserPort->mp_SigBit;

            if (prepGfx(&data->gfx))
            {
                LoadRGB32(&data->screen.s->ViewPort, data->gfx.pal);
                if (prepJoy(&data->joy))
                {
                    return(TRUE);
                }
                closeGfx(&data->gfx);
            }
            closeWindows(&data->windows);
        }
        closeScreen(&data->screen);
    }
    return(FALSE);
}

cleanup(struct mainData *data)
{
    closeJoy(&data->joy);
    closeGfx(&data->gfx);
    closeWindows(&data->windows);
    closeScreen(&data->screen);
}

play(struct mainData *data)
{
    BOOL done = FALSE;
    WORD counter = 0;
    WORD prev[2] = { 0, 0 };

    while (!done)
    {
        ULONG total = 0L;
        WORD i;
        for (i = 0; i < SIGNALS; i++)
        {
            total |= data->signals[i];
        }

        ULONG result = Wait(total);

        if (result & data->signals[SAFE_SIG])
        {
            struct RastPort *rp = data->windows.w[BOARD_WIN]->RPort;
            WORD frame = data->screen.frame;
            UBYTE text[5];

            rp->BitMap = data->screen.sb[frame]->sb_BitMap;

            if (!data->screen.safe)
            {
                while (!GetMsg(data->screen.safePort))
                {
                    WaitPort(data->screen.safePort);
                }
                data->screen.safe = TRUE;
            }

            BltBitMapRastPort(data->gfx.gfx, 0, prev[frame ^ 1], rp, 0, prev[frame ^ 1], 320, 1, 0xc0);
            BltBitMapRastPort(data->gfx.gfx, 0, counter, rp, 0, counter, 320, 1, 0xc0);
            prev[frame] = counter;

            counter++;

            Move(rp, 0, rp->Font->tf_Baseline);
            SetAPen(rp, 1);
            sprintf(text, "%4d", counter);
            Text(rp, text, 4);

            if (counter == 256)
            {
                done = TRUE;
            }
        }

        if (result & data->signals[COPPER_SIG])
        {
            WORD frame = data->screen.frame;

            WaitBlit();
            while (!ChangeScreenBuffer(data->screen.s, data->screen.sb[frame]))
            {
                WaitTOF();
            }
            data->screen.frame = frame ^= 1;
            data->screen.safe = FALSE;
        }

        if (result & data->signals[BOARD_SIG])
        {

        }
    }
}

int main(int argc, char **argv)
{
    /* Deklaracja struktury */
    struct mainData data;

    if (prep(&data))
    {
        play(&data);
        cleanup(&data);
        return(RETURN_OK);
    }
    return(RETURN_FAIL);
}
