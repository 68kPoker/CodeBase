
#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "IFF.h"

#define RowBytes(w) ((((w)+15)>>4)<<1)

struct IFFHandle *openIFF(STRPTR name)
{
	struct IFFHandle *iff;

	if (iff = AllocIFF())
	{
		if (iff->iff_Stream = Open(name, MODE_OLDFILE))
		{
			InitIFFasDOS(iff);
			if (!OpenIFF(iff, IFFF_READ))
			{
				return(iff);
			}
			Close(iff->iff_Stream);
		}
		FreeIFF(iff);
	}
	return(NULL);
}

BOOL scanILBM(struct IFFHandle *iff)
{
	if (!PropChunk(iff, ID_ILBM, ID_BMHD))
	{
		if (!PropChunk(iff, ID_ILBM, ID_CMAP))
		{
			if (!StopChunk(iff, ID_ILBM, ID_BODY))
			{
				if (!ParseIFF(iff, IFFPARSE_SCAN))
				{
					return(TRUE);
				}
			}
		}
	}
	return(FALSE);
}

BOOL unpackRow(BYTE *plane, BYTE **bufptr, LONG *sizeptr, WORD bpr, UBYTE cmp)
{
	BYTE *buf = *bufptr;
	LONG size = *sizeptr;

	if (cmp == cmpNone)
	{
		if (size < bpr)
		{
			return(FALSE);
		}
		size -= bpr;
		CopyMem(buf, plane, bpr);
		buf += bpr;
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
			if ((c = *buf++) >= 0)
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
					*plane++ = *buf++;
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
				data = *buf++;
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

	*bufptr = buf;
	*sizeptr = size;

	return(TRUE);
}

BOOL unpackILBM(BYTE *buffer, LONG size, struct BitMap *bm, WORD bpr, UBYTE cmp)
{
	PLANEPTR planes[9];
	WORD i, j;
	UBYTE depth = bm->Depth;
	UWORD height = bm->Rows;

	for (i = 0; i < depth; i++)
	{
		planes[i] = bm->Planes[i];
	}

	for (j = 0; j < height; j++)
	{
		for (i = 0; i < depth; i++)
		{
			if (!unpackRow(planes[i], &buffer, &size, bpr, cmp))
			{
				return(FALSE);
			}
			planes[i] += bm->BytesPerRow;
		}
	}
	return(TRUE);
}

struct BitMap *readILBM(struct IFFHandle *iff)
{
	struct StoredProperty *sp;
	struct BitMapHeader *bmhd;

	if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
	{
		UWORD width, height;
		UBYTE depth, cmp, msk;
		struct BitMap *bm;

		bmhd = (struct BitMapHeader *)sp->sp_Data;

		width = bmhd->bmh_Width;
		height = bmhd->bmh_Height;
		depth = bmhd->bmh_Depth;
		cmp = bmhd->bmh_Compression;
		msk = bmhd->bmh_Masking;

		if (bm = AllocBitMap(width, height, depth, BMF_INTERLEAVED, NULL))
		{
			struct ContextNode *cn;

			if (cn = CurrentChunk(iff))
			{
				BYTE *buffer;
				LONG size = cn->cn_Size;

				if (buffer = AllocMem(size, MEMF_PUBLIC))
				{
					BOOL result = FALSE;
					if (ReadChunkBytes(iff, buffer, size) == size)
					{
						result = unpackILBM(buffer, size, bm, RowBytes(width), cmp);
					}
					FreeMem(buffer, size);
					if (result)
					{
						return(bm);
					}
				}
			}
			FreeBitMap(bm);
		}
	}
	return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
	CloseIFF(iff);
	Close(iff->iff_Stream);
	FreeIFF(iff);
}
