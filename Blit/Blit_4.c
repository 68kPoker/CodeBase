
/* Blit icons and bobs */

#define NDEBUG 1
#include <assert.h>
#include <graphics/rastport.h>
#include <hardware/custom.h>
#include <hardware/blit.h>

#include <clib/layers_protos.h>
#include <clib/graphics_protos.h>
#include <clib/alib_protos.h>

#include "Blit.h"

#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN(a,b) ((a)<=(b)?(a):(b))

extern void hookEntry();

void blitIconInterleaved(struct BitMap *iconbm, WORD iconx, WORD icony, struct BitMap *destbm, WORD destx, WORD desty, WORD width, WORD height);

far extern struct Custom custom;

struct blitMessage
{
	struct Layer *layer;
	struct Rectangle bounds;
	LONG offsetX, offsetY;
};

struct blitInfo
{
	struct BitMap *iconbm, *bobbm;
	PLANEPTR mask;
	BOOL repeatmask;
	WORD iconx, icony;
	WORD bobx, boby;
	WORD destx, desty;
	WORD width, height;
};

/* Blit icon (or its part) from given position in icon bitmap
 * 		to given position in destination bitmap
 *		Note: icon, destination and size must be aligned
 */

void blitIcon(struct BitMap *iconbm, WORD iconx, WORD icony, struct BitMap *destbm, WORD destx, WORD desty, WORD width, WORD height)
{
	ULONG flags;
	struct Custom *cust = &custom;
	WORD i, depth;
	WORD iconbpr;
	WORD destbpr;
	LONG iconoffset;
	LONG destoffset;
	PLANEPTR *iconplanes;
	PLANEPTR *destplanes;

	if ((iconx & 0xf) || (destx & 0xf) || (width & 0xf))
	{
		return;
	}

	flags = GetBitMapAttr(iconbm, BMA_FLAGS);

	if (flags & BMF_INTERLEAVED)
	{
		blitIconInterleaved(iconbm, iconx, icony, destbm, destx, desty, width, height);
		return;
	}

	depth = destbm->Depth;

	iconbpr = iconbm->BytesPerRow;
	destbpr = destbm->BytesPerRow;

	iconoffset = (iconbpr * icony) + ((iconx >> 4) << 1);
	destoffset = (destbpr * desty) + ((destx >> 4) << 1);

	iconplanes = iconbm->Planes;
	destplanes = destbm->Planes;

	width >>= 3; /* Get size in bytes */
	iconbpr -= width; /* Calc modulos */
	destbpr -= width;
	width >>= 1; /* Get size in words */

	OwnBlitter();

	for (i = 0; i < depth; i++)
	{
		WaitBlit();

		cust->bltcon0 = SRCA | DEST | 0xf0;
		cust->bltcon1 = 0;
		cust->bltapt  = iconplanes[i] + iconoffset;
		cust->bltdpt  = destplanes[i] + destoffset;
		cust->bltamod = iconbpr;
		cust->bltdmod = destbpr;
		cust->bltafwm = 0xffff;
		cust->bltalwm = 0xffff;
		cust->bltsizv = height;
		cust->bltsizh = width;
	}

	DisownBlitter();
}

void blitIconInterleaved(struct BitMap *iconbm, WORD iconx, WORD icony, struct BitMap *destbm, WORD destx, WORD desty, WORD width, WORD height)
{
	/* assert((!(iconx & 0xf)) && (!(destx & 0xf)) && (!(width & 0xf))); */

	struct Custom *cust = &custom;
	WORD depth = destbm->Depth;
	PLANEPTR *iconplanes;
	PLANEPTR *destplanes;

	WORD iconbpr = iconbm->BytesPerRow;
	WORD destbpr = destbm->BytesPerRow;

	LONG iconoffset = (iconbpr * icony) + ((iconx >> 4) << 1);
	LONG destoffset = (destbpr * desty) + ((destx >> 4) << 1);

	iconbpr /= depth;
	destbpr /= depth;

	iconplanes = iconbm->Planes;
	destplanes = destbm->Planes;

	width >>= 3; /* Get size in bytes */
	iconbpr -= width; /* Calc modulos */
	destbpr -= width;
	width >>= 1; /* Get size in words */

	OwnBlitter();

	WaitBlit();

	cust->bltcon0 = SRCA | DEST | 0xf0;
	cust->bltcon1 = 0;
	cust->bltapt  = iconplanes[0] + iconoffset;
	cust->bltdpt  = destplanes[0] + destoffset;
	cust->bltamod = iconbpr;
	cust->bltdmod = destbpr;
	cust->bltafwm = 0xffff;
	cust->bltalwm = 0xffff;
	cust->bltsizv = height * depth;
	cust->bltsizh = width;

	DisownBlitter();
}


/* Blit bob (or its part) from given position in bob bitmap
 * 		to given position in destination bitmap using background
 *		Note: background and bob must be aligned
 */

void blitBob(struct BitMap *backbm, WORD backx, WORD backy, struct BitMap *bobbm, WORD bobx, WORD boby, PLANEPTR mask, struct BitMap *destbm, WORD destx, WORD desty, WORD width, WORD height, BOOL repeatmask)
{
	UWORD bltcon1 = 0;

/*	if(!((!(backx & 0xf)) && (!(bobx & 0xf))))
	{
	}*/

	struct Custom *cust = &custom;
	WORD i, depth = destbm->Depth;

	WORD backbpr = backbm->BytesPerRow;
	WORD bobbpr  = bobbm->BytesPerRow;
	WORD destbpr = destbm->BytesPerRow;

	LONG backoffset, boboffset, destoffset;
	UWORD firstmask = 0xffff, lastmask = 0xffff;
	WORD destshift = 0;
	WORD prevwidth;

	PLANEPTR *backplanes;
	PLANEPTR *bobplanes;
	PLANEPTR *destplanes;

	if (bobx & 0xf)
	{
		bltcon1 |= BLITREVERSE;

		destshift = bobx & 0xf;

		backoffset = (backbpr * (backy + height - 1)) + (((backx + width - 1) >> 4) << 1);
		boboffset  = (bobbpr * (boby + height - 1)) + (((bobx + width - 1) >> 4) << 1);
		destoffset = (destbpr * (desty + height - 1)) + (((destx + width - 1) >> 4) << 1);

		lastmask = 0xffff >> (bobx & 0xf);
	}
	else
	{
		lastmask = 0xffff << ((16 - (width & 0xf)) & 0xf);

		backoffset = (backbpr * backy) + ((backx >> 4) << 1);
		boboffset  = (bobbpr * boby) + ((bobx >> 4) << 1);
		destoffset = (destbpr * desty) + ((destx >> 4) << 1);

		destshift = destx & 0xf; /* Bob shift */
	}


	prevwidth = (width + 15) >> 4;

	width += destshift; /* Calc required width */
	width += 15;

	bltcon1 |= destshift << BSHIFTSHIFT;

	backplanes = backbm->Planes;
	bobplanes = bobbm->Planes;
	destplanes = destbm->Planes;

	width >>= 3; /* Get size in bytes */
	width &= 0xfffe; /* Make sure it's even value */
	backbpr -= width; /* Calc modulos */
	bobbpr -= width;
	destbpr -= width;
	width >>= 1; /* Get size in words */

	if (prevwidth < width)
	{
		lastmask = 0;
	}

	OwnBlitter();

	for (i = 0; i < depth; i++)
	{
		UWORD bltcon0;
		WaitBlit();

		bltcon0 = SRCB | SRCC | DEST | 0xca | (destshift << ASHIFTSHIFT);

		if (mask)
			cust->bltcon0 = bltcon0 | SRCA;
		else
			cust->bltcon0 = bltcon0;

		cust->bltcon1 = bltcon1;

		if (mask)
			cust->bltapt  = mask + boboffset;
		else
			cust->bltadat = 0xffff;

		cust->bltbpt  = bobplanes[i] + boboffset;
		cust->bltcpt  = backplanes[i] + backoffset;
		cust->bltdpt  = destplanes[i] + destoffset;
		cust->bltamod = repeatmask ? -(width << 1) : bobbpr;
		cust->bltbmod = bobbpr;
		cust->bltcmod = backbpr;
		cust->bltdmod = destbpr;
		cust->bltafwm = firstmask;
		cust->bltalwm = lastmask;
		cust->bltsizv = height;
		cust->bltsizh = width;
	}

	DisownBlitter();
}

void blitFunc(struct Hook *hook, struct RastPort *rp, struct blitMessage *msg)
{
	struct blitInfo *bi = (struct blitInfo *)hook->h_Data;
	struct Rectangle is;
	WORD width = msg->bounds.MaxX - msg->bounds.MinX + 1;
	WORD height = msg->bounds.MaxY - msg->bounds.MinY + 1;

	is.MinX = MAX(msg->offsetX, bi->destx);
	is.MinY = MAX(msg->offsetY, bi->desty);
	is.MaxX = MIN(msg->offsetX + width - 1, bi->destx + bi->width - 1);
	is.MaxY = MIN(msg->offsetY + height - 1, bi->desty + bi->height - 1);

	if (is.MinX > is.MaxX || is.MinY > is.MaxY)
		return;

	if (!bi->bobbm)
	{
		blitIcon(bi->iconbm, bi->iconx + is.MinX - bi->destx, bi->icony + is.MinY - bi->desty, rp->BitMap, msg->bounds.MinX + is.MinX - msg->offsetX, msg->bounds.MinY + is.MinY - msg->offsetY, is.MaxX - is.MinX + 1, is.MaxY - is.MinY + 1);
	}
	else
	{
		blitBob(bi->iconbm, bi->iconx + is.MinX - bi->destx, bi->icony + is.MinY - bi->desty, bi->bobbm, bi->bobx + is.MinX - bi->destx, bi->boby + is.MinY - bi->desty, bi->mask, rp->BitMap, msg->bounds.MinX + is.MinX - msg->offsetX, msg->bounds.MinY + is.MinY - msg->offsetY, is.MaxX - is.MinX + 1, is.MaxY - is.MinY + 1, bi->repeatmask);
	}
}

void bltRastPort(struct BitMap *iconbm, WORD iconx, WORD icony, struct BitMap *bobbm, WORD bobx, WORD boby, PLANEPTR mask, struct RastPort *destrp, WORD destx, WORD desty, WORD width, WORD height, BOOL repeatmask)
{
	struct blitInfo bi;
	struct Hook hook;
	struct Rectangle rect;

	rect.MinX = rect.MinY = 0;
	rect.MaxX = (destrp->BitMap->BytesPerRow << 3) - 1;
	rect.MaxY = (destrp->BitMap->Rows) - 1;

	bi.iconbm = iconbm;
	bi.iconx  = iconx;
	bi.icony  = icony;
	bi.bobbm  = bobbm;
	bi.bobx	  = bobx;
	bi.boby   = boby;
	bi.mask   = mask;
	bi.repeatmask = repeatmask;
	bi.destx  = destx;
	bi.desty  = desty;
	bi.width  = width;
	bi.height = height;

	hook.h_Entry = (ULONG(*)())hookEntry;
	hook.h_SubEntry = (ULONG(*)())blitFunc;
	hook.h_Data = (APTR)&bi;

	DoHookClipRects(&hook, destrp, &rect);
}

PLANEPTR makeMask(struct BitMap *bm, WORD width, WORD height, UBYTE depth)
{
	struct Custom *cust = &custom;
	PLANEPTR mask;
	WORD i;

	if (mask = AllocRaster(width, height))
	{
		OwnBlitter();

		WaitBlit();

		cust->bltcon0 = SRCA | DEST | NABC | NANBC | NABNC | NANBNC;
		cust->bltcon1 = 0;
		cust->bltapt  = bm->Planes[0];
		cust->bltdpt  = mask;
		cust->bltamod = 0;
		cust->bltdmod = 0;
		cust->bltafwm = 0xffff;
		cust->bltalwm = 0xffff;
		cust->bltsizv = height;
		cust->bltsizh = width >> 4;

		for (i = 1; i < depth; i++)
		{
			WaitBlit();

			cust->bltcon0 = SRCA | SRCB | DEST | A_OR_B;
			cust->bltcon1 = 0;
			cust->bltapt  = bm->Planes[i];
			cust->bltbpt  = mask;
			cust->bltdpt  = mask;
			cust->bltamod = 0;
			cust->bltbmod = 0;
			cust->bltdmod = 0;
			cust->bltafwm = 0xffff;
			cust->bltalwm = 0xffff;
			cust->bltsizv = height;
			cust->bltsizh = width >> 4;
		}

		DisownBlitter();

		return(mask);
	}
	return(NULL);
}

