
/* $Log:	IFF.c,v $
 * Revision 1.2  12/.0/.1  .1:.2:.1  Robert
 * Kod uspójniony
 * 
 * Revision 1.1  12/.0/.1  .2:.3:.2  Robert
 * Initial revision
 *  */

#include <dos/dos.h>
#include <exec/memory.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"

LONG ilbmProps[] =
{
	ID_ILBM, ID_BMHD,
	ID_ILBM, ID_CMAP,
	0
};

LONG ilbmStops[] =
{
	ID_ILBM, ID_BODY,
	0
};

/* openIFF() - Otwórz plik lub schowek IFF */

struct IFFHandle *openIFF(ULONG s, void (*init)(struct IFFHandle *), LONG mode)
{
	struct IFFHandle *iff;

	if (iff = AllocIFF())
	{
		iff->iff_Stream = s;
		(*init)(iff);

		if (!OpenIFF(iff, mode))
		{
			return(iff);
		}
		FreeIFF(iff);
	}
	return(NULL);
}

/* openIFile() - Otwórz plik IFF */

struct IFFHandle *openIFile(STRPTR name, LONG mode)
{
	BPTR f;

	if (mode == IFFF_WRITE)
	{
		f = Open(name, MODE_NEWFILE);
	}
	else
	{
		f = Open(name, MODE_OLDFILE);
	}

	if (f)
	{
		struct IFFHandle *iff;
		if (iff = openIFF((ULONG)f, InitIFFasDOS, mode))
		{
			return(iff);
		}
		Close(f);
	}
	return(NULL);
}

/* openIClip() - Otwórz schowek IFF */

struct IFFHandle *openIClip(UBYTE unit, LONG mode)
{
	struct ClipboardHandle *ch;

	if (ch = OpenClipboard(unit))
	{
		struct IFFHandle *iff;
		if (iff = openIFF((ULONG)ch, InitIFFasClip, mode))
		{
			return(iff);
		}
		CloseClipboard(ch);
	}
	return(NULL);
}

/* closeIFF() - Zamknij IFF */

void closeIFF(struct IFFHandle *iff, LONG (*close)(LONG s))
{
	CloseIFF(iff);
	(*close)(iff->iff_Stream);
	FreeIFF(iff);
}

/* countChunks() - Zlicz pary chunków */

WORD countChunks(LONG *chunks)
{
	WORD count = 0;

	if (!*chunks)
	{
		return(0);
	}
	while (*chunks++)
	{
		count++;
	}
	return(count / 2);
}

/* scanIFF() - Przeskanuj plik IFF */

BOOL scanIFF(struct IFFHandle *iff, LONG *props, LONG *stops)
{
	if (!PropChunks(iff, props, countChunks(props)))
	{
		if (!StopChunks(iff, stops, countChunks(stops)))
		{
			if (!ParseIFF(iff, IFFPARSE_SCAN))
			{
				return(TRUE);
			}
		}
	}
	return(FALSE);
}

/* obtainCMAP() - Pobierz mapë i liczbë kolorów */

BOOL obtainCMAP(struct IFFHandle *iff, UBYTE **cmap, WORD *colors)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
	{
		*cmap = sp->sp_Data;
		*colors = sp->sp_Size / 3;
		return(TRUE);
	}
	return(FALSE);
}

/* loadCMAP() - Zaîaduj paletë kolorów */

void loadCMAP(struct ColorMap *cm, UBYTE *cmap, WORD colors)
{
	WORD i;

	for (i = 0; i < colors; i++)
	{
		UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
		SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
	}
}

/* obtainBMHD() - Pobierz dane z naglówka pliku ILBM */

struct BitMapHeader *obtainBMHD(struct IFFHandle *iff, UBYTE *cmp, WORD *bpr, WORD *rows, UBYTE *depth)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
	{
		struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;

		*cmp = bmhd->bmh_Compression;
		*bpr = RowBytes(bmhd->bmh_Width);
		*rows = bmhd->bmh_Height;
		*depth = bmhd->bmh_Depth;

		if (*cmp == cmpNone || *cmp == cmpByteRun1)
		{
			return(bmhd);
		}
	}
	return(NULL);
}

/* loadILBM() - Wczytaj ciaîo obrazka */

struct BitMap *loadILBM(struct IFFHandle *iff, UBYTE cmp, WORD bpr, WORD rows, UBYTE depth)
{
	struct BitMap *bm;

	if (bm = AllocBitMap(bpr << 3, rows, depth, BMF_INTERLEAVED, NULL))
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
					if (unpackILBM(buffer, size, bm, cmp, bpr, rows, depth))
					{
						FreeMem(buffer, size);
						return(bm);
					}
				}
				FreeMem(buffer, size);
			}
		}
		FreeBitMap(bm);
	}
	return(NULL);
}

/* unpackILBM() - Rozpakuj ciaîo obrazka */

BOOL unpackILBM(BYTE *buffer, LONG size, struct BitMap *bm, UBYTE cmp, WORD bpr, WORD rows, UBYTE depth)
{
	PLANEPTR planes[9];
	WORD i, j;
	LONG offset = bm->BytesPerRow;

	for (i = 0; i < depth; i++)
	{
		planes[i] = bm->Planes[i];
	}
	for (j = 0; j < rows; j++)
	{
		for (i = 0; i < depth; i++)
		{
			if (!unpackRow(&buffer, &size, planes[i], cmp, bpr))
			{
				return(FALSE);
			}
			planes[i] += offset;
		}
	}
	return(TRUE);
}

/* unpackRow() - Rozpakuj wiersz obrazka */

BOOL unpackRow(BYTE **bufptr, LONG *sizeptr, BYTE *plane, UBYTE cmp, WORD bpr)
{
	BYTE *buffer = *bufptr;
	LONG size = *sizeptr;

	if (cmp == cmpNone)
	{
		if (bpr < size)
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

	*bufptr = buffer;
	*sizeptr = size;
	return(TRUE);
}
