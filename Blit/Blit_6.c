
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

extern __far struct Custom custom;

struct blitMsg
{
	struct Layer *l;
	struct Rectangle cr;
	LONG ox, oy;
};

struct blitInfo
{
	struct BitMap *src;
	WORD sx, sy, dx, dy, w, h;
};

void bltTileBitMap(struct BitMap *src, WORD sx, WORD sy, struct BitMap *dest, WORD dx, WORD dy, WORD width, WORD height)
{
	struct Custom *cust = &custom;
	WORD srcbpr = src->BytesPerRow, destbpr = dest->BytesPerRow;
	LONG srcoffset = (sy * srcbpr) + ((sx >> 4) << 1);
	LONG destoffset = (dy * destbpr) + ((dx >> 4) << 1);
	UBYTE depth = dest->Depth;
	WORD wordwidth = (width + 15) >> 4;
	WORD srcmod = (srcbpr / depth) - (wordwidth << 1);
	WORD destmod = (destbpr / depth) - (wordwidth << 1);

	OwnBlitter();

	WaitBlit();

	cust->bltcon0 = A_TO_D | SRCA | DEST;
	cust->bltcon1 = 0;
	cust->bltapt  = src->Planes[0] + srcoffset;
	cust->bltdpt  = dest->Planes[0] + destoffset;
	cust->bltamod = srcmod;
	cust->bltdmod = destmod;
	cust->bltafwm = 0xffff;
	cust->bltalwm = 0xffff;
	cust->bltsizv = height * depth;
	cust->bltsizh = wordwidth;

	DisownBlitter();
}

__saveds void myBlitHook(register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct blitMsg *bm)
{
	struct Rectangle *cr = &bm->cr;
	WORD px, py;
	/* LONG ox = bm->ox, oy = bm->oy; */
	struct blitInfo *bi = (struct blitInfo *)hook->h_Data;
	LONG dx = bi->dx + bm->l->bounds.MinX, dy = bi->dy + bm->l->bounds.MinY;
	WORD w = bi->w, h = bi->h;
	struct Rectangle dr;

	if (cr->MinX > dx)
	{
		px = cr->MinX - dx;
		dr.MinX = cr->MinX;
	}
	else
	{
		px = 0;
		dr.MinX = dx;
	}

	if (cr->MinY > dy)
	{
		py = cr->MinY - dy;
		dr.MinY = cr->MinY;
	}
	else
	{
		py = 0;
		dr.MinY = dy;
	}

	if (cr->MaxX < (dx + w - 1))
	{
		dr.MaxX = cr->MaxX;
	}
	else
	{
		dr.MaxX = dx + w - 1;
	}

	if (cr->MaxY < (dy + h - 1))
	{
		dr.MaxY = cr->MaxY;
	}
	else
	{
		dr.MaxY = dy + h - 1;
	}

	w = dr.MaxX - dr.MinX + 1;
	h = dr.MaxY - dr.MinY + 1;

	if (w > 0 && h > 0)
	{
		bltTileBitMap(bi->src, bi->sx + px, bi->sy + py, rp->BitMap, dr.MinX, dr.MinY, w, h);
	}
}

__saveds void myBlitBoardHook(register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct blitMsg *bm)
{
	struct Rectangle *cr = &bm->cr;
	WORD px, py;
	/* LONG ox = bm->ox, oy = bm->oy; */
	struct blitInfo *bi = (struct blitInfo *)hook->h_Data;
	LONG dx = bi->dx + bm->l->bounds.MinX, dy = bi->dy + bm->l->bounds.MinY;
	WORD w = bi->w, h = bi->h;
	struct Rectangle dr;
	WORD x, y;

	if (cr->MinX > dx)
	{
		px = cr->MinX - dx;
		dr.MinX = cr->MinX;
	}
	else
	{
		px = 0;
		dr.MinX = dx;
	}

	if (cr->MinY > dy)
	{
		py = cr->MinY - dy;
		dr.MinY = cr->MinY;
	}
	else
	{
		py = 0;
		dr.MinY = dy;
	}

	if (cr->MaxX < (dx + w - 1))
	{
		dr.MaxX = cr->MaxX;
	}
	else
	{
		dr.MaxX = dx + w - 1;
	}

	if (cr->MaxY < (dy + h - 1))
	{
		dr.MaxY = cr->MaxY;
	}
	else
	{
		dr.MaxY = dy + h - 1;
	}

	w = dr.MaxX - dr.MinX + 1;
	h = dr.MaxY - dr.MinY + 1;

	/* Setup board */
	for (y = dr.MinY; y < dr.MaxY; y += 32)
	{
		for (x = dr.MinX; x < dr.MaxX; x += 32)
		{
			WORD src = 1;
			if (x == 0 || y == 32 || x == (640 - 32) || y == (512 - 32))
				src = 0;
			bltTileBitMap(bi->src, src << 5, 0 << 5, rp->BitMap, x, y, 32, 32);
		}
	}
}

void bltTileRastPort(struct BitMap *src, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height)
{
	struct blitInfo bi = { src, sx, sy, dx, dy, width, height };
	struct Hook	hook = { { NULL, NULL }, (ULONG(*)())myBlitHook, NULL, (APTR)&bi };
	struct Rectangle clip = { 0, 0, 639, 511 };

	DoHookClipRects(&hook, rp, &clip);
}

void bltBoardRastPort(struct BitMap *src, WORD sx, WORD sy, struct RastPort *rp, WORD dx, WORD dy, WORD width, WORD height)
{
	struct blitInfo bi = { src, sx, sy, dx, dy, width, height };
	struct Hook	hook = { { NULL, NULL }, (ULONG(*)())myBlitBoardHook, NULL, (APTR)&bi };
	struct Rectangle clip = { 0, 0, 639, 511 };

	DoHookClipRects(&hook, rp, &clip);
}
