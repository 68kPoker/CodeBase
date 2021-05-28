
/* $Id$ */

#include <stdlib.h>
#include <stdio.h>

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"

/* Open IFF file for reading or writing */
/* name:	Name of the file or -c, -c1, -c2, ... for clipboard */
/* mode:	IFFF_READ or IFFF_WRITE */
/* clip:	Pointer to BOOL */
/* err:		Pointer to LONG */

struct IFFHandle *openIFF(STRPTR name, LONG mode, BOOL *clip, LONG *err)
{
	struct IFFHandle *iff;
	ULONG s;
	LONG dosmodes[] =
	{
		MODE_OLDFILE,
		MODE_NEWFILE
	};

	if (name[0] == '-' && name[1] == 'c')
	{
		UBYTE c = PRIMARY_CLIP;
		if (name[2])
		{
			c = atoi(name + 2);
		}
		if (!(s = (ULONG)OpenClipboard(c)))
		{
			printf("Couldn't open Clipboard unit %d.\n", c);
			return(NULL);
		}
		*clip = TRUE;
	}
	else
	{
		if (!(s = (ULONG)Open(name, dosmodes[mode & 1])))
		{
			printf("Couldn't open file %s.\n", name);
			return(NULL);
		}
		*clip = FALSE;
	}

	if (iff = AllocIFF())
	{
		iff->iff_Stream = s;
		if (*clip)
		{
			InitIFFasClip(iff);
		}
		else
		{
			InitIFFasDOS(iff);
		}
		if (!(*err = OpenIFF(iff, mode)))
		{
			return(iff);
		}
		else
		{
			printf("OpenIFF error %ld.\n", *err);
		}
		FreeIFF(iff);
	}
	else
	{
		printf("Out of memory.\n");
	}

	if (*clip)
	{
		CloseClipboard((struct ClipboardHandle *)s);
	}
	else
	{
		Close((BPTR)s);
	}
	return(NULL);
}

void closeIFF(struct IFFHandle *iff, BOOL clip)
{
	ULONG s = iff->iff_Stream;
	CloseIFF(iff);
	FreeIFF(iff);
	if (clip)
	{
		CloseClipboard((struct ClipboardHandle *)s);
	}
	else
	{
		Close((BPTR)s);
	}
}

/* Count IFF identifier pairs */
/* chunks:	identifier pairs */

WORD countIFF(LONG *chunks)
{
	WORD n = 0;
	if (!*chunks)
	{
		return(0);
	}
	while (*chunks++)
	{
		n++;
	}
	return(n / 2);
}

/* Scan IFF file */
/* iff:		File to scan */
/* props:	Property chunks */
/* colls:	Collection chunks */
/* stops:	Stop chunks */

LONG scanIFF(struct IFFHandle *iff, LONG *props, LONG *colls, LONG *stops)
{
	LONG err;

	if ((err = PropChunks		(iff, props, countIFF(props))) != 0	||
		(err = CollectionChunks	(iff, colls, countIFF(colls))) != 0	||
		(err = StopChunks		(iff, stops, countIFF(stops))) != 0)
	{
		printf("Chunk error %ld.\n", err);
		return(err);
	}

	if ((err = ParseIFF(iff, IFFPARSE_SCAN)) != 0	&&
		 err != IFFERR_EOC	&&
		 err != IFFERR_EOF)
	{
		printf("Parse error %ld.\n", err);
		return(err);
	}

	return(err);
}

/* Return Property data pointer. */
/* iff:		IFF file */
/* type:	Property type */
/* id:		Property ID */

BYTE *findPropData(struct IFFHandle *iff, LONG type, LONG id)
{
	struct StoredProperty *sp;

	if (sp = FindProp(iff, type, id))
	{
		return(sp->sp_Data);
	}
	return(NULL);
}

/* Read BODY type chunk to memory. */
/* iff		IFF file */
/* size:	Pointer to LONG - buffer size */

BYTE *readBODY(struct IFFHandle *iff, LONG *size)
{
	struct ContextNode *cn;

	if (cn = CurrentChunk(iff))
	{
		BYTE *buffer;
		*size = cn->cn_Size;
		if (buffer = AllocMem(*size, MEMF_PUBLIC))
		{
			if (ReadChunkBytes(iff, buffer, *size) == *size)
			{
				return(buffer);
			}
			FreeMem(buffer, *size);
		}
	}
	return(NULL);
}

void freeBODY(BYTE *buffer, LONG size)
{
	FreeMem(buffer, size);
}
