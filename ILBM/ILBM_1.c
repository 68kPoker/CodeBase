
/* loadilbm.c: load IFF ILBM files */

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>

#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "ILBM.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define RowBytes(w) ((((w)+15)>>4)<<1)

BOOL openIFF(struct IFFHandle *iff, STRPTR name)
{
	if (iff->iff_Stream = Open(name, MODE_OLDFILE))
	{
		InitIFFasDOS(iff);
		if (OpenIFF(iff, IFFF_READ) == 0)
		{
			return(TRUE);
		}
		Close(iff->iff_Stream);
	}
	return(FALSE);
}

void closeIFF(struct IFFHandle *iff)
{
	CloseIFF(iff);
	Close(iff->iff_Stream);
}

BOOL installILBM(struct IFFHandle *iff)
{
	if (PropChunk(iff, ID_ILBM, ID_BMHD) == 0)
	{
		if (PropChunk(iff, ID_ILBM, ID_CMAP) == 0)
		{
			if (StopChunk(iff, ID_ILBM, ID_BODY) == 0)
			{
				if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
				{
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

BYTE *findProp(struct IFFHandle *iff, LONG type, LONG id)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, type, id))
	{
		return(sp->sp_Data);
	}
	return(NULL);
}

BOOL loadCMAP(struct IFFHandle *iff, struct ColorMap **cm)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
	{
		UBYTE *cmap = sp->sp_Data;
		LONG size = sp->sp_Size;
		WORD colors = size / 3;

		if ((*cm) || (*cm = GetColorMap(colors)))
		{
			WORD i;

			for (i = 0; i < colors; i++)
			{
				UBYTE red = *cmap++;
				UBYTE green = *cmap++;
				UBYTE blue = *cmap++;

				SetRGB32CM(*cm, i, RGB(red), RGB(green), RGB(blue));
			}
			return(TRUE);
		}
	}
	return(FALSE);
}

BOOL loadBODY(struct IFFHandle *iff, BYTE **buffer, LONG *size)
{
	struct ContextNode *cn;

	if (cn = CurrentChunk(iff))
	{
		*size = cn->cn_Size;

		if (*buffer = AllocMem(*size, MEMF_PUBLIC))
		{
			if (ReadChunkBytes(iff, *buffer, *size) == *size)
			{
				return(TRUE);
			}
			FreeMem(*buffer, *size);
		}
	}
	return(FALSE);
}

BOOL unpackRow(BYTE **bufferptr, LONG *sizeptr, BYTE *plane, WORD bpr, UBYTE cmp)
{
	BYTE *buffer = *bufferptr;
	LONG size = *sizeptr;

	if (cmp == cmpNone)
	{
		if (size < bpr)
		{
			return(FALSE);
		}
		CopyMem(buffer, plane, bpr);
		size -= bpr;
		buffer += bpr;
	}
	else if (cmp == cmpByteRun1)
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

	*bufferptr = buffer;
	*sizeptr = size;

	return(TRUE);
}

BOOL unpackILBM(struct BitMap **bm, struct BitMapHeader *bmhd, BYTE *buffer, LONG size)
{
	WORD width = bmhd->bmh_Width;
	WORD height = bmhd->bmh_Height;
	WORD depth = bmhd->bmh_Depth;
	UBYTE cmp = bmhd->bmh_Compression;
	UBYTE msk = bmhd->bmh_Masking;
	WORD bpr = RowBytes(width);
	WORD i, j;
	PLANEPTR planes[9];

	if (*bm = AllocBitMap(width, height, depth, BMF_INTERLEAVED, NULL))
	{
		for (i = 0; i < depth; i++)
		{
			planes[i] = (*bm)->Planes[i];
		}

		for (j = 0; j < height; j++)
		{
			for (i = 0; i < depth; i++)
			{
				if (!unpackRow(&buffer, &size, planes[i], bpr, cmp))
				{
					FreeBitMap(*bm);
					return(FALSE);
				}
				planes[i] += (*bm)->BytesPerRow;
			}
		}
		return(TRUE);
	}
	return(FALSE);
}

BOOL loadILBM(STRPTR name, struct BitMap **bm, struct ColorMap **cm)
{
	struct IFFHandle *iff;
	BOOL result = FALSE;

	if (iff = AllocIFF())
	{
		if (openIFF(iff, name))
		{
			if (installILBM(iff))
			{
				struct BitMapHeader *bmhd;

				if (bmhd = (struct BitMapHeader *)findProp(iff, ID_ILBM, ID_BMHD))
				{
					if (loadCMAP(iff, cm))
					{
						BYTE *buffer;
						LONG size;

						if (loadBODY(iff, &buffer, &size))
						{
							result = unpackILBM(bm, bmhd, buffer, size);
							FreeMem(buffer, size);
						}
						if (!result)
						{
							FreeColorMap(*cm);
						}
					}
				}
			}
			closeIFF(iff);
		}
		FreeIFF(iff);
	}
	return(result);
}

void freeILBM(struct BitMap *bm, struct ColorMap *cm)
{
	FreeBitMap(bm);
	if (cm)
	{
		FreeColorMap(cm);
	}
}
