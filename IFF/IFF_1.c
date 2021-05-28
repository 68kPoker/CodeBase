
/* $Log:	IFF.c,v $
 * Revision 1.1  12/.0/.2  .0:.5:.5  Unknown
 * Initial revision
 *  */

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

typedef struct IFF {
    struct IFFHandle *iff;
} IFF;

typedef struct ColorMap CMAP;
typedef struct BitMap   BMAP;
typedef struct BitMapHeader BMHD;

typedef struct {
    BYTE *body;
    LONG size;
} BODY;

enum iffErrors {
    NOERR,
    NOMEM,
    NOFILE,
    NOCMAP,
    NOCN,
    NOREAD
};

/* iffOpen() - Open iff file for reading or writing */
LONG iffOpen(IFF *p, STRPTR name, LONG mode)
{
    struct IFFHandle *iff;
    LONG dosModes[] = {
        MODE_OLDFILE,
        MODE_NEWFILE
    };
    LONG err;
    if (p->iff = iff = AllocIFF()) {
        BPTR f;
        if (iff->iff_Stream = f = Open(name, dosModes[mode & 1])) {
            InitIFFasDOS(iff);
            if (!(err = OpenIFF(iff, mode)))
                return(NOERR);
            Close(f);
        }
        else
            err = NOFILE;
        FreeIFF(iff);
    }
    else
        err = NOMEM;
    return(err);
}

void iffClose(IFF *p)
{
    struct IFFHandle *iff = p->iff;
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

/* iffScanAsIlbm() - Scan IFF ILBM chunks */
LONG iffScanAsIlbm(IFF *p)
{
    struct IFFHandle *iff = p->iff;
    LONG props[] = {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };
    if (!(err = PropChunks(iff, props, 2)))
        if (!(err = StopChunk(iff, ID_ILBM, ID_BODY)))
            if (!(err = ParseIFF(iff, IFFPARSE_SCAN)) ||
                err == IFFERR_EOC ||
                err == IFFERR_EOF)
                return(err);
    return(err);
}

/* iffReadCmap() - Read CMAP chunk into ColorMap */
LONG iffReadCmap(IFF *p, CMAP *cm)
{
    struct IFFHandle *iff = p->iff;
    struct StoredProperty *sp;
    if (sp = FindProp(iff, ID_ILBM, ID_CMAP)) {
        UBYTE *data = sp->sp_Data;
        LONG size = sp->sp_Size;
        WORD colors = size / 3, i;
        for (i = 0; i < colors; i++) {
            UBYTE red = *data++, green = *data++, blue = *data++;
            SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
        }
        return(NOERR);
    }
    return(NOCMAP);
}

LONG iffReadBody(IFF *p, BODY *body)
{
    struct IFFHandle *iff = p->iff;
    struct ContextNode *cn;
    LONG err;

    if (cn = CurrentChunk(iff)) {
        body->size = cn->cn_Size;
        if (body->body = AllocMem(body->size, MEMF_PUBLIC)) {
            if (ReadChunkBytes(iff, body->body, body->size) == body->size)
                return(NOERR);
            else
                err = NOREAD;
            FreeMem(body->body, body->size);
        }
        else
            err = NOMEM;
    }
    else
        err = NOCN;
    return(err);

}

LONG iffReadBmap(IFF *p, BMAP *bm, BMHD *bmhd)
{
    BODY body;
    LONG err = 0;
    if (iffReadBody(p, &body)) {
        WORD i, j;
        WORD depth = bmhd->bmh_Depth;
        WORD rows = bmhd->bmh_Height;
        for (i = 0; i < depth; i++)
            planes[i] = bm->Planes[i];
        for (j = 0; j < rows; j++) {
            for (i = 0; i < depth; i++) {
                if (err = unpackRow(body, bmhd, planes[i]))
                    break;
                planes[i] += bm->BytesPerRow;
            }
            if (err)
                break;
        }
        FreeMem(body.body, body.size);
    }
    return(err);
}
