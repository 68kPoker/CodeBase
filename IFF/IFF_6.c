
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "IFF.h"
#include <stdio.h>
#include "debug.h"

#define SIZE 4096 /* Buffer size */

BOOL prepIFF(IFF *iff)
{
    if (iff->iff = AllocIFF())
    {
        if (iff->buffer = iff->current = AllocMem(SIZE, MEMF_PUBLIC))
        {
            iff->read = 0;
            return(TRUE);
        }
        FreeIFF(iff->iff);
    }
    return(FALSE);
}

VOID freeIFF(IFF *iff)
{
    FreeMem(iff->buffer, SIZE);
    FreeIFF(iff->iff);
}

BOOL openIFF(IFF *iff, STRPTR name)
{
    if (name[0] == '-' && name[1] == 'c')
    {
        UBYTE clip = name[2] ? atoi(name + 2) : PRIMARY_CLIP;

        if (iff->iff->iff_Stream = (ULONG)OpenClipboard(clip))
        {
            InitIFFasClip(iff->iff);
            iff->isClip = TRUE;
            if ((iff->err = OpenIFF(iff->iff, IFFF_READ)) == 0)
            {
                return(TRUE);
            }
            CloseClipboard((struct ClipboardHandle *)iff->iff->iff_Stream);
        }
    }
    else if (iff->iff->iff_Stream = Open(name, MODE_OLDFILE))
    {
        InitIFFasDOS(iff->iff);
        iff->isClip = FALSE;
        if ((iff->err = OpenIFF(iff->iff, IFFF_READ)) == 0)
        {
            return(TRUE);
        }
        Close((BPTR)iff->iff->iff_Stream);
    }
    return(FALSE);
}

VOID closeIFF(IFF *iff)
{
    CloseIFF(iff->iff);
    if (iff->isClip)
    {
        CloseClipboard((struct ClipboardHandle *)iff->iff->iff_Stream);
    }
    else
    {
        Close((BPTR)iff->iff->iff_Stream);
    }
}

struct ContextNode *enterIFF(IFF *iff)
{
    if ((iff->err = ParseIFF(iff->iff, IFFPARSE_STEP)) == 0)
    {
        struct ContextNode *cn;
        if (cn = CurrentChunk(iff->iff))
        {
            return(cn);
        }
    }
    return(NULL);
}

struct BitMapHeader *scanILBM(IFF *iff) /* Scan ILBM chunks */
{
    LONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };

    if ((iff->err = PropChunks(iff->iff, props, 2)) == 0)
    {
        if ((iff->err = StopChunk(iff->iff, ID_ILBM, ID_BODY)) == 0)
        {
            if ((iff->err = ParseIFF(iff->iff, IFFPARSE_SCAN)) == 0)
            {
                struct StoredProperty *sp;

                if (sp = FindProp(iff->iff, ID_ILBM, ID_BMHD))
                {
                    return((struct BitMapHeader *)sp->sp_Data);
                }
            }
        }
    }
    return(NULL);
}

struct VoiceHeader *scan8SVX(IFF *iff)
{

}

BOOL readIFF(IFF *iff, BYTE *buffer, LONG size)
{
    LONG toRead;

    while (size > 0)
    {
        if (iff->read == 0)
        {
            if ((iff->read = ReadChunkBytes(iff->iff, iff->buffer, SIZE)) == 0)
            {
                return(FALSE);
            }
            iff->current = iff->buffer;
        }
        if (iff->read < size)
        {
            toRead = iff->read;
        }
        else
        {
            toRead = size;
        }
        CopyMem(iff->current, buffer, toRead);
        size -= toRead;
        iff->read -= toRead;
        iff->current += toRead;
        buffer += toRead;
    }
    return(TRUE);
}
