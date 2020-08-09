
/* Additional IFF info */

#ifndef IFF_H
#define IFF_H

struct ILBMInfo
{
	struct BitMapHeader *bmhd;
	struct ColorMap		*cm;
	struct BitMap		*bm;
};

struct IFFInfo
{
	STRPTR name; /* File name */
	struct IFFHandle *iff;
	LONG err; /* Error code */
	ULONG type, ID; /* Requested type and ID (e.g. ILBM FORM) */
	ULONG *props, *stops;
	BOOL (*interpret)(struct IFFInfo *ii); /* Chunk interpret function */

	struct ILBMInfo *ilbm; /* Extra data */
};

BOOL readIFF(struct IFFInfo *ii);
void closeIFF(struct IFFInfo *ii);
BOOL readILBM(struct ILBMInfo *ilbm, STRPTR name);

#endif /* IFF_H */
