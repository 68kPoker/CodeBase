
/* Load color palette and picture */

#include <dos/dos.h>
#include <exec/memory.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

ULONG ilbmProps[] =
{
	ID_ILBM, ID_BMHD,
	ID_ILBM, ID_CMAP,
	0
};

ULONG ilbmStops[] =
{
	ID_ILBM, ID_BODY,
	0
};

BOOL scanIFF(struct IFFInfo *ii);
BOOL interpretILBM(struct IFFInfo *ii);
BOOL unpackILBM(BYTE *buffer, LONG size, struct ILBMInfo *ilbm);
BOOL unpackRow(BYTE **srcptr, LONG *sizeptr, PLANEPTR *destptr, WORD bpr, UBYTE cmp);

BOOL readILBM(struct ILBMInfo *ilbm, STRPTR name)
{
	struct IFFInfo ii;

	ii.name  = name;
	ii.type  = ID_ILBM;
	ii.ID	 = ID_FORM;
	ii.props = ilbmProps;
	ii.stops = ilbmStops;
	ii.interpret = interpretILBM;
	ii.ilbm  = ilbm;

	if (readIFF(&ii))
	{
		closeIFF(&ii);
		return(TRUE);
	}
	return(FALSE);
}

/* Open and scan IFF file */
BOOL readIFF(struct IFFInfo *ii)
{
	struct IFFHandle *iff;

	if (ii->iff = iff = AllocIFF())
	{
		if (iff->iff_Stream = Open(ii->name, MODE_OLDFILE))
		{
			InitIFFasDOS(iff);
			if ((ii->err = OpenIFF(iff, IFFF_READ)) == 0)
			{
				/* Perform scanning */
				if (scanIFF(ii))
				{
					return(TRUE);
				}
				CloseIFF(iff);
			}
			Close(iff->iff_Stream);
		}
		FreeIFF(iff);
	}
	return(FALSE);
}

void closeIFF(struct IFFInfo *ii)
{
	struct IFFHandle *iff = ii->iff;

	CloseIFF(iff);
	Close(iff->iff_Stream);
	FreeIFF(iff);
}

/* Count chunk ID pairs */
WORD countIFF(ULONG *chunkIDs)
{
	WORD count = 0;

	if (!chunkIDs)
	{
		return(0);
	}

	while (*chunkIDs++)
	{
		count++;
	}
	return(count);
}

BOOL scanIFF(struct IFFInfo *ii)
{
	struct IFFHandle *iff = ii->iff;
	struct ContextNode *cn;

	if ((ii->err = ParseIFF(iff, IFFPARSE_STEP)) == 0)
	{
		if (cn = CurrentChunk(iff))
		{
			if (cn->cn_Type == ii->type && cn->cn_ID == ii->ID)
			{
				if ((ii->err = PropChunks(iff, (LONG *)ii->props, countIFF(ii->props))) == 0)
				{
					if ((ii->err = StopChunks(iff, (LONG *)ii->stops, countIFF(ii->stops))) == 0)
					{
						if ((ii->err = ParseIFF(iff, IFFPARSE_SCAN)) == 0	||
							ii->err == IFFERR_EOC	||
							ii->err == IFFERR_EOF)
						{
							if (ii->interpret(ii))
							{
								return(TRUE);
							}
						}
					}
				}
			}
		}
	}
	return(FALSE);
}

/* Interpret ILBM */
BOOL interpretILBM(struct IFFInfo *ii)
{
	struct ILBMInfo *ilbm = ii->ilbm;
	struct IFFHandle *iff = ii->iff;
	struct StoredProperty *sp;

	/* Read header */
	if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
	{
		ilbm->bmhd = (struct BitMapHeader *)sp->sp_Data;

		/* Read colors */
		if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
		{
			WORD colors = sp->sp_Size / 3;
			if (ilbm->cm) /* = GetColorMap(colors)) */
			{
				UBYTE *cmap = sp->sp_Data;

				/* Read body */
				WORD depth = ilbm->bmhd->bmh_Depth;

				WORD i;
				for (i = 0; i < colors; i++)
				{
					UBYTE red 	= *cmap++;
					UBYTE green = *cmap++;
					UBYTE blue 	= *cmap++;

					SetRGB32CM(ilbm->cm, i, RGB(red), RGB(green), RGB(blue));
				}

				if (ilbm->bmhd->bmh_Masking == mskHasMask)
				{
					depth++;
				}

				if (ilbm->bm = AllocBitMap(ilbm->bmhd->bmh_Width, ilbm->bmhd->bmh_Height, depth, 0, NULL))
				{
					struct ContextNode *cn;

					if (cn = CurrentChunk(iff))
					{
						BYTE *buffer;
						LONG size = cn->cn_Size;

						if (buffer = AllocMem(size, MEMF_PUBLIC))
						{
							if (ReadChunkBytes(iff, buffer, size) == size)
							{
								if (unpackILBM(buffer, size, ilbm))
								{
									FreeMem(buffer, size);
									return(TRUE);
								}
							}
							FreeMem(buffer, size);
						}
					}
					FreeBitMap(ilbm->bm);
				}
				/* FreeColorMap(ilbm->cm); */
			}
		}
	}
	return(FALSE);
}

BOOL unpackILBM(BYTE *buffer, LONG size, struct ILBMInfo *ilbm)
{
	WORD width 	= ilbm->bmhd->bmh_Width;
	WORD height = ilbm->bmhd->bmh_Height;
	WORD depth  = ilbm->bmhd->bmh_Depth;
	UBYTE cmp   = ilbm->bmhd->bmh_Compression;
	UBYTE msk   = ilbm->bmhd->bmh_Masking;
	WORD bpr	= RowBytes(width);
	PLANEPTR planes[9];
	WORD i, j;

	if (msk == mskHasMask)
	{
		depth++;
	}

	for (i = 0; i < depth; i++)
	{
		planes[i] = ilbm->bm->Planes[i];
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

BOOL unpackRow(BYTE **srcptr, LONG *sizeptr, PLANEPTR *destptr, WORD bpr, UBYTE cmp)
{
	BYTE *src = *srcptr;
	LONG size = *sizeptr;
	BYTE *dest = *destptr;

	if (cmp == cmpNone)
	{
		if (size < bpr)
		{
			return(FALSE);
		}
		size -= bpr;
		CopyMem(src, dest, bpr);
		src += bpr;
		dest += bpr;
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
			if ((c = *src++) >= 0)
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
					*dest++ = *src++;
				}
			}
			else if (c != -128)
			{
				BYTE data;
				WORD count = (-c) + 1;
				if (size < 1 || bpr < count)
				{
					return(FALSE);
				}
				size--;
				bpr -= count;
				data = *src++;
				while (count-- > 0)
				{
					*dest++ = data;
				}
			}
		}
	}
	else
		return(FALSE);

	*srcptr = src;
	*sizeptr = size;
	*destptr = dest;
	return(TRUE);
}
