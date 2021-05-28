
/*
** Loading IFF pictures.
*/

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/dos_protos.h>

#include "IFF.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

/*
** Obtain BitMapHeader from IFF file.
*/
struct BitMapHeader *getBMHD(struct IFFHandle *iff)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
	{
		return((struct BitMapHeader *)sp->sp_Data);
	}
	return(NULL);
}

/*
** Count IFF chunk identifier pair.
*/
WORD countChunks(ULONG *chunks)
{
	WORD n = 0;

	if (!chunks)
	{
		return(0);
	}

	while (*chunks++)
	{
		n++;
	}
	return(n / 2);
}

/*
** Open and scan IFF file.
*/
struct IFFHandle *openIFF(STRPTR name, ULONG mode, ULONG *props, ULONG *stops)
{
	struct IFFHandle *iff;
	LONG dosmodes[] = { MODE_OLDFILE, MODE_NEWFILE };

	if (iff = AllocIFF())
	{
		if (iff->iff_Stream = Open(name, dosmodes[mode & 1]))
		{
			InitIFFasDOS(iff);
			if (!OpenIFF(iff, mode))
			{
				if (!PropChunks(iff, props, countChunks(props)))
				{
					if (!StopChunks(iff, stops, countChunks(stops)))
					{
						if (!ParseIFF(iff, IFFPARSE_SCAN))
						{
							return(iff);
						}
					}
				}
				CloseIFF(iff);
			}
			Close(iff->iff_Stream);
		}
		FreeIFF(iff);
	}
	return(NULL);
}

/*
** Close IFF file.
*/
void closeIFF(struct IFFHandle *iff)
{
	CloseIFF(iff);
	Close(iff->iff_Stream);
	FreeIFF(iff);
}

/*
** Load ColorMap from IFF.
*/
BOOL loadCMAP(struct IFFHandle *iff, struct ColorMap **cm)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
	{
		UBYTE *cmap = sp->sp_Data;
		LONG size = sp->sp_Size;
		WORD colors = size / 3;
		BOOL custom = FALSE;

		if (!*cm)
		{
			*cm = GetColorMap(colors);
		}

		if (*cm)
		{
			WORD i;

			for (i = 0; i < colors; i++)
			{
				UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;

				SetRGB32CM(*cm, i, RGB(red), RGB(green), RGB(blue));
			}

			return(TRUE);
		}
	}
	return(FALSE);
}

/*
** Unpack one row of picture data.
*/
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
		buffer += bpr;
		size -= bpr;
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

/*
** Unpack picture body.
*/
BOOL unpackBODY(BYTE *buffer, LONG size, struct BitMap *bm, struct BitMapHeader *bmhd)
{
	WORD width = bmhd->bmh_Width, height = bmhd->bmh_Height;
	UBYTE depth = bmhd->bmh_Depth, cmp = bmhd->bmh_Compression;
	UBYTE plane;
	PLANEPTR planes[9];
	WORD row;
	WORD bpr = RowBytes(width);

	for (plane = 0; plane < depth; plane++)
	{
		planes[plane] = bm->Planes[plane];
	}

	for (row = 0; row < height; row++)
	{
		for (plane = 0; plane < depth; plane++)
		{
			if (!unpackRow(&buffer, &size, planes[plane], bpr, cmp))
			{
				return(FALSE);
			}
			planes[plane] += bm->BytesPerRow;
		}
	}
	return(TRUE);
}

/*
** Load ILBM body into BitMap.
*/
BOOL loadBODY(struct IFFHandle *iff, struct BitMap *bm, struct BitMapHeader *bmhd)
{
	struct ContextNode *cn;
	BOOL result = FALSE;

	if (cn = CurrentChunk(iff))
	{
		LONG size = cn->cn_Size;
		BYTE *buffer;

		if (buffer = AllocMem(size, MEMF_PUBLIC))
		{
			if (ReadChunkBytes(iff, buffer, size) == size)
			{
				result = unpackBODY(buffer, size, bm, bmhd);
			}
			FreeMem(buffer, size);
		}
	}
	return(result);
}

/*
** Load BitMap and ColorMap from file.
*/
struct BitMap *loadBitMap(STRPTR name, struct ColorMap **cm)
{
	struct IFFHandle *iff;
	ULONG ilbmprops[] = { ID_ILBM, ID_BMHD, ID_ILBM, ID_CMAP, 0 };
	ULONG ilbmstops[] = { ID_ILBM, ID_BODY, 0 };
	BOOL custom = ((*cm) != NULL);

	if (iff = openIFF(name, IFFF_READ, ilbmprops, ilbmstops))
	{
		struct BitMapHeader *bmhd;

		if (bmhd = getBMHD(iff))
		{
			struct BitMap *bm;

			if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED, NULL))
			{
				if (loadCMAP(iff, cm))
				{
					if (loadBODY(iff, bm, bmhd))
					{
						closeIFF(iff);
						return(bm);
					}
					if (!custom)
					{
						FreeColorMap(*cm);
					}
				}
				FreeBitMap(bm);
			}
		}
		closeIFF(iff);
	}
	return(NULL);
}
