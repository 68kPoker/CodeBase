
/* $Id$ */

#include "IFF.h"
#include "ILBM.h"

#include <datatypes/pictureclass.h>
#include <libraries/iffparse.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

BOOL unpackRow(BYTE **bufferPtr, LONG *sizePtr, BYTE *plane, WORD bpr, UBYTE cmp);
BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMap *bm, struct BitMapHeader *bmhd);

/* Open IFF ILBM and retrieve header. */

struct IFFHandle *openILBM(STRPTR name, BOOL *clip, LONG *err, struct BitMapHeader **bmhd)
{
	struct IFFHandle *iff;
	LONG props[] =
	{
		ID_ILBM, ID_BMHD,
		ID_ILBM, ID_CMAP,
		0
	}, stops[] =
	{
		ID_ILBM, ID_BODY,
		0
	};

	if (iff = openIFF(name, IFFF_READ, clip, err))
	{
		if ((*err = scanIFF(iff, props, NULL, stops)) == 0)
		{
			if (*bmhd = (struct BitMapHeader *)findPropData(iff, ID_ILBM, ID_BMHD))
			{
				return(iff);
			}
		}
		closeIFF(iff, *clip);
	}
	return(NULL);
}

/* Load ColorMap from IFF ILBM. */
/* cm:		Own ColorMap or NULL to alloc one */

struct ColorMap *loadColorMap(struct IFFHandle *iff, struct ColorMap *cm)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
	{
		WORD colors = sp->sp_Size / 3;
		if (!cm)
		{
			cm = GetColorMap(colors);
		}
		if (cm)
		{
			WORD i;
			UBYTE *cmap = sp->sp_Data;

			for (i = 0; i < colors; i++)
			{
				UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;

				SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
			}
			return(cm);
		}
	}
	return(NULL);
}

/* Load BitMap from IFF ILBM. */

struct BitMap *loadBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
	BYTE *buffer;
	LONG size;
	struct BitMap *bm = NULL;

	if (buffer = readBODY(iff, &size))
	{
		if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED, NULL))
		{
			if (!unpackBitMap(buffer, size, bm, bmhd))
			{
				FreeBitMap(bm);
				bm = NULL;
			}
		}
		freeBODY(buffer, size);
	}
	return(bm);
}

BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMap *bm, struct BitMapHeader *bmhd)
{
	UBYTE cmp = bmhd->bmh_Compression;
	UBYTE msk = bmhd->bmh_Masking;
	WORD width = bmhd->bmh_Width;
	WORD bpr = RowBytes(width);
	WORD height = bmhd->bmh_Height;
	UBYTE depth = bmhd->bmh_Depth;
	PLANEPTR planes[8];
	WORD i, j;

	if (cmp != cmpNone && cmp != cmpByteRun1)
	{
		return(FALSE);
	}

	if (msk != mskNone && msk != mskHasTransparentColor)
	{
		return(FALSE);
	}

	for (i = 0; i < depth; i++)
	{
		planes[i] = bm->Planes[i];
	}

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < depth; i++)
		{
			if (!unpackRow(&buffer, &size, planes[i], bpr, cmp))
			{
				return(FALSE);
			}
			planes[i] += bm->BytesPerRow;
		}
	}
	return(TRUE);
}

BOOL unpackRow(BYTE **bufferPtr, LONG *sizePtr, BYTE *plane, WORD bpr, UBYTE cmp)
{
	BYTE *buffer = *bufferPtr;
	LONG size = *sizePtr;

	if (cmp == cmpNone)
	{
		if (size < bpr)
		{
			return(FALSE);
		}
		CopyMem(buffer, plane, bpr);
		buffer += bpr;
		size -= bpr;
	}
	else
	{
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
				BYTE data;
				if (size < 1 || bpr < count)
				{
					return(FALSE);
				}
				data = *buffer++;
				size--;
				bpr -= count;
				while (count-- > 0)
				{
					*plane++ = data;
				}
			}
		}
	}

	*bufferPtr = buffer;
	*sizePtr = size;
	return(TRUE);
}
